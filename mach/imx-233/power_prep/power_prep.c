/*
 * Copyright 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

/*
 * This "boot applet" assists the boot ROM by initializing the hardware platform
 * to facilitate and optimize booting. For example, it initializes the external
 * memory controller, so applications can be loaded directly to external SDRAM.
 * This work is not done in the ROM, because it is platform dependent.
 */

////////////////////////////////////////////////////////////////////////////////
#include "types.h"
#include "error.h"

#include "registers/regsrtc.h"
#include "registers/regspower.h"
#include "registers/regsdigctl.h"
#include "registers/regsicoll.h"
#include "registers/hw_irq.h"
#include "hw_power.h"
#include "hw_icoll.h"
#include "hw_core.h"
#include "hw_lradc.h"
#include "hw_digctl.h"
#include "ddi_power.h"
#include "registers/regsuartdbg.h"
#include <stdarg.h>


#define POWER_PREP_VERSION_R      1
#define POWER_PREP_VERSION_RC     2


////////////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////////////
bool IsVbusValid( void );
bool IsVdd5vGtVddio( void );
bool Is5vPresent( void );
bool IsBattLevelValidForBoot( void );
void PowerPrep_ClearAutoRestart( void );
int PowerPrep_ConfigurePowerSource( void );
void PowerPrep_PowerDown( void );
void PowerPrep_Setup5vDetect( void );
void PowerPrep_SetupBattDetect( void );
int PowerPrep_BootValid5v( void );
void PowerPrep_Enable4p2( void );
void PowerPrep_Init4p2Parameters( void );
void PowerPrep_Init4p2Regulator( void );
void PowerPrep_Enable4p2BoShutdown( void );
void PowerPrep_Handle5vDisconnectErrata( void );
void PowerPrep_Enable4p2Fiq( void );
void PowerPrep_InitDcdc4p2Source( void );
int PowerPrep_Handle5vConflict( void );
int PowerPrep_InitBattBoFiq( void );
int PowerPrep_InitBattBo( void );
int PowerPrep_Init4p2Bo( void );
int PowerPrep_Init4p2BoFiq( void );
int PowerPrep_5vBoot( void );
int PowerPrep_BattBoot( void );
void PowerPrep_SwitchVdddToDdcdcSource( void );
void PowerPrep_EnableOutputRailProtection( void );
bool PowerPrep_IsBatteryReady( void );


////////////////////////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////////////////////////
#define POWER_PREP_GROUP                        (0x80000000)
#define ERROR_VBUSVALID_RACE_CONDITION          ((int)POWER_PREP_GROUP + 1)
#define ERROR_VDDIO_BROWNOUT                    ((int)POWER_PREP_GROUP + 2)
#define ERROR_5V_OSCILLATING                    ((int)POWER_PREP_GROUP + 3)
#define ERROR_INSUFFICIENT_BATT_AFTER_PSWITCH   ((int)POWER_PREP_GROUP + 4)
#define ERROR_UNEXPECTED_LOOP_EXIT              ((int)POWER_PREP_GROUP + 5)


/* We need to protect the DCDC from trying to source charge
 * from too low of a voltage.  This limit is given in
 * the characteristics and specifications section of the
 * reference manual.  Also, we want to stop sourcing
 * charge from the battery before the battery's internal
 * protection circuitry may activate which would then cause
 * us to lose RTC time.  Generically using 3.0V as the
 * battery's maximum voltage it may activate it's
 * undervoltage protection.  At the time of this writing,
 * 3V is greater than the minimum DCDC operating voltage
 * so we will set the BRWNOUT_LVL bitfield to 3V value.
 */
#define BATTERY_BRWNOUT_BITFIELD_VALUE 15 /* 15 = 3.0V */

