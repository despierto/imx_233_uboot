/*
 * iMX233 HW memory init
 *
 * Copyright 2008-2009 Freescale Semiconductor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <stdarg.h>
#include "registers/regsclkctrl.h"
#include "registers/regsemi.h"
#include "registers/regsdram.h"
#include "registers/regspower.h"
#include "registers/regsuartdbg.h"
#include "registers/regspinctrl.h"
#include "registers/regsdigctl.h"
#include "registers/regsocotp.h"
#include "drv_utils.h"
#include "platform.h"
#include "sys_utils.h"

#define PIN_DRIVE_12mA 2

void init_ddr_mt46v32m16_133Mhz(int ce)
{
    HW_DRAM_CTL00_WR(0x01010001);
    HW_DRAM_CTL01_WR(0x00010100);
    HW_DRAM_CTL02_WR(0x01000101);
    HW_DRAM_CTL03_WR(0x00000001);
    HW_DRAM_CTL04_WR(0x00000101);
    HW_DRAM_CTL05_WR(0x00000000);
    HW_DRAM_CTL06_WR(0x00010000);
    HW_DRAM_CTL07_WR(0x01000001);
    HW_DRAM_CTL09_WR(0x00000001);
    HW_DRAM_CTL10_WR(0x07000200);
    HW_DRAM_CTL11_WR(0x00070202);
    HW_DRAM_CTL12_WR(0x02020000);
    //pi    HW_DRAM_CTL13_WR(0x04040a01);
    HW_DRAM_CTL13_WR(0x04040a01);//CAS latency
    HW_DRAM_CTL14_WR(0x00000200|ce);
    HW_DRAM_CTL15_WR(0x02040000);
    HW_DRAM_CTL16_WR(0x02000000);
    HW_DRAM_CTL17_WR(0x19000f08);
    //pi    HW_DRAM_CTL18_WR(0x0d0d0000);
    HW_DRAM_CTL18_WR(0x1f1f0000);
    HW_DRAM_CTL19_WR(0x02021313);
    HW_DRAM_CTL20_WR(0x02061521);
    HW_DRAM_CTL21_WR(0x0000000a);
    HW_DRAM_CTL22_WR(0x00080008);
    HW_DRAM_CTL23_WR(0x00200020);
    HW_DRAM_CTL24_WR(0x00200020);
    HW_DRAM_CTL25_WR(0x00200020);
    HW_DRAM_CTL26_WR(0x000003f7);
    HW_DRAM_CTL29_WR(0x00000020);
    HW_DRAM_CTL30_WR(0x00000020);
    HW_DRAM_CTL31_WR(0x00c80000);
    HW_DRAM_CTL32_WR(0x000a23cd);
    HW_DRAM_CTL33_WR(0x000000c8);
    HW_DRAM_CTL34_WR(0x00006665);
    HW_DRAM_CTL36_WR(0x00000101);
    HW_DRAM_CTL37_WR(0x00040001);
    HW_DRAM_CTL38_WR(0x00000000);
    HW_DRAM_CTL39_WR(0x00000000);
    HW_DRAM_CTL40_WR(0x00010000);
    HW_DRAM_CTL08_WR(0x01000000);
}

void poweron_pll(void)
{
    HW_CLKCTRL_PLLCTRL0_SET(BM_CLKCTRL_PLLCTRL0_POWER);
}
void turnon_mem_rail(int mv)
{
    unsigned int value;

    print_hw("%s", " - Clock gate enable");
    HW_POWER_CTRL_CLR(BM_POWER_CTRL_CLKGATE);

    value = BM_POWER_VDDMEMCTRL_ENABLE_ILIMIT|
        BM_POWER_VDDMEMCTRL_ENABLE_LINREG|
        BM_POWER_VDDMEMCTRL_PULLDOWN_ACTIVE|
        (mv-1700)/50;

    HW_POWER_VDDMEMCTRL_WR(value);
    delay(20000);
    value &= ~(BM_POWER_VDDMEMCTRL_ENABLE_ILIMIT|
        BM_POWER_VDDMEMCTRL_PULLDOWN_ACTIVE);
    HW_POWER_VDDMEMCTRL_WR(value);
   
    print_hw(" - VDDMEM level is %d mV", mv); 
}
void set_emi_frac(unsigned int div)
{
    HW_CLKCTRL_FRAC_SET(BM_CLKCTRL_FRAC_EMIFRAC);
    div = (~div);
    HW_CLKCTRL_FRAC_CLR(BF_CLKCTRL_FRAC_EMIFRAC(div));
}
void init_clock(void)
{
    HW_CLKCTRL_FRAC_SET(BM_CLKCTRL_FRAC_CLKGATEEMI);
    set_emi_frac(33);
    HW_CLKCTRL_FRAC_CLR(BM_CLKCTRL_FRAC_CLKGATEEMI);
    delay(11000);

    HW_CLKCTRL_EMI_WR(BF_CLKCTRL_EMI_DIV_XTAL(1)|
            BF_CLKCTRL_EMI_DIV_EMI(2)
            );

    /*choose ref_emi*/
    HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_EMI);

    /*Reset EMI*/
    HW_EMI_CTRL_CLR(BM_EMI_CTRL_SFTRST);
    HW_EMI_CTRL_CLR(BM_EMI_CTRL_CLKGATE);
    
    print_hw(" - EMI CTRL 0x%x",  HW_EMI_CTRL_RD()); 
    print_hw(" - Fractional Clock CTRL  0x%x",  HW_CLKCTRL_FRAC_RD());     
}

