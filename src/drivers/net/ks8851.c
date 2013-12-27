/*
 * 2010 Alexander Kudjashev
 *
 * Adapted Simtec Electronics linux driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <config.h>
#include <common.h>
#include <malloc.h>
#include <net.h>
#include <asm/arch/spi.h>
#include <linux/types.h>
#include "ks8851.h"

extern void NetReceive(volatile uchar *, int);

#define ETH_ALEN        6
#define KS_ERR          -1
#define MAX_BUF_SIZE    2048

#define ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1)
#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))

/* shift for byte-enable data */
#define BYTE_EN(_x) ((_x) << 2)

/* turn register number and byte-enable mask into data for start of packet */
#define MK_OP(_byteen, _reg) (BYTE_EN(_byteen) | (_reg)  << (8+2) | (_reg) >> 6)


union ks8851_tx_hdr {
    uchar   txb[6];
    ushort  txw[3];
};

typedef struct {
    ushort  fid;
    uchar   buff[MAX_BUF_SIZE];
    union ks8851_tx_hdr txh;
} ks8851_inf;

static ks8851_inf  *ks;
//uchar def_mac_addr[] = {0x00, 0x10, 0xA1, 0x86, 0x95, 0x11};
uchar def_mac_addr[] = {0x00, 0x1F, 0xF2, 0x00, 0x00, 0x00};


/*
 * write a 8 bit register to ks8851 chip
 */
static void ks_reg8_write(ushort reg, ushort val)
{
    ushort  txb[2];
    ushort  bit;

    bit = 1 << (reg & 3);

    txb[0] = MK_OP(bit, reg) | KS_SPIOP_WR;
    txb[1] = val;

    spi_txrx(txb, 3, 0, 0, SPI_START | SPI_STOP);
}

/*
 * write a 16 bit register to ks8851 chip
*/
static void ks_reg16_write(ushort reg, ushort val)
{
    ushort txb[2];

    txb[0] = MK_OP(reg & 2 ? 0xC : 0x03, reg) | KS_SPIOP_WR;
    txb[1] = val;

    spi_txrx(txb, 4, 0, 0, SPI_START | SPI_STOP);
}

/*
 * issue read register command and return the data
 */
static void ks_reg_read(ushort op, uchar *rxb, ushort len)
{
    ushort txb = op | KS_SPIOP_RD;

    spi_txrx(&txb, 2, 0, 0, SPI_START);
    spi_txrx(0, 0, rxb, len, SPI_STOP);
}

/*
 * read 8 bit register from device
 */
static ushort ks_reg8_read(ushort reg)
{
    ushort rxb = 0;

    ks_reg_read(MK_OP(1 << (reg & 3), reg), &rxb, 1);

    return rxb;
}

/*
 * read 16 bit register from device
 */
static ushort ks_reg16_read(ushort reg)
{
    ushort rxb = 0;

    ks_reg_read(MK_OP(reg & 2 ? 0xC : 0x3, reg), &rxb, 2);

    return rxb;
}

/*
 * read 32 bit register from device
 */
static uint ks_reg32_read(ushort reg)
{
    uint rxb = 0;

    ks_reg_read(MK_OP(0x0f, reg), &rxb, 4);

    return rxb;
}

/*
 * set KS8851 MAC address
*/
void ks_mac_set(void)
{
    int i;
    char ethaddr[64];
    char *env_ethaddr = getenv ("ethaddr");

    for(i = 0; i < ETH_ALEN; i++)
        ks_reg8_write(KS_MAR(i), def_mac_addr[i]);

    sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
        def_mac_addr[0], def_mac_addr[1],
        def_mac_addr[2], def_mac_addr[3],
        def_mac_addr[4], def_mac_addr[5]);

    if(strcmp(ethaddr, env_ethaddr)) {
        printf("set environment from HW MAC addr = \"%s\"\n", ethaddr);
        setenv("ethaddr", ethaddr);
    }
}

/*
 * read data from the receive fifo
 */
static void ks_fifo_read(uchar *buff, ushort len)
{
    uchar txb = KS_SPIOP_RXFIFO;

    spi_txrx(&txb, 1, 0, 0, SPI_START);
    spi_txrx(0, 0, buff, len, SPI_STOP);
}

/*
 * receive packets from the host
 */
static void ks_rx_pkts(void)
{
    uint    rxh;
    ushort  rxfc, rxlen, status;
    int i;

    status = ks_reg16_read(KS_ISR);
    if(status & IRQ_RXI == 0)
        return;

    ks_reg16_write(KS_ISR, IRQ_RXI);
    rxfc = ks_reg8_read(KS_RXFC);

    for (; rxfc != 0; rxfc--) {
        rxh = ks_reg32_read(KS_RXFHSR);
        /* the length of the packet includes the 32bit CRC */
        rxlen = rxh >> 16;

        /* setup Receive Frame Data Pointer Auto-Increment */
        ks_reg16_write(KS_RXFDPR, RXFDPR_RXFPAI);

        /* start the packet dma process, and set auto-dequeue rx */
        ks_reg16_write(KS_RXQCR, RXQCR_SDA | RXQCR_ADRFE);

        if(rxlen > 0) {
            /* align the packet length to 4 bytes, and add 4 bytes
               as we're getting the rx status header as well */
            ks_fifo_read(ks->buff, ALIGN(rxlen, 4) + 8);

            NetReceive(ks->buff + 8, rxlen);
        }
        ks_reg16_write(KS_RXQCR, 0);
    }
}