/* per the reference manul, the power supplies DCDC output capabilities
 * are given with 3.3V as the minimum battery input voltage. By default,
 * we'd rather not start the device unless we have know that we can
 * supply a known amount of power (given in the reference manual).
 * This value is used only for booting from battery, not from 5V.
 * Typically, you'll want to set this slightly above the lowest
 * input voltage you want to provide the DCDC during boot to account
 * for larger voltage drops battery output resistance when the load
 * is higher than when this voltage check is made.
 *
 * Lastly, we want this voltage high enough above the battery
 * brownout level so that we don't accidentally trigger a
 * battery brownout later on during a more critical time
 * (such as when writing to media which might cause some
 * media corruption).
 */
#define MINIMUM_SAFE_BOOTING_BATTERY_VOLTAGE_MV 3350


#define VDD4P2_ENABLED

#define MAX_4P2_CAP_CHARGE_TIME 100000
#define BOOTUP_CHARGE_4P2_CURRENT_LIMIT 48
#define CAP_CHARGE_4P2_CURRENT_LIMIT 3
#define POWER_PREP_MINIMUM_VALID_BATTERY_BOOTUP_VOLTAGE 3200
#define MAX_BATTERY_CAPACITOR_SIZE_UF 100



#if 0
#define RUN_BATTERY_TEST
#define BATTERY_TEST_CHARGE_CURRENT_MA 50
#define BATTERY_TEST_CHARGE_VOLTAGE_THRESHOLD_MV 3900
#endif

/* #define DISABLE_VDDIO_BO_PROTECTION */

////////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////////
#if 0
bool bBatteryConnected = false;
#endif

bool bBatteryReady = false;
////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

bool IsVbusValid( void )
{
    return hw_power_GetVbusValid();
}

bool IsVdd5vGtVddio( void )
{
    return hw_power_GetVdd5vGtVddio();
}

bool Is5vPresent( void )
{
    return ( IsVbusValid() || IsVdd5vGtVddio() );
}

bool IsBattLevelValidForBoot( void )
{
	bool bBattLevelValid=false;

	if((HW_POWER_BATTMONITOR.B.BATT_VAL * 8) >=
		POWER_PREP_MINIMUM_VALID_BATTERY_BOOTUP_VOLTAGE)
	{
		bBattLevelValid=true;
	}

    return (bBattLevelValid);
}

void putc(char ch)
{
	int loop = 0;
	while (HW_UARTDBGFR_RD()&BM_UARTDBGFR_TXFF) {
		loop++;
		if (loop > 10000)
			break;
	};

	/* if(!(HW_UARTDBGFR_RD() &BM_UARTDBGFR_TXFF)) */
	HW_UARTDBGDR_WR(ch);
}
void printhex(int data)
{
	int i = 0;
	char c;
	for (i = sizeof(int)*2-1; i >= 0; i--) {
		c = data>>(i*4);
		c &= 0xf;
		if (c > 9)
			putc(c-10+'A');
		else
			putc(c+'0');
	}
}
void printf(char *fmt, ...)
{
	va_list args;
	int one;
	va_start(args, fmt);
	while (*fmt) {

		if (*fmt == '%') {
			fmt++;
			switch (*fmt) {

			case 'x':
			case 'X':
				printhex(va_arg(args, int));
				break;
			case '%':
				putc('%');
				break;
			default:
				break;
			}

		} else {
			putc(*fmt);
		}
		fmt++;
	}
	va_end(args);
}


