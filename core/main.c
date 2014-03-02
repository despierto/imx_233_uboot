/**
 * X-Boot Operation System Entry
 *
 * Copyright (c) 2014 X-boot GITHUB team
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

/*************************************************
 *                   X-BOOT entry                *
 *************************************************/
#include "global.h"
#include "drv_eth.h"
#include "clkctrl.h"
#include "pinmux.h"
#include "spi.h"

#define XBOOT_VERSION_R      0
#define XBOOT_VERSION_RC     2

static unsigned int system_time_msec = 0;
static void initialization(void);
static void termination(void);
static void rt_process(void);

void  _start(void)
{
    unsigned int i, a;

    print_inf("\r\n");
    print_inf("--- IMX-233: X-BOOT initialization ---\r\n");
    print_inf("%s %s\r\n", __DATE__, __TIME__);
    print_inf("Version: %d.%d\r\n\r\n", XBOOT_VERSION_R, XBOOT_VERSION_RC);

    initialization();

    //viewk system configuration
    drv_eth_info();
    
    //do a test ping
    net_ping_req(10000UL, pEth->cfg_ip_server);

    print_log("%s", "Entry to the main loop...");
    while(1)
    {
        if (system_time_msec%600000 == 0)
          print_inf("[%d sec] Next cycle...\r\n", system_time_msec/1000);

        rt_process();
        
        sleep_ms(50);
        system_time_msec+=50;
    }

    termination();
        
    return;
}

static void initialization(void)
{
    print_log("%s", "Environment initialization");

    /* Configure CPU and SSP clocks*/
    init_clocks();
   
    /* Configure SSP1 pins*/
    init_pinmux();

    /* Configure SPI on SSP1*/
    spi_init();

    /* Configure Ethernt device*/
    drv_eth_init();

    return;
}

static void rt_process(void)
{
    //runtime
    

    return;
}
static void termination(void)
{
    print_log("%s", "Environment termination");

    drv_eth_halt();

    return;
}

