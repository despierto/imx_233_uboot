/*
 * Copyright (C) 2008 Embedded Alley Solutions Inc.
 *
 * (C) Copyright 2009 Freescale Semiconductor, Inc.
 *
 * Freescale STMP378x SSP/SPI driver
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
#include "drv_spi.h"

/************************************************
 *              DEFINITIONS                                                *
 ************************************************/

#define SPI_NUM_BUSES	2
#define SPI_NUM_SLAVES	3

/* Initalized in spi_init() depending on SSP port configuration */
static unsigned long ssp_bases[SPI_NUM_BUSES];

/* Set in spi_set_cfg() depending on which SSP port is being used */
static unsigned long ssp_base = SSP1_BASE;

static void ssp_spi_init(unsigned int bus);
static unsigned char spi_read(void);
static void spi_write(unsigned char b);
static void spi_lock_cs(void);
static void spi_unlock_cs(void);

/************************************************
  *              GLOBAL FUNCTIONS                                      *
  ************************************************/

/* Init SSP ports, must be called first and only once */
void spi_init(void)
{
    ssp_bases[0] = SSP1_BASE;
    ssp_spi_init(0);
    sleep_ms(100);
    
    return;
}

void spi_set_cfg(unsigned int bus, unsigned int cs, unsigned long mode)
{
    U32 clr_mask = 0;
    U32 set_mask = 0;

    print_spi("SPI set configuration: bus (%d) cs (%d) mode (%d)", bus, cs, (unsigned int)mode);

    if (bus >= SPI_NUM_BUSES || cs >= SPI_NUM_SLAVES) {
        print_err("SPI device %d:%d doesn't exist", bus, cs);
        return;
    }

    if (ssp_bases[bus] == 0) {
        print_err("SSP port %d isn't in SPI mode", bus + 1);
        return;
    }

    /* Set SSP port to use */
    ssp_base = ssp_bases[bus];

    /* Set phase and polarity: HW_SSP_CTRL1 */
    if (mode & SPI_PHASE)
        set_mask |= CTRL1_PHASE;
    else
        clr_mask |= CTRL1_PHASE;

    if (mode & SPI_POLARITY)
        set_mask |= CTRL1_POLARITY;
    else
        clr_mask |= CTRL1_POLARITY;

    REG_SET(ssp_base + SSP_CTRL1, set_mask);
    REG_CLR(ssp_base + SSP_CTRL1, clr_mask);

    /* Set SSn number: HW_SSP_CTRL0 */
    REG_CLR(ssp_base + SSP_CTRL0, SPI_CS_CLR_MASK);

    switch (cs) {
    case 0:
        set_mask = SPI_CS0;
        break;
    case 1:
        set_mask = SPI_CS1;
        break;
    case 2:
        set_mask = SPI_CS2;
        break;
    }

    REG_SET(ssp_base + SSP_CTRL0, set_mask);

    return;
}

void spi_txrx(const char *dout, unsigned int tx_len, char *din, unsigned int rx_len, unsigned long flags)
{
    int i;

    //print_spi("---> spi_txrx: tx_len (%d) rx_len (%d) dout (%x)", tx_len, rx_len, (unsigned int)dout);
    
    if (tx_len == 0 && rx_len == 0)
        return;

    if (flags & SPI_START)
        spi_lock_cs();

    for (i = 0; i < tx_len; i++) {

        /* Check if it is last data byte to transfer */
        if (flags & SPI_STOP && rx_len == 0 && i == tx_len - 1)
            spi_unlock_cs();

        spi_write(dout[i]);
    }

    for (i = 0; i < rx_len; i++) {

        /* Check if it is last data byte to transfer */
        if (flags & SPI_STOP && i == rx_len - 1)
            spi_unlock_cs();

        din[i] = spi_read();
    }

    return;
}



/************************************************
 *              LOCAL FUNCTIONS                                        *
 ************************************************/