////////////////////////////////////////////////////////////////////////////////
//! \brief  Prepares the power block for the application.
//!
//! \return SUCCESS(0) Power block ready.
////////////////////////////////////////////////////////////////////////////////
//int PowerPrep( void )
int _start( void )
{
	int iRtn = SUCCESS;

	printf("\r\n");
	printf("--- IMX-233: HW initialization ---\r\n");
	printf(__DATE__ __TIME__);
	printf("\r\n");
	printf("Version: %x.%x\r\n", POWER_PREP_VERSION_R, POWER_PREP_VERSION_RC);
	
	HW_DIGCTL_CTRL_SET(BM_DIGCTL_CTRL_USE_SERIAL_JTAG);
	PowerPrep_ClearAutoRestart();
	hw_power_SetPowerClkGate( false );

	HW_POWER_VDDDCTRL.B.LINREG_OFFSET = HW_POWER_LINREG_OFFSET_STEP_BELOW;
	HW_POWER_VDDACTRL.B.LINREG_OFFSET = HW_POWER_LINREG_OFFSET_STEP_BELOW;
	HW_POWER_VDDIOCTRL.B.LINREG_OFFSET = HW_POWER_LINREG_OFFSET_STEP_BELOW;

	// Ready the power block for 5V detection.
	PowerPrep_Setup5vDetect();
	PowerPrep_SetupBattDetect();

	// Ensure the power source that turned on the device is sufficient to
	// power the device.
	PowerPrep_ConfigurePowerSource();
	PowerPrep_EnableOutputRailProtection();

	/* set up either handoff or brownout */
	if(bBatteryReady)
	{
		/* disable hardware shutdown on loss of 5V */
		HW_POWER_5VCTRL_CLR(BM_POWER_5VCTRL_PWDN_5VBRNOUT);
	}
	/* 3.3V is necessary to achieve best power supply capability
	* and best EMI I/O performance.
	*/

	ddi_power_SetVddio(3300, 3150);

	printf("HW initialization: done\r\n\r\n");
	
	return iRtn;
}

/* clear RTC ALARM wakeup or AUTORESTART bits here. */
void PowerPrep_ClearAutoRestart( void )
{
	HW_RTC_CTRL_CLR( BM_ICOLL_CTRL_SFTRST );
	while( HW_RTC_CTRL.B.SFTRST == 1 );
	HW_RTC_CTRL_CLR( BM_ICOLL_CTRL_CLKGATE );
	while( HW_RTC_CTRL.B.CLKGATE == 1 );

	if(HW_RTC_PERSISTENT0.B.AUTO_RESTART==1)
	{
		while( BF_RD( RTC_STAT, NEW_REGS ) );
		HW_RTC_PERSISTENT0.B.AUTO_RESTART = 0;
		BF_SET(RTC_CTRL, FORCE_UPDATE);
		BF_CLR(RTC_CTRL, FORCE_UPDATE);
		while( BF_RD( RTC_STAT, NEW_REGS ) );
		while( BF_RD( RTC_STAT, STALE_REGS ) );
	}
}
////////////////////////////////////////////////////////////////////////////////
//! \brief
//!
//! \return
////////////////////////////////////////////////////////////////////////////////
int PowerPrep_ConfigurePowerSource( void )
{
    int iReturnValue = SUCCESS;

    /* initialize DCDC operating parameters */
    hw_power_Init();

    /* check if Battery Voltage is high enough for full
     * power operation.
     */
    bBatteryReady = PowerPrep_IsBatteryReady();

    if( IsVdd5vGtVddio() )
    {
        iReturnValue = PowerPrep_5vBoot();
    }
    else
    {
	    iReturnValue = PowerPrep_BattBoot();
    }

    /* Lastly, let's switch the Vddd power source to DCDC instead of
     * the linear regulator(linear regulator is the hardware default
     * configuration at bootup.
     */

    /* raise battery brownout level to programmed value. */
    PowerPrep_InitBattBo();

    PowerPrep_SwitchVdddToDdcdcSource();


    return iReturnValue;
}


void PowerPrep_PowerDown( void )
{
    #if 0
    // If chip is powering down unexpectedly, add this code
    // in to catch the chip shutdown before it occurs.
    volatile int Wait = 1;
    while(Wait);
    #endif

    hw_power_PowerDown();
}

void PowerPrep_Setup5vDetect( void )
{
    //---------------------------------------------------------------------------
    // Turn on the 5V detection hardware.
    //---------------------------------------------------------------------------
    hw_power_EnableVbusValid5vDetect( true );


    //---------------------------------------------------------------------------
    // Select the 5V detection methods and thresholds based on the chip.
    //---------------------------------------------------------------------------
    // 378x can use VBUSVALID for all 5V detection.  Use the normal threshold.
    hw_power_SetVbusValidThresh( VBUS_VALID_THRESH_4000 );



}

