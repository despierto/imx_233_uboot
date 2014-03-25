/**
 * HW Print Driver header file
 *
 * Copyright (c) 2014 Alex Winter <eterno.despierto@gmail.com>
 * (c) Copyright 2009 Freescale Semiconductor, Inc.
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

#ifndef _DBGUART_H_
#define _DBGUART_H_

/************************************************
 *              DEFINITIONS                                                *
 ************************************************/

#define UARTDBGDR           0x00
#define UARTDBGRSR_ECR      0x04
#define UARTDBGFR           0x18
#define UARTDBGILPR         0x20
#define UARTDBGIBRD         0x24
#define UARTDBGFBRD         0x28
#define UARTDBGLCR_H        0x2c
#define UARTDBGCR           0x30
#define UARTDBGIFLS         0x34
#define UARTDBGIMSC         0x38
#define UARTDBGRIS          0x3c
#define UARTDBGMIS          0x40
#define UARTDBGICR          0x44
#define UARTDBGDMACR        0x48

/* UARTDBGFR - Flag Register bits */
#define CTS                 0x0001
#define DSR                 0x0002
#define DCD                 0x0004
#define BUSY                0x0008
#define RXFE                0x0010
#define TXFF                0x0020
#define RXFF                0x0040
#define TXFE                0x0080
#define RI                  0x0100

/* UARTDBGLCR_H - Line Control Register bits */
#define BRK                 0x0001
#define PEN                 0x0002
#define EPS                 0x0004
#define STP2                0x0008
#define FEN                 0x0010
#define WLEN5               0x0000
#define WLEN6               0x0020
#define WLEN7               0x0040
#define WLEN8               0x0060
#define SPS                 0x0080

/* UARTDBGCR - Control Register bits */
#define UARTEN              0x0001
#define LBE                 0x0080
#define TXE                 0x0100
#define RXE                 0x0200
#define DTR                 0x0400
#define RTS                 0x0800
#define OUT1                0x1000
#define OUT2                0x2000
#define RTSEN               0x4000
#define CTSEN               0x8000


/************************************************
 *              FUNSTIONS                                                  *
 ************************************************/
int     drv_serial_init(void);
void    drv_serial_putc(const char ch);
void    drv_serial_puts(const char *s);
int     drv_serial_getc(void);
int     drv_serial_tstc(void);

#endif /*_DBGUART_H_*/