/*
 * write packet to TX FIFO
 */
static void ks_tx_pkt(uchar *txp, int len)
{
    ushort fid = 0;

    fid = ks->fid++;
    fid &= TXFR_TXFID_MASK;

    /* start header at txb[1] to align txw entries */
    ks->txh.txb[1] = KS_SPIOP_TXFIFO;
    ks->txh.txw[1] = fid;
    ks->txh.txw[2] = len;

    ks_reg16_write(KS_RXQCR, RXQCR_SDA);

    spi_txrx(&ks->txh.txb[1], 5, 0, 0, SPI_START);
    spi_txrx(txp, ALIGN(len, 4), 0, 0, SPI_STOP);

    ks_reg16_write(KS_RXQCR, 0);
}

/*
 * set power mode of the device
 */
static void ks_powermode_set(ushort pwrmode)
{
    ushort pmecr;

    pmecr = ks_reg16_read(KS_PMECR);
    pmecr &= ~PMECR_PM_MASK;
    pmecr |= pwrmode;

    ks_reg16_write(KS_PMECR, pmecr);
}

/*
 * configure network device
 */
static void ks_config(void)
{
    /* bring chip out of any power saving mode it was in */
    ks_powermode_set(PMECR_PM_NORMAL);

    /* auto-increment tx data, reset tx pointer */
    ks_reg16_write(KS_TXFDPR, TXFDPR_TXFPAI);

    /* Enable QMU TxQ Auto-Enqueue frame */
    ks_reg16_write(KS_TXQCR, TXQCR_AETFE);

    /* setup transmission parameters */
    ks_reg16_write(KS_TXCR, (TXCR_TXE |     /* enable transmit process */
                            TXCR_TCGIP |
                            TXCR_TCGTCP |
                            TXCR_TCGICMP |
                            TXCR_TXPE |     /* pad to min length */
                            TXCR_TXCRC |    /* add CRC */
                            TXCR_TXFCE));   /* enable flow control */

    /* Setup Receive Frame Threshold - 1 frame */
    ks_reg16_write(KS_RXFCTR, 1 << RXFCTR_RXFCT_SHIFT);

    /* setup receiver control */
    ks_reg16_write(KS_RXCR1, (RXCR1_RXPAFMA |   /* mac filter */
                            RXCR1_RXUDPFCC |
                            RXCR1_RXTCPFCC |
                            RXCR1_RXIPFCC |
                            RXCR1_RXME |
                            RXCR1_RXAE |
                            RXCR1_RXFCE |       /* enable flow control */
                            RXCR1_RXUE |        /* unicast enable */
                            RXCR1_RXE));        /* enable rx block */
                          /*  RXCR1_RXBE |         broadcast enable */

    /* transfer entire frames out in one go */
    ks_reg16_write(KS_RXCR2, RXCR2_SRDBL_FRAME);
}

/*
 * close network device
 */
static void ks_disable(void)
{
    /* shutdown RX process */
    ks_reg16_write(KS_RXCR1, 0x0000);

    /* shutdown TX process */
    ks_reg16_write(KS_TXCR, 0x0000);

    /* set powermode to soft power down to save power */
    ks_powermode_set(PMECR_PM_SOFTDOWN);
}

/*
 * KS8851 chip initialization
*/
static int ks_init(void)
{
    ushort chip_id;

    /* issue a global soft reset to reset the device. */
    ks_reg16_write(KS_GRR, GRR_GSR);
    udelay(500);  /* wait a short time to effect reset */
    ks_reg16_write(KS_GRR, 0);
    udelay(500);  /* wait for condition to clear */

    /* simple check for a valid chip being connected to the bus */
    chip_id = ks_reg16_read(KS_CIDER);
    printf("ks8851 chip ID=0x%x\n", chip_id);
    if((chip_id & ~CIDER_REV_MASK) != CIDER_ID) {
        printf("Error: the ks8851 chip ID is wrong\n");
        return KS_ERR;
    }

    ks = malloc(sizeof(ks8851_inf));
    if(!ks) {
        printf("malloc: cannot allocate space\n");
        return KS_ERR;
    }

    ks->fid = 0;

    ks_mac_set();
    ks_config();

    ks_reg16_write(KS_ISR, 0xffff);
    ks_reg16_write(KS_IER, IRQ_RXI);

    return 0;
}

/******************** UBOOT Ethernet interface **********************/

int eth_init(bd_t *bd)
{
    int ret;

    ret = ks_init();
    return ret;
}

void eth_halt()
{

    ks_disable();
}

int eth_rx()
{

    ks_rx_pkts();
    return 0;
}

int eth_send(volatile void *packet, int length)
{

    ks_tx_pkt((uchar *)packet, length);
    return 0;
}