void PowerPrep_SetupBattDetect( void )
{
    extern bool bHwLradcInitialized;

    // Initialize LRADC for battery monitoring.
    bHwLradcInitialized = false;
    hw_lradc_Init(FALSE, LRADC_CLOCK_6MHZ);

    hw_lradc_EnableBatteryMeasurement( hw_lradc_AcquireBatteryMonitorDelayTrigger(),
                                       100);
}

#if 0
bool PowerPrep_BatteryDischargeTest( void )
{
	uint16_t starting_voltage = hw_power_GetBatteryVoltage();
	uint32_t counter, loaded_voltage;
	bool bTimedOut = false;


	HW_POWER_CHARGE.B.ENABLE_LOAD = 1;

	counter = hw_digctl_GetCurrentTime();

	/* with a load on 4p2 (and thus load on
	 * Vbat via the required external Schottky diode, check
	 * to see if voltage drops by >100mV in 10ms.  If so,
	 * we think that the battery connection is unreliable.
	 * The datasheet says that the ENABLE_LOAD causes a 100ohm
	 * load but one test showed a load ~450ohm.  Assuming a
	 * maximum resistance of 1kohm for now.
	 */
	while(!bTimedOut)
	{

		bTimedOut = hw_digctl_CheckTimeOut(counter, 10000);

		loaded_voltage = hw_power_GetBatteryVoltage();
		if( ( loaded_voltage => starting voltage ) ||
			(loaded_voltage - starting_voltage < 100) )
		{
			continue;
		}
		else
			exit;
	}

	return (bTimedOut);
}


void PowerPrep_BatteryConnectionChargeTest( void )
{



}
#endif

int PowerPrep_BootValid5v( void )
{

	/* now use VBUSVALID level instead of VDD5V_GT_VDDIO level
	 * to trigger a 5V disconnect event
	 */
	HW_POWER_5VCTRL_SET(BM_POWER_5VCTRL_VBUSVALID_5VDETECT);

	/* configure polarity to check for 5V disconnection.  This will
	 * give us indication if a VBUSVALID_IRQ event occurred occurred
	 * causing 4P2 rail to be disabled in hardware.  The kernel
	 * driver can then check the VBUSVALID_IRQ along with the DCDC4P2
	 * bit to know whether or not 4P2 is enabled
	 */
	HW_POWER_CTRL_CLR(BM_POWER_CTRL_POLARITY_VBUSVALID |
		BM_POWER_CTRL_POLARITY_VDD5V_GT_VDDIO );

	HW_POWER_CTRL_CLR(BM_POWER_CTRL_VBUSVALID_IRQ |
		BM_POWER_CTRL_VDD5V_GT_VDDIO_IRQ );

#if 0
	/* if battery voltage is so high that we can't reliably
	 * run a battery charging test to determine if the battery is
	 * connected or not, we have to run a dis-charge test.
	 */
	if(hw_power_GetBatteryVoltage() > 3300)
	{
		bBatteryConnected = PowerPrep_BatteryConnectionDischargeTest();
		bBatteryConnectionTested = true;
	}
#endif
	PowerPrep_Enable4p2();

#if 0
	if(!bBatteryConnectionTested)
	{
		bBatteryConnected = PowerPrep_BatteryConnectionChargeTest();
		bBatteryConnectionTested = true;
	}
#endif


	return SUCCESS;

}


void PowerPrep_CheckBatteryConnection( void )
{



}