void disable_emi_padkeepers(void)
{
    HW_PINCTRL_CTRL_CLR(BM_PINCTRL_CTRL_SFTRST | BM_PINCTRL_CTRL_CLKGATE);

    HW_PINCTRL_PULL3_SET(
    BM_PINCTRL_PULL3_BANK3_PIN17 |
    BM_PINCTRL_PULL3_BANK3_PIN16 |
    BM_PINCTRL_PULL3_BANK3_PIN15 |
    BM_PINCTRL_PULL3_BANK3_PIN14 |
    BM_PINCTRL_PULL3_BANK3_PIN13 |
    BM_PINCTRL_PULL3_BANK3_PIN12 |
    BM_PINCTRL_PULL3_BANK3_PIN11 |
    BM_PINCTRL_PULL3_BANK3_PIN10 |
    BM_PINCTRL_PULL3_BANK3_PIN09 |
    BM_PINCTRL_PULL3_BANK3_PIN08 |
    BM_PINCTRL_PULL3_BANK3_PIN07 |
    BM_PINCTRL_PULL3_BANK3_PIN06 |
    BM_PINCTRL_PULL3_BANK3_PIN05 |
    BM_PINCTRL_PULL3_BANK3_PIN04 |
    BM_PINCTRL_PULL3_BANK3_PIN03 |
    BM_PINCTRL_PULL3_BANK3_PIN02 |
    BM_PINCTRL_PULL3_BANK3_PIN01 |
    BM_PINCTRL_PULL3_BANK3_PIN00);

}

