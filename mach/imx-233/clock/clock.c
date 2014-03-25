/*
 * iMX233 HW clock init
 *
 * Copyright (c) 2013 X-boot GITHUB team
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "global.h"
#include "clkctrl.h"
#include "registers/regsclkctrl.h"
#include "registers/regsrtc.h"


#define KHz             (1000)
#define MHz             (1000 * KHz)
#define IO_DIVIDER      (18)

static int rtc_init(void);

int init_clocks(void)
{
    U32 ssp_source_clk, ssp_clk;
    U32 ssp_div = 1;
    U32 val = 0;
    U32 devider;

    //print_clk("%s", "Initialize clocks");

    /* Configure CPU IO clock  */

    /* Update IO_CLK and set divider back */
    REG_CLR(CLKCTRL_BASE + CLKCTRL_FRAC, FRAC_CLKGATEIO);
    REG_CLR(CLKCTRL_BASE + CLKCTRL_FRAC, 0x3f << FRAC_IOFRAC);
    REG_SET(CLKCTRL_BASE + CLKCTRL_FRAC, CPU_CLK_DEVIDER << FRAC_IOFRAC);

    val = HW_CLKCTRL_FRAC_RD();
    devider = (val & BM_CLKCTRL_FRAC_CPUFRAC);
    if (CPU_CLK_DEVIDER != devider) {
        //print_err("CPU frequency wasn't adjusted, read devider is (%d)", devider);
        return  FAILURE;
    } else {
       // print_clk(" - check CPU frequency: %d Hz", 480*(18*1000000/devider)); 
    }

    /* Set SSP CLK to desired value  */

    /* Calculate SSP_CLK divider relatively to 480Mhz IO_CLK*/
    ssp_source_clk = 480 * MHz;
    ssp_clk = CONFIG_SSP_CLK;
    ssp_div = (ssp_source_clk + ssp_clk - 1) / ssp_clk;

    /* Enable SSP clock */
    val = REG_RD(CLKCTRL_BASE + CLKCTRL_SSP);
    val &= ~SSP_CLKGATE;
    REG_WR(CLKCTRL_BASE + CLKCTRL_SSP, val);

    /* Wait while clock is gated */
    while (REG_RD(CLKCTRL_BASE + CLKCTRL_SSP) & SSP_CLKGATE)
        ;

    /* Set SSP clock divider */
    val &= ~(0x1ff << SSP_DIV);
    val |= ssp_div << SSP_DIV;
    REG_WR(CLKCTRL_BASE + CLKCTRL_SSP, val);

    /* Wait until new divider value is set */
    while (REG_RD(CLKCTRL_BASE + CLKCTRL_SSP) & SSP_BUSY)
        ;

    /* Set SSP clock source to IO_CLK */
    REG_SET(CLKCTRL_BASE + CLKCTRL_CLKSEQ, CLKSEQ_BYPASS_SSP);
    REG_CLR(CLKCTRL_BASE + CLKCTRL_CLKSEQ, CLKSEQ_BYPASS_SSP);

    //print_clk(" - SSP clock was initialized with frequenct %d Hz", ssp_source_clk);     
    sleep_ms(10);

    //reset rtc clock
    HW_RTC_MILLISECONDS_WR(0);
    HW_RTC_SECONDS_WR(0);    
   
    return SUCCESS;
}