void PowerPrep_Enable4p2( void )
{

	PowerPrep_Init4p2Parameters();

	PowerPrep_Init4p2Regulator();

	/* if battery isn't ready, go ahead and shutdown on a 4p2 brownout
	 * (because hardware battery brownout is disabled if 5V is present
	 */



	if(!bBatteryReady)
		PowerPrep_Enable4p2BoShutdown();
	else
	{
		/* in the case where 4p2 browns out, battery needs to
		 * take over powering the system.  We need to ensure
		 * the battery voltage level is sufficient to do so.
		 * Otherwise, the DCDC will keep sourcing from battery
		 * and 4p2 down to level below the safe minimum
		 * operating level of the DCDC.
		 */
		PowerPrep_InitBattBoFiq();
	}

	PowerPrep_InitDcdc4p2Source();

}

void PowerPrep_Init4p2Parameters( void )
{
	HW_POWER_DCDC4P2.B.CMPTRIP = 31;

	HW_POWER_DCDC4P2.B.TRG = 0;
	HW_POWER_5VCTRL.B.HEADROOM_ADJ = 0x4;
	HW_POWER_DCDC4P2.B.DROPOUT_CTRL = 0xA;  //100mV drop before stealing
		// charging current
	HW_POWER_5VCTRL.B.CHARGE_4P2_ILIMIT = 0x3f;
}


void PowerPrep_Init4p2Regulator( void )
{

	bool bMaxChargeTimeElapsed = false;
	// Enables but DCDC 4P2 logic.  This appears to be a necessary step not
	// only for DCDC operation from 4p2, but for the 4p2 rail itself to
	// start up properly.
	BF_SET(POWER_DCDC4P2, ENABLE_4P2);  //enable 5V to 4p2 regulator

	// This must be used if there is no static load on 4p2 as 4p2 will
	// become unstable with no load.
	BF_SET(POWER_CHARGE,ENABLE_LOAD);

	// Initialize 4p2 current limt before powering up 4p2
	HW_POWER_5VCTRL.B.CHARGE_4P2_ILIMIT = 0;

	HW_POWER_DCDC4P2.B.TRG = 0; // 4.2V

	// power up the 4p2 rail and logic/control
	hw_power_EnableMaster4p2( true );

	// Start charging up the 4p2 capacitor
	HW_POWER_5VCTRL.B.CHARGE_4P2_ILIMIT = CAP_CHARGE_4P2_CURRENT_LIMIT;

	HW_POWER_DCDC4P2.B.BO = 22; // 4.15V



	/* In PowerPrep, HW_POWER_5VCTRL.B.ENABLE_DCDC should always
	 * be 0 at this point.  But in case someone incorrectly
	 * changes this, we double check it here to make sure we
	 * don't try to turn on the DCDC from 4p2 before 4p2 voltage
	 * level is high enough.  Unfortunately, to measure 4p2 using the
	 * BO level, we must set the HW_POWER_DCDC4P2 ENABLE_DCDC bit first.
	 */
	if(HW_POWER_5VCTRL.B.ENABLE_DCDC == 0)
	{
		uint32_t u32StartTime = hw_digctl_GetCurrentTime();
		hw_power_Enable4p2DcdcInput(true);

		while(HW_POWER_STS.B.DCDC_4P2_BO && !bMaxChargeTimeElapsed)
		{
			bMaxChargeTimeElapsed = hw_digctl_CheckTimeOut(
				u32StartTime,MAX_4P2_CAP_CHARGE_TIME);
		}

		HW_POWER_5VCTRL.B.CHARGE_4P2_ILIMIT = BOOTUP_CHARGE_4P2_CURRENT_LIMIT;
		if(bMaxChargeTimeElapsed)
		{
			/* if we get here, there must be some existing
			 * load on the 4p2 rail.  Abandon attempts
			 * to charge up the rail as smoothly and
			 * quickly as possible and just give
			 * a long wait at relatively high
			 * charging current.
			 */
			hw_digctl_MicrosecondWait(10000);
		}
	}
	else
	{
		hw_digctl_MicrosecondWait(10000);
		HW_POWER_5VCTRL.B.CHARGE_4P2_ILIMIT = hw_power_ConvertSettingToCurrent(BOOTUP_CHARGE_4P2_CURRENT_LIMIT);
		hw_digctl_MicrosecondWait(10000);
	}

	/* we are finished using the BO level for 4p2 bring-up.  Lowering it
	 * back down to permanent value.
	 */
	HW_POWER_DCDC4P2.B.BO = 0; // 3.6V
}