#define PIN_VOL(pin , v) ((v) ? (pin) : 0)
void init_emi_pin(int pin_voltage, int pin_drive )
{
    HW_PINCTRL_CTRL_CLR(BM_PINCTRL_CTRL_SFTRST | BM_PINCTRL_CTRL_CLKGATE);
    /* EMI_A00-06 */
    /* Configure Bank-2 Pins 9-15 voltage and drive strength*/
    HW_PINCTRL_DRIVE9_CLR(
        BM_PINCTRL_DRIVE9_BANK2_PIN09_V |
        BM_PINCTRL_DRIVE9_BANK2_PIN09_MA |
        BM_PINCTRL_DRIVE9_BANK2_PIN10_V |
        BM_PINCTRL_DRIVE9_BANK2_PIN10_MA |
        BM_PINCTRL_DRIVE9_BANK2_PIN11_V |
        BM_PINCTRL_DRIVE9_BANK2_PIN11_MA |
        BM_PINCTRL_DRIVE9_BANK2_PIN12_V |
        BM_PINCTRL_DRIVE9_BANK2_PIN12_MA |
        BM_PINCTRL_DRIVE9_BANK2_PIN13_V |
        BM_PINCTRL_DRIVE9_BANK2_PIN13_MA |
        BM_PINCTRL_DRIVE9_BANK2_PIN14_V |
        BM_PINCTRL_DRIVE9_BANK2_PIN14_MA |
        BM_PINCTRL_DRIVE9_BANK2_PIN15_V |
        BM_PINCTRL_DRIVE9_BANK2_PIN15_MA);

    HW_PINCTRL_DRIVE9_SET(
        PIN_VOL(BM_PINCTRL_DRIVE9_BANK2_PIN09_V , pin_voltage) |
        BF_PINCTRL_DRIVE9_BANK2_PIN09_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE9_BANK2_PIN10_V , pin_voltage) |
        BF_PINCTRL_DRIVE9_BANK2_PIN10_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE9_BANK2_PIN11_V , pin_voltage) |
        BF_PINCTRL_DRIVE9_BANK2_PIN11_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE9_BANK2_PIN12_V , pin_voltage) |
        BF_PINCTRL_DRIVE9_BANK2_PIN12_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE9_BANK2_PIN13_V , pin_voltage) |
        BF_PINCTRL_DRIVE9_BANK2_PIN13_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE9_BANK2_PIN14_V , pin_voltage) |
        BF_PINCTRL_DRIVE9_BANK2_PIN14_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE9_BANK2_PIN15_V , pin_voltage) |
        BF_PINCTRL_DRIVE9_BANK2_PIN15_MA(pin_drive));

    /* EMI_A07-12, EMI_BA0-1 */
    /* Configure Bank-2 Pins 16-23 voltage and drive strength */
    HW_PINCTRL_DRIVE10_CLR(
        BM_PINCTRL_DRIVE10_BANK2_PIN16_V |
        BM_PINCTRL_DRIVE10_BANK2_PIN16_MA |
        BM_PINCTRL_DRIVE10_BANK2_PIN17_V |
        BM_PINCTRL_DRIVE10_BANK2_PIN17_MA |
        BM_PINCTRL_DRIVE10_BANK2_PIN18_V |
        BM_PINCTRL_DRIVE10_BANK2_PIN18_MA |
        BM_PINCTRL_DRIVE10_BANK2_PIN19_V |
        BM_PINCTRL_DRIVE10_BANK2_PIN19_MA |
        BM_PINCTRL_DRIVE10_BANK2_PIN20_V |
        BM_PINCTRL_DRIVE10_BANK2_PIN20_MA |
        BM_PINCTRL_DRIVE10_BANK2_PIN21_V |
        BM_PINCTRL_DRIVE10_BANK2_PIN21_MA |
        BM_PINCTRL_DRIVE10_BANK2_PIN22_V |
        BM_PINCTRL_DRIVE10_BANK2_PIN22_MA |
        BM_PINCTRL_DRIVE10_BANK2_PIN23_V |
        BM_PINCTRL_DRIVE10_BANK2_PIN23_MA);

    HW_PINCTRL_DRIVE10_SET(
        PIN_VOL(BM_PINCTRL_DRIVE10_BANK2_PIN16_V , pin_voltage) |
        BF_PINCTRL_DRIVE10_BANK2_PIN16_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE10_BANK2_PIN17_V , pin_voltage) |
        BF_PINCTRL_DRIVE10_BANK2_PIN17_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE10_BANK2_PIN18_V , pin_voltage) |
        BF_PINCTRL_DRIVE10_BANK2_PIN18_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE10_BANK2_PIN19_V , pin_voltage) |
        BF_PINCTRL_DRIVE10_BANK2_PIN19_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE10_BANK2_PIN20_V , pin_voltage) |
        BF_PINCTRL_DRIVE10_BANK2_PIN20_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE10_BANK2_PIN21_V , pin_voltage) |
        BF_PINCTRL_DRIVE10_BANK2_PIN21_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE10_BANK2_PIN22_V , pin_voltage) |
        BF_PINCTRL_DRIVE10_BANK2_PIN22_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE10_BANK2_PIN23_V , pin_voltage) |
        BF_PINCTRL_DRIVE10_BANK2_PIN23_MA(pin_drive));

    /* EMI_CAS,RAS,CE0-2,WEN,CKE */
    /* Configure Bank-2 Pins 24-31 voltage and drive strength */
    HW_PINCTRL_DRIVE11_CLR(
        BM_PINCTRL_DRIVE11_BANK2_PIN24_V |
        BM_PINCTRL_DRIVE11_BANK2_PIN24_MA |
        BM_PINCTRL_DRIVE11_BANK2_PIN25_V |
        BM_PINCTRL_DRIVE11_BANK2_PIN25_MA |
        BM_PINCTRL_DRIVE11_BANK2_PIN26_V |
        BM_PINCTRL_DRIVE11_BANK2_PIN26_MA |
        BM_PINCTRL_DRIVE11_BANK2_PIN29_V |
        BM_PINCTRL_DRIVE11_BANK2_PIN29_MA |
        BM_PINCTRL_DRIVE11_BANK2_PIN30_V |
        BM_PINCTRL_DRIVE11_BANK2_PIN30_MA |
        BM_PINCTRL_DRIVE11_BANK2_PIN31_V |
        BM_PINCTRL_DRIVE11_BANK2_PIN31_MA);

    HW_PINCTRL_DRIVE11_SET(
        PIN_VOL(BM_PINCTRL_DRIVE11_BANK2_PIN24_V , pin_voltage) |
        BF_PINCTRL_DRIVE11_BANK2_PIN24_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE11_BANK2_PIN25_V , pin_voltage) |
        BF_PINCTRL_DRIVE11_BANK2_PIN25_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE11_BANK2_PIN26_V , pin_voltage) |
        BF_PINCTRL_DRIVE11_BANK2_PIN26_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE11_BANK2_PIN29_V , pin_voltage) |
        BF_PINCTRL_DRIVE11_BANK2_PIN29_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE11_BANK2_PIN30_V , pin_voltage) |
        BF_PINCTRL_DRIVE11_BANK2_PIN30_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE11_BANK2_PIN31_V , pin_voltage) |
        BF_PINCTRL_DRIVE11_BANK2_PIN31_MA(pin_drive));

    /* Configure Bank-2 Pins 9-15 as EMI pins */
    HW_PINCTRL_MUXSEL4_CLR(
        BM_PINCTRL_MUXSEL4_BANK2_PIN09 |
        BM_PINCTRL_MUXSEL4_BANK2_PIN10 |
        BM_PINCTRL_MUXSEL4_BANK2_PIN11 |
        BM_PINCTRL_MUXSEL4_BANK2_PIN12 |
        BM_PINCTRL_MUXSEL4_BANK2_PIN13 |
        BM_PINCTRL_MUXSEL4_BANK2_PIN14 |
        BM_PINCTRL_MUXSEL4_BANK2_PIN15);

    /* Configure Bank-2 Pins 16-31 as EMI pins */
    HW_PINCTRL_MUXSEL5_CLR(
        BM_PINCTRL_MUXSEL5_BANK2_PIN16 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN17 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN18 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN19 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN20 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN21 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN22 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN23 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN24 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN25 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN26 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN29 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN30 |
        BM_PINCTRL_MUXSEL5_BANK2_PIN31);

    HW_PINCTRL_DRIVE12_CLR(
        BM_PINCTRL_DRIVE12_BANK3_PIN00_V |
        BM_PINCTRL_DRIVE12_BANK3_PIN00_MA |
        BM_PINCTRL_DRIVE12_BANK3_PIN01_V |
        BM_PINCTRL_DRIVE12_BANK3_PIN01_MA |
        BM_PINCTRL_DRIVE12_BANK3_PIN02_V |
        BM_PINCTRL_DRIVE12_BANK3_PIN02_MA |
        BM_PINCTRL_DRIVE12_BANK3_PIN03_V |
        BM_PINCTRL_DRIVE12_BANK3_PIN03_MA |
        BM_PINCTRL_DRIVE12_BANK3_PIN04_V |
        BM_PINCTRL_DRIVE12_BANK3_PIN04_MA |
        BM_PINCTRL_DRIVE12_BANK3_PIN05_V |
        BM_PINCTRL_DRIVE12_BANK3_PIN05_MA |
        BM_PINCTRL_DRIVE12_BANK3_PIN06_V |
        BM_PINCTRL_DRIVE12_BANK3_PIN06_MA |
        BM_PINCTRL_DRIVE12_BANK3_PIN07_V |
        BM_PINCTRL_DRIVE12_BANK3_PIN07_MA);

    HW_PINCTRL_DRIVE12_SET(
        PIN_VOL(BM_PINCTRL_DRIVE12_BANK3_PIN00_V , pin_voltage) |
        BF_PINCTRL_DRIVE12_BANK3_PIN00_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE12_BANK3_PIN01_V , pin_voltage) |
        BF_PINCTRL_DRIVE12_BANK3_PIN01_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE12_BANK3_PIN02_V , pin_voltage) |
        BF_PINCTRL_DRIVE12_BANK3_PIN02_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE12_BANK3_PIN03_V , pin_voltage) |
        BF_PINCTRL_DRIVE12_BANK3_PIN03_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE12_BANK3_PIN04_V , pin_voltage) |
        BF_PINCTRL_DRIVE12_BANK3_PIN04_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE12_BANK3_PIN05_V , pin_voltage) |
        BF_PINCTRL_DRIVE12_BANK3_PIN05_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE12_BANK3_PIN06_V , pin_voltage) |
        BF_PINCTRL_DRIVE12_BANK3_PIN06_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE12_BANK3_PIN07_V , pin_voltage) |
        BF_PINCTRL_DRIVE12_BANK3_PIN07_MA(pin_drive));

    /* EMI_D08-15
      Configure Bank-3 Pins 08-15 voltage and drive strength
    */
    HW_PINCTRL_DRIVE13_CLR(
        BM_PINCTRL_DRIVE13_BANK3_PIN08_V |
        BM_PINCTRL_DRIVE13_BANK3_PIN08_MA |
        BM_PINCTRL_DRIVE13_BANK3_PIN09_V |
        BM_PINCTRL_DRIVE13_BANK3_PIN09_MA |
        BM_PINCTRL_DRIVE13_BANK3_PIN10_V |
        BM_PINCTRL_DRIVE13_BANK3_PIN10_MA |
        BM_PINCTRL_DRIVE13_BANK3_PIN11_V |
        BM_PINCTRL_DRIVE13_BANK3_PIN11_MA |
        BM_PINCTRL_DRIVE13_BANK3_PIN12_V |
        BM_PINCTRL_DRIVE13_BANK3_PIN12_MA |
        BM_PINCTRL_DRIVE13_BANK3_PIN13_V |
        BM_PINCTRL_DRIVE13_BANK3_PIN13_MA |
        BM_PINCTRL_DRIVE13_BANK3_PIN14_V |
        BM_PINCTRL_DRIVE13_BANK3_PIN14_MA |
        BM_PINCTRL_DRIVE13_BANK3_PIN15_V |
        BM_PINCTRL_DRIVE13_BANK3_PIN15_MA);

    HW_PINCTRL_DRIVE13_SET(
        PIN_VOL(BM_PINCTRL_DRIVE13_BANK3_PIN08_V , pin_voltage) |
        BF_PINCTRL_DRIVE13_BANK3_PIN08_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE13_BANK3_PIN09_V , pin_voltage) |
        BF_PINCTRL_DRIVE13_BANK3_PIN09_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE13_BANK3_PIN10_V , pin_voltage) |
        BF_PINCTRL_DRIVE13_BANK3_PIN10_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE13_BANK3_PIN11_V , pin_voltage) |
        BF_PINCTRL_DRIVE13_BANK3_PIN11_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE13_BANK3_PIN12_V , pin_voltage) |
        BF_PINCTRL_DRIVE13_BANK3_PIN12_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE13_BANK3_PIN13_V , pin_voltage) |
        BF_PINCTRL_DRIVE13_BANK3_PIN13_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE13_BANK3_PIN14_V , pin_voltage) |
        BF_PINCTRL_DRIVE13_BANK3_PIN14_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE13_BANK3_PIN15_V , pin_voltage) |
        BF_PINCTRL_DRIVE13_BANK3_PIN15_MA(pin_drive));

    /* EMI_DQS0-1,DQM0-1,CLK,CLKN
       Configure Bank-3 Pins 08-15 voltage and drive strength
     */
    HW_PINCTRL_DRIVE14_CLR(
        BM_PINCTRL_DRIVE14_BANK3_PIN16_V  |
        BM_PINCTRL_DRIVE14_BANK3_PIN16_MA |
        BM_PINCTRL_DRIVE14_BANK3_PIN17_V  |
        BM_PINCTRL_DRIVE14_BANK3_PIN17_MA |
        BM_PINCTRL_DRIVE14_BANK3_PIN18_V  |
        BM_PINCTRL_DRIVE14_BANK3_PIN18_MA |
        BM_PINCTRL_DRIVE14_BANK3_PIN19_V  |
        BM_PINCTRL_DRIVE14_BANK3_PIN19_MA |
        BM_PINCTRL_DRIVE14_BANK3_PIN20_V  |
        BM_PINCTRL_DRIVE14_BANK3_PIN20_MA |
        BM_PINCTRL_DRIVE14_BANK3_PIN21_V  |
        BM_PINCTRL_DRIVE14_BANK3_PIN21_MA);

    HW_PINCTRL_DRIVE14_SET(
        PIN_VOL(BM_PINCTRL_DRIVE14_BANK3_PIN16_V , pin_voltage) |
        BF_PINCTRL_DRIVE14_BANK3_PIN16_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE14_BANK3_PIN17_V , pin_voltage) |
        BF_PINCTRL_DRIVE14_BANK3_PIN17_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE14_BANK3_PIN18_V , pin_voltage) |
        BF_PINCTRL_DRIVE14_BANK3_PIN18_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE14_BANK3_PIN19_V , pin_voltage) |
        BF_PINCTRL_DRIVE14_BANK3_PIN19_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE14_BANK3_PIN20_V , pin_voltage) |
        BF_PINCTRL_DRIVE14_BANK3_PIN20_MA(pin_drive) |
        PIN_VOL(BM_PINCTRL_DRIVE14_BANK3_PIN21_V , pin_voltage) |
        BF_PINCTRL_DRIVE14_BANK3_PIN21_MA(pin_drive));

    /* Configure Bank-3 Pins 0-15 as EMI pins*/
    HW_PINCTRL_MUXSEL6_CLR(
        BM_PINCTRL_MUXSEL6_BANK3_PIN00 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN01 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN02 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN03 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN04 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN05 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN06 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN07 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN08 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN09 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN10 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN11 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN12 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN13 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN14 |
        BM_PINCTRL_MUXSEL6_BANK3_PIN15);

    /* Configure Bank-3 Pins 16-21 as EMI pins */
    HW_PINCTRL_MUXSEL7_CLR(
        BM_PINCTRL_MUXSEL7_BANK3_PIN16 |
        BM_PINCTRL_MUXSEL7_BANK3_PIN17 |
        BM_PINCTRL_MUXSEL7_BANK3_PIN18 |
        BM_PINCTRL_MUXSEL7_BANK3_PIN19 |
        BM_PINCTRL_MUXSEL7_BANK3_PIN20 |
        BM_PINCTRL_MUXSEL7_BANK3_PIN21);
}
void exit_selfrefresh(void)
{
    unsigned int start, curr;
    unsigned int value;
    value = HW_DRAM_CTL16_RD();
    value &= ~(1<<17);
    HW_DRAM_CTL16_WR(value);

    start = HW_DIGCTL_MICROSECONDS_RD();

    while ((HW_EMI_STAT_RD()&BM_EMI_STAT_DRAM_HALTED)) {

        if ((curr = HW_DIGCTL_MICROSECONDS_RD()) > (start + 1000000)) {
            print_err("exit self refresh timeout (diff 0x%x)\r\n", curr - (start + 1000000) );
            return;
        }
    }

    print_hw("%s", " - DDR selfrefresh successfully completed");
    
    return;
}