/* Init SSP port: SSP1 (@bus = 0) or SSP2 (@bus == 1) */
static void ssp_spi_init(unsigned int bus)
{
    U32 spi_div;
    U32 val = 0;

    print_spi("Configure SPI bus %d", bus);

    if (bus >= SPI_NUM_BUSES) {
        print_err("SPI bus %d doesn't exist", bus);
        return;
    }

    ssp_base = ssp_bases[bus];
    print_spi(" - SSP base 0x%x", ssp_base);

    
    /* Reset block */
    print_spi("%s", " - SSP block reset");

    /* Clear SFTRST */
    REG_CLR(ssp_base + SSP_CTRL0, CTRL0_SFTRST);
    while (REG_RD(ssp_base + SSP_CTRL0) & CTRL0_SFTRST)
        ;

    /* Clear CLKGATE */
    REG_CLR(ssp_base + SSP_CTRL0, CTRL0_CLKGATE);

    /* Set SFTRST and wait until CLKGATE is set */
    REG_SET(ssp_base + SSP_CTRL0, CTRL0_SFTRST);
    while (!(REG_RD(ssp_base + SSP_CTRL0) & CTRL0_CLKGATE))
        ;

    /* Clear SFTRST and CLKGATE */
    REG_CLR(ssp_base + SSP_CTRL0, CTRL0_SFTRST);
    REG_CLR(ssp_base + SSP_CTRL0, CTRL0_CLKGATE);

    /* Set CLK to desired value  */
    
    spi_div = ((CONFIG_SSP_CLK>>1) + CONFIG_SPI_CLK - 1) / CONFIG_SPI_CLK;
    val = (2 << TIMING_CLOCK_DIVIDE) | ((spi_div - 1) << TIMING_CLOCK_RATE);
    REG_WR(ssp_base + SSP_TIMING, val);
    print_spi(" - Set SSP bus speed %d Hz", CONFIG_SSP_CLK/spi_div);

    /* Set transfer parameters */

    /* Set SSP SPI Master mode and word length to 8 bit */
    REG_WR(ssp_base + SSP_CTRL1, WORD_LENGTH8 | SSP_MODE_SPI);

    /* Set BUS_WIDTH to 1 bit and XFER_COUNT to 1 byte */
    REG_WR(ssp_base + SSP_CTRL0,
           BUS_WIDTH_SPI1 | (0x1 << CTRL0_XFER_COUNT));

    /*
    * Set BLOCK_SIZE and BLOCK_COUNT to 0, so that XFER_COUNT
    * reflects number of bytes to send. Disalbe other bits as
    * well
    */
    REG_WR(ssp_base + SSP_CMD0, 0x0);

    return;
}


/* Read single data byte */
static unsigned char spi_read(void)
{
    unsigned char b = 0;

    /* Set XFER_LENGTH to 1 */
    REG_CLR(ssp_base + SSP_CTRL0, 0xffff);
    REG_SET(ssp_base + SSP_CTRL0, 1);

    /* Enable READ mode */
    REG_SET(ssp_base + SSP_CTRL0, CTRL0_READ);

    /* Set RUN bit */
    REG_SET(ssp_base + SSP_CTRL0, CTRL0_RUN);


    /* Set transfer */
    REG_SET(ssp_base + SSP_CTRL0, CTRL0_DATA_XFER);

    while (REG_RD(ssp_base + SSP_STATUS) & STATUS_FIFO_EMPTY)
        ;

    /* Read data byte */
    b = REG_RD(ssp_base + SSP_DATA) & 0xff;

    /* Wait until RUN bit is cleared */
    while (REG_RD(ssp_base + SSP_CTRL0) & CTRL0_RUN)
        ;

    return b;
}

/* Write single data byte */
static void spi_write(unsigned char b)
{
    /* Set XFER_LENGTH to 1 */
    REG_CLR(ssp_base + SSP_CTRL0, 0xffff);
    REG_SET(ssp_base + SSP_CTRL0, 1);

    /* Enable WRITE mode */
    REG_CLR(ssp_base + SSP_CTRL0, CTRL0_READ);

    /* Set RUN bit */
    REG_SET(ssp_base + SSP_CTRL0, CTRL0_RUN);

    /* Write data byte */
    REG_WR(ssp_base + SSP_DATA, b);

    /* Set transfer */
    REG_SET(ssp_base + SSP_CTRL0, CTRL0_DATA_XFER);

    /* Wait until RUN bit is cleared */
    while (REG_RD(ssp_base + SSP_CTRL0) & CTRL0_RUN)
        ;

    return;    
}

static void spi_lock_cs(void)
{
    REG_CLR(ssp_base + SSP_CTRL0, CTRL0_IGNORE_CRC);
    REG_SET(ssp_base + SSP_CTRL0, CTRL0_LOCK_CS);

    return;    
}

static void spi_unlock_cs(void)
{
    REG_CLR(ssp_base + SSP_CTRL0, CTRL0_LOCK_CS);
    REG_SET(ssp_base + SSP_CTRL0, CTRL0_IGNORE_CRC);

    return;
}