/* This function will enable the 4p2 brownout to cause an FIQ.  The default
 * fiq handler in ROM will then cause a shutdown.
 */
void PowerPrep_Enable4p2BoShutdown( void )
{
	/* setting brownout to lowest value.  3.6V is plenty
	 * to power the system.  Going below there means there
	 * is a major problem with the 4p2 rail and so we need
	 * to shut down.
	 */
	HW_POWER_DCDC4P2.B.BO = 0;
	HW_POWER_CTRL_CLR(BM_POWER_CTRL_DCDC4P2_BO_IRQ);
	HW_POWER_CTRL_SET(BM_POWER_CTRL_ENIRQ_DCDC4P2_BO);

	PowerPrep_Enable4p2Fiq();
}

/* See errata iMX23 errata 5835.  The 5V disconnect problem is improved
 * by setting the 4p2 voltage level to equal the battery level.  When
 * testing a nominal part at room temperature (not sure if those
 * parameters affect the behavior), using a 12-ohm load on VDDIO, the VDDIO
 * voltage drop was less than 150mV.
 */
void PowerPrep_Handle5vDisconnectErrata( void )
{
	HW_POWER_DCDC4P2.B.BO=0;
	HW_POWER_DCDC4P2.B.TRG=4;
}


void PowerPrep_Enable4p2Fiq( void )
{

	HW_ICOLL_CTRL_CLR( BM_ICOLL_CTRL_SFTRST );
	while( HW_ICOLL_CTRL.B.SFTRST == 1 );
	HW_ICOLL_CTRL_CLR( BM_ICOLL_CTRL_CLKGATE );
	while( HW_ICOLL_CTRL.B.CLKGATE == 1 );

	hw_icoll_EnableVector(VECTOR_IRQ_DCDC4P2_BO , 0);
	hw_icoll_SetFiqMode(VECTOR_IRQ_DCDC4P2_BO , 1);
	hw_icoll_CtrlRegisterUpdate(ICOLL_CTRL_FIQ_FINAL_ENABLE, 1);
	hw_core_EnableFiqInterrupt(true);


	// For 378x, we must also enable the IRQ to enable the FIQ.
	hw_icoll_EnableVector(VECTOR_IRQ_DCDC4P2_BO , 1);

}


void PowerPrep_InitDcdc4p2Source( void )
{

	if(HW_POWER_DCDC4P2.B.ENABLE_DCDC == false)
	{
		hw_power_Enable4p2DcdcInput(true);
	}

	/* Enable Master 4p2 */
	hw_power_EnableDcdc( true );

	/* now that DCDC load is present, we can remove the 4p2 internal load */
	BF_CLR(POWER_CHARGE,ENABLE_LOAD);
}

int PowerPrep_5vBoot( void )
{
    if( IsVbusValid() && IsVdd5vGtVddio() )
    {
        // Both 5V detections see 5V.  Continue to boot device
        // with 5V.
        return PowerPrep_BootValid5v();
    }
    else
    {
	/* Delay before next check to give VDD5V more time to charge
	 * up */
	hw_digctl_MicrosecondWait(1000);
	if( IsVbusValid() && IsVdd5vGtVddio() )
	{
		return PowerPrep_BootValid5v();
	}
	else
	{
		// VBUS is not at a valid value, but VDD5V_GT_VDDIO is true.
		// This is a bad situation for the chip and we need to
		// handle it.
		return PowerPrep_Handle5vConflict();
	}
    }
}