void set_port_priority(void)
{
    unsigned int value;

    HW_EMI_CTRL_CLR(BM_EMI_CTRL_PORT_PRIORITY_ORDER);
    HW_EMI_CTRL_SET(BF_EMI_CTRL_PORT_PRIORITY_ORDER(
            BV_EMI_CTRL_PORT_PRIORITY_ORDER__PORT1230)
            );

    HW_EMI_CTRL_CLR(BM_EMI_CTRL_PORT_PRIORITY_ORDER);
    HW_EMI_CTRL_SET(BF_EMI_CTRL_PORT_PRIORITY_ORDER(0x2));
}
void entry_auto_clock_gate(void)
{
    unsigned int value;
    value =  HW_DRAM_CTL16_RD();
    value |= 1<<19;
    HW_DRAM_CTL16_WR(value);

    value =  HW_DRAM_CTL16_RD();
    value |= 1<<11;
    HW_DRAM_CTL16_WR(value);
}

void change_cpu_freq(void)
{
    int value = 0;

    //set VDDD power level
    value = HW_POWER_VDDDCTRL_RD();
    if ((value & BM_POWER_VDDDCTRL_TRG) != 27) {
        print_hw(" - Set VDDD level from %d mV to %d mV", 
                (value & BM_POWER_VDDDCTRL_TRG)*25 + 800,
                27*25 + 800);        
        value &= ~BM_POWER_VDDDCTRL_TRG;
        value |= BF_POWER_VDDDCTRL_TRG(27); //27 = 1.475V max is 31 == 1.575V
        HW_POWER_VDDDCTRL_WR(value);
        delay(10000);
    }
    else
    {
        print_hw(" - VDDD level is %d mV", (value & BM_POWER_VDDDCTRL_TRG)*25 + 800); 
    }

    //set CPU frequency
    value = HW_CLKCTRL_FRAC_RD();
    if (((value & BM_CLKCTRL_FRAC_CPUFRAC) != CPU_CLK_DEVIDER)||(value & BM_CLKCTRL_FRAC_CLKGATECPU)) {
        print_hw(" - Set CPU frequency from %d Hz to %d Hz", 
                480*(18*1000000/(value & BM_CLKCTRL_FRAC_CPUFRAC)),
                480*(18*1000000/CPU_CLK_DEVIDER));        
        value &= ~BM_CLKCTRL_FRAC_CPUFRAC;
        value |= BF_CLKCTRL_FRAC_CPUFRAC(CPU_CLK_DEVIDER); //19 = 454Mhz 18 = 480MHz
        value &= ~BM_CLKCTRL_FRAC_CLKGATECPU;
        HW_CLKCTRL_FRAC_WR(value);
    }
    else
    {
        print_hw(" - CPU frequency is %d Hz", 480*(18*1000000/(value & BM_CLKCTRL_FRAC_CPUFRAC))); 
    }

    HW_CLKCTRL_CLKSEQ_SET(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);        
    HW_CLKCTRL_HBUS_SET(BM_CLKCTRL_HBUS_DIV);
    HW_CLKCTRL_HBUS_CLR(((~3)&BM_CLKCTRL_HBUS_DIV));

    delay(10000);

    HW_CLKCTRL_CLKSEQ_CLR(BM_CLKCTRL_CLKSEQ_BYPASS_CPU);

    print_hw(" - HBUS frequency is %d Hz", (80*(18*1000000/(value & BM_CLKCTRL_FRAC_CPUFRAC)))/(HW_CLKCTRL_HBUS_RD()&BM_CLKCTRL_HBUS_DIV)); 
}

