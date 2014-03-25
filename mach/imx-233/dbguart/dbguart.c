/**
 * HW Debug UART Driver file
 *
 * (c) 2007 Sascha Hauer <s.hauer@pengutronix.de>
 * (c) Copyright 2009 Freescale Semiconductor, Inc.
 * Copyright (c) 2014 Alex Winter <eterno.despierto@gmail.com>
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

#include "registers/regsuartdbg.h"
#include "dbguart.h"
#include "platform.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
#define     PINCTRL_BASE            (REGS_BASE + 0x18000)
#define     PINCTRL_MUXSEL(n)       (0x100 + 0x10*(n))
#define     DBGUART_BASE            (REGS_BASE + 0x00070000)


static void drv_serial_setbrg(void);


/************************************************
 *              GLOBAL FUNCTIONS                                      *
 ************************************************/
int drv_serial_init(void)
{
    unsigned int cr;
    unsigned long reg = REG_RD(PINCTRL_BASE + PINCTRL_MUXSEL(3));

    /* Disable UART */
    REG_WR(DBGUART_BASE + UARTDBGCR, 0);

    /* Mask interrupts */
    REG_WR(DBGUART_BASE + UARTDBGIMSC, 0);

    /* Set default baudrate */
    drv_serial_setbrg();

    /* Enable UART */
    cr = TXE | RXE | UARTEN;
    REG_WR(DBGUART_BASE + UARTDBGCR, cr);

    /*setup mux for rx pin */
    reg &= ~(0x3UL << 20);
    reg |= (0x2UL << 20);
    REG_WR(PINCTRL_BASE + PINCTRL_MUXSEL(3), reg);

    return 0;
}

void drv_serial_putc(const char ch)
{
    int loop = 0;
    //while (REG_RD(DBGUART_BASE + UARTDBGFR) & TXFF)
    while (HW_UARTDBGFR_RD()&BM_UARTDBGFR_TXFF) {
#if 0        
        loop++;
        if (loop > 10000)
            break;
#else
        ;
#endif
    };

    /* if(!(HW_UARTDBGFR_RD() &BM_UARTDBGFR_TXFF)) */
    HW_UARTDBGDR_WR(ch);
    //REG_WR(DBGUART_BASE + UARTDBGDR, c);
    
    if (ch == '\n')
        drv_serial_putc('\r');
    
    return;
}

void drv_serial_puts(const char *s)
{
    while (*s) {
        drv_serial_putc(*s++);
    }

    return;
}


/* Test whether a character is in TX buffer */
int drv_serial_tstc(void)
{
    /* Check if RX FIFO is not empty */
    return !(REG_RD(DBGUART_BASE + UARTDBGFR) & RXFE);
}


/* Receive character */
int drv_serial_getc(void)
{
    /* Wait while TX FIFO is empty */
    while (REG_RD(DBGUART_BASE + UARTDBGFR) & RXFE)
        ;

    /* Read data byte */
    return REG_RD(DBGUART_BASE + UARTDBGDR) & 0xff;
}



/************************************************
 *              LOCAL FUNCTIONS                                      *
 ************************************************/

/* Set baud rate. The settings are always 8n1: 8 data bits, no parity, 1 stop bit */
static void drv_serial_setbrg(void)
{
    unsigned int cr, lcr_h;
    unsigned int quot;

    /* Disable everything */
    cr = REG_RD(DBGUART_BASE + UARTDBGCR);
    REG_WR(DBGUART_BASE + UARTDBGCR, 0);

    /* Calculate and set baudrate */
    quot = (CONFIG_DBGUART_CLK * 4) / CONFIG_BAUDRATE;
    REG_WR(DBGUART_BASE + UARTDBGFBRD, quot & 0x3f);
    REG_WR(DBGUART_BASE + UARTDBGIBRD, quot >> 6);

    /* Set 8n1 mode, enable FIFOs */
    lcr_h = WLEN8 | FEN;
    REG_WR(DBGUART_BASE + UARTDBGLCR_H, lcr_h);

    /* Enable Debug UART */
    REG_WR(DBGUART_BASE + UARTDBGCR, cr);

    return;
}


