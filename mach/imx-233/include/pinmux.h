/*
 * Copyright (C) 2008 Embedded Alley Solutions Inc.
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 *
 * Clock control register descriptions
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
#ifndef __PINMUX_H__
#define __PINMUX_H__

#include "platform.h"

#define	PINCTRL_BASE        (REGS_BASE + 0x18000)

#define PINCTRL_CTRL        0x000
#define PINCTRL_MUXSEL(n)   (0x100 + 0x10*(n))
#define PINCTRL_DRIVE(n)    (0x200 + 0x10*(n))
#define PINCTRL_PULL(n)     (0x400 + 0x10*(n))
#define PINCTRL_DOUT(n)     (0x500 + 0x10*(n))
#define PINCTRL_DIN(n)      (0x600 + 0x10*(n))
#define PINCTRL_DOE(n)      (0x700 + 0x10*(n))
#define PINCTRL_PIN2IRQ(n)  (0x800 + 0x10*(n))
#define PINCTRL_IRQEN(n)    (0x900 + 0x10*(n))
#define PINCTRL_IRQLEVEL(n) (0xa00 + 0x10*(n))
#define PINCTRL_IRQPOL(n)   (0xb00 + 0x10*(n))
#define PINCTRL_IRQSTAT(n)  (0xc00 + 0x10*(n))

void init_pinmux(void);

#endif /* __PINMUX_H__ */