int hw_mem_test(void)
{
    volatile int *pTest = (volatile int *)SDRAM_BASE;
    int i, j, k;    
    unsigned int mem_test_size = 1000; //SDRAM_SIZE

#if 0
    printf("\r\n\r\n---------------------------\r\n");
    printf("Show REGS_DRAM_CTL before test:\r\n");
    for (i = 0; i <= 40; i++) {
        printf("mem 0x%x:  0x%x\r\n",
                (REGS_DRAM_BASE + i * 4), 
                *(volatile int*)(REGS_DRAM_BASE + i * 4));
    }
#endif

    print_hw(" - DDR base: 0x%X size %d bytes", SDRAM_BASE, SDRAM_SIZE);
    printf("[hw]  - DDR memory diagnostics: size %d bytes... ", mem_test_size);

    for (i = 0; i < (mem_test_size/4); i++)
        *pTest++ = i;

    pTest = (volatile int *)SDRAM_BASE;

    j = 0;
    for (i = 0; i < (mem_test_size/4); i++) {
        if (*pTest != (i)) {
            j++;
            print_err("Addr 0x%x has error value 0x%x, expected 0x%x", 
                (unsigned int)pTest, *pTest, i);
        }
        pTest++;
    }

    memset((void *)SDRAM_BASE, 0, mem_test_size);
    pTest = (volatile int *)SDRAM_BASE;
    
    k = 0;
    for (i = 0; i < (mem_test_size/4); i++) {
        if (*pTest != (0)) {
            k++;
            print_err("Addr 0x%x has error value 0x%x, expected 0x%x", 
                (unsigned int)pTest, *pTest, 0);
        }
        pTest++;
    }        

    if (j || k)
        printf("completed with errors: i%d z%d 32bit words are broken from %d tested\r\n", j, k, (mem_test_size/4));
    else
        printf("done\r\n");

#if 0
    printf("\r\n\r\n---------------------------\r\n");
    printf("Show REGS_DRAM_CTL after test:\r\n");
    for (i = 0; i <= 40; i++) {
        printf("mem 0x%x:  0x%x\r\n",
                (REGS_DRAM_BASE + i * 4), 
                *(volatile int*)(REGS_DRAM_BASE + i * 4));
    }
#endif

    return 0;    
}
    