int PowerPrep_Handle5vConflict( void )
{
    bool bLoop;

    // Need to check the VDDIO brownout level, so initialize it to maximum offset.
    HW_POWER_VDDIOCTRL.B.BO_OFFSET = BO_OFFSET_MAX;

    //--------------------------------------------------------------------------
    // The VBUSVALID and VDD5V_GT_VDDIO have different status
    // if we are here.  This is a bad state and we can't
    // continue to boot unless something changes.  So, we'll
    // stay in this loop.
    //--------------------------------------------------------------------------
    bLoop = true;
    while(bLoop)
    {
        //---------------------------------------------------------------------
        // Did output supply brownout?
        //---------------------------------------------------------------------
        if( HW_POWER_STS.B.VDDIO_BO )
        {
            // If VDDIO has a brownout, then the VDD5V_GT_VDDIO becomes
            // unreliable.  We need to shut down and try again.
            PowerPrep_PowerDown();
            return ERROR_VDDIO_BROWNOUT;
        }



        //---------------------------------------------------------------------
        // Did 5V change state?
        //---------------------------------------------------------------------
        if( IsVbusValid() && IsVdd5vGtVddio() )
        {
            // Both 5V detections see 5V.  Break out of loop and continue
            // to boot device with 5V and consider it a valid 5V.
            PowerPrep_BootValid5v();
            return SUCCESS;
        }

        if( Is5vPresent() == false )
        {
            // Neither 5V detection sees 5V.  It has dropped below
            // detection levels so we will shut down.
            PowerPrep_PowerDown();
            return ERROR_5V_OSCILLATING;
        }



        //---------------------------------------------------------------------
        // Was PSWITCH pressed?
        //---------------------------------------------------------------------
        if( HW_POWER_STS.B.PSWITCH > 0 )
        {
        	return PowerPrep_BattBoot();

        }
    } // end while(bLoop)

    return ERROR_UNEXPECTED_LOOP_EXIT;
}

int PowerPrep_InitBattBoFiq( void )
{

    HW_ICOLL_CTRL_CLR( BM_ICOLL_CTRL_SFTRST );
    while( HW_ICOLL_CTRL.B.SFTRST == 1 );
    HW_ICOLL_CTRL_CLR( BM_ICOLL_CTRL_CLKGATE );
    while( HW_ICOLL_CTRL.B.CLKGATE == 1 );

    hw_icoll_EnableVector(VECTOR_IRQ_BATT_BRNOUT , 0);
    hw_icoll_SetFiqMode(VECTOR_FIQ_BATT_BRNOUT , 1);
    hw_icoll_CtrlRegisterUpdate(ICOLL_CTRL_FIQ_FINAL_ENABLE, 1);
    hw_core_EnableFiqInterrupt(true);


    // For 378x, we must also enable the IRQ to enable the FIQ.
    hw_icoll_EnableVector(VECTOR_IRQ_BATT_BRNOUT , 1);


    return SUCCESS;
}

int PowerPrep_InitBattBo( void )
{
	// Set up soft battery brownout
	HW_POWER_BATTMONITOR.B.BRWNOUT_LVL = BATTERY_BRWNOUT_BITFIELD_VALUE;

	BF_CLR( POWER_CTRL, BATT_BO_IRQ );
	BF_SET( POWER_CTRL, ENIRQBATT_BO );


	return SUCCESS;

}

int PowerPrep_Init4p2Bo( void )
{
	// Set up 4p2 rail brownout


	// Set the 4.2V rail brownout to 3.6V.
	HW_POWER_DCDC4P2.B.BO = 0;

	// Enable the brownout.
	BF_CLR( POWER_CTRL, DCDC4P2_BO_IRQ );
	BF_SET( POWER_CTRL, ENIRQ_DCDC4P2_BO );



	return SUCCESS;

}

