/*
 * iMX233 HW pin mux init
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
#include "pinmux.h"

void init_pinmux(void)
{
    print_pin("%s", "Configure SSP1 pins for ENC28j60: 8maA");
        
    /*  */
    REG_CLR(PINCTRL_BASE + PINCTRL_MUXSEL(4), 0x00003fff);

    REG_CLR(PINCTRL_BASE + PINCTRL_DRIVE(8), 0X03333333);
    REG_SET(PINCTRL_BASE + PINCTRL_DRIVE(8), 0x01111111);

    REG_CLR(PINCTRL_BASE + PINCTRL_PULL(2), 0x0000003f);
    sleep_ms(10);
    
    return;
}