int hw_mem_init(void)
{
    unsigned int value;
    unsigned int CE = 0x1;

    print_hw("%s", "Memory initialization");

    /*increas VDDIO from reset level 1.75 to 1.8v*/
    value = HW_POWER_VDDACTRL_RD() ;
    if ((value & BM_POWER_VDDACTRL_TRG) != 12) {
        print_hw(" - Set VDDA level from %d mV to %d mV", 
                (value & BM_POWER_VDDACTRL_TRG)*25 + 1500,
                12*25 + 1500);        
        value &= ~BM_POWER_VDDACTRL_TRG;
        value |= 12;
        HW_POWER_VDDACTRL_WR(value);
        delay(20000);
    }
    else
    {
        print_hw(" - VDDA level is %d mV", (value & BM_POWER_VDDACTRL_TRG)*25 + 1500); 
    }

    /*increass VDDIO 3.3v */
    value = HW_POWER_VDDIOCTRL_RD();
    if ((value & BM_POWER_VDDIOCTRL_TRG) != 20) {
        print_hw(" - Set VDDIO level from %d mV to %d mV", 
                (value & BM_POWER_VDDIOCTRL_TRG)*25 + 2800,
                12*25 + 2800);        
        value &= ~BM_POWER_VDDIOCTRL_TRG;
        value |= 20;
        HW_POWER_VDDIOCTRL_WR(value);
        delay(20000);
    }
    else
    {
        print_hw(" - VDDIO level is %d mV", (value & BM_POWER_VDDIOCTRL_TRG)*25 + 2800); 
    }

    print_hw("%s", " - PLL power enable");
    poweron_pll();
    delay(11000);

    turnon_mem_rail(2500);
    delay(11000);

    print_hw("%s", " - Init EMI pins");
    init_emi_pin(0, PIN_DRIVE_12mA);

    disable_emi_padkeepers();

    init_clock();

    delay(10000);

    init_ddr_mt46v32m16_133Mhz(CE);
    print_hw("%s", " - DDR mt46v32m16 configured for 133Mhz");

    print_hw("%s", " - Initiate active mode for the memory controller");
    value = HW_DRAM_CTL08_RD();
    value |= BM_DRAM_CTL08_START;
    HW_DRAM_CTL08_WR(value);

    exit_selfrefresh();

    set_port_priority();

    entry_auto_clock_gate();

    change_cpu_freq();

    hw_mem_test();

    return 0;
}