int PowerPrep_Init4p2BoFiq( void )
{


    // Enable the ICOLL block.
    HW_ICOLL_CTRL_CLR( BM_ICOLL_CTRL_SFTRST );
    while( HW_ICOLL_CTRL.B.SFTRST == 1 );
    HW_ICOLL_CTRL_CLR( BM_ICOLL_CTRL_CLKGATE );
    while( HW_ICOLL_CTRL.B.CLKGATE == 1 );

    // Setup the FIQ for 4.2V rail.
    hw_icoll_EnableVector(VECTOR_IRQ_DCDC4P2_BO , 0);
    hw_icoll_SetFiqMode(VECTOR_FIQ_DCDC4P2_BO , 1);
    hw_icoll_CtrlRegisterUpdate(ICOLL_CTRL_FIQ_FINAL_ENABLE, 1);
    hw_core_EnableFiqInterrupt(true);

    // For 378x, we must also enable the IRQ to enable the FIQ.
    hw_icoll_EnableVector(VECTOR_IRQ_DCDC4P2_BO , 1);


    return SUCCESS;
}

int PowerPrep_BattBoot( void )
{
    if(!bBatteryReady)
    {
	    /* wait until PSWITCH is no longer asserted to avoid
	     * multiple boots under this condition */
	    while((HW_POWER_STS.B.PSWITCH > 0 ) &&
		    !bBatteryReady)
	    {
		    bBatteryReady = PowerPrep_IsBatteryReady();
	    }

	    PowerPrep_PowerDown();
    }
    else
    {
	    /* enable battery brownout fiq to shutdown device if the battery
	     * voltage gets to low, even if 5V is present.
	     */
	    PowerPrep_InitBattBoFiq();

	    /* now that protection is enabled, we can startup the DCDC converter */
	    HW_POWER_5VCTRL_SET(BM_POWER_5VCTRL_ENABLE_DCDC);
    }

    return SUCCESS;
}

void PowerPrep_SwitchVdddToDdcdcSource( void )
{
	/* This should have already been done, but just in case */
	HW_POWER_VDDDCTRL.B.LINREG_OFFSET = HW_POWER_LINREG_OFFSET_STEP_BELOW;

	/* Enabe DCDC output to Vddd rail */
	HW_POWER_VDDDCTRL.B.DISABLE_FET = 0;
	/* For maximum stability on the Vddd rail, we will leave the
	 * Vddd linreg forced on (even under battery power).  The linreg
	 * will only regulate if the target voltage get 25mV below the
	 * DCDC target
	 */
	HW_POWER_VDDDCTRL.B.ENABLE_LINREG = 0;
	HW_POWER_VDDDCTRL.B.DISABLE_STEPPING = 0;

}

void PowerPrep_EnableOutputRailProtection( void )
{


	HW_POWER_CTRL_CLR(BM_POWER_CTRL_VDDD_BO_IRQ |
			BM_POWER_CTRL_VDDA_BO_IRQ |
			BM_POWER_CTRL_VDDIO_BO_IRQ);

	HW_POWER_VDDDCTRL.B.PWDN_BRNOUT = 1;
	HW_POWER_VDDACTRL.B.PWDN_BRNOUT = 1;

	/* due to imx23 errata 5835, we there is a possibility
	 * of large VDDIO droops in on a 5V disconnect.  If one
	 * occurs, we need to shutdown to protect the system
	 * against unknown conditions
	 */

	/* note that VDDIO brownout indicator has been found to falsely
	 * trigger due to a 5V connection.  Possibly also due to a pswitch
	 * press.
	 */
#ifndef	DISABLE_VDDIO_BO_PROTECTION
	HW_POWER_VDDIOCTRL.B.PWDN_BRNOUT = 1;
#endif

}

bool PowerPrep_IsBatteryReady( void )
{
    if(hw_power_GetBatteryVoltage() >=
		    MINIMUM_SAFE_BOOTING_BATTERY_VOLTAGE_MV)
	    return true;
    else
	    return false;
}

// eof power_prep.c
//! @}

/* kiss gcc's ass to make it happy */
//void __aeabi_unwind_cpp_pr0() {}
void __aeabi_unwind_cpp_pr1() {}
