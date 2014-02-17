/**
 * Driver for Ethernet PHY KS8851
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

/***********************************************************************************************************************************
Features
    - Integrated MAC and PHY Ethernet Controller fully  compliant with IEEE 802.3/802.3£g standards
    - Designed for high performance and high throughput  applications
    - Supports 10BASE-T/100BASE-TX 
    - Supports IEEE 802.3x full-duplex flow control and halfduplex  backpressure collision flow control
    - Supports DMA-slave burst data read and write  transfers
    - Supports IP Header (IPv4)/TCP/UDP/ICMP checksum  generation and checking
    - Supports IPv6 TCP/UDP/ICMP checksum generation  and checking
    - Automatic 32-bit CRC generation and checking
    - Simple SRAM-like host interface easily connects to  most common embedded MCUs.
    - Supports multiple data frames for transmit and receive without address bus and byte-enable signals
    - Supports both Big- and Little-Endian processors
    - Larger internal memory with 12K Bytes for RX FIFO and 6K Bytes for TX FIFO. Programmable low, high  and overrun watermark for flow control in RX FIFO
    - Shared data bus for Data, Address and Byte Enable 
    - Efficient architecture design with configurable host  interrupt schemes to minimize host CPU overhead and  utilization
    - Powerful and flexible address filtering scheme
    - Optional to use external serial EEPROM configuration for MAC address
    - Single 25MHz reference clock for both PHY and MAC
    - HBM ESD Rating 6kV
Power Modes, Power Supplies, and Packaging
    - Single 3.3V power supply with options for 1.8V, 2.5V and 3.3V VDD I/O
    - Built-in integrated 3.3V or 2.5V to 1.8V low noise regulator (LDO) for core and analog blocks
    - Enhanced power management feature with energy  detect mode and soft power-down mode to ensure low-power dissipation during device idle periods
            Comprehensive LED indicator support for link, activity  and 10/100 speed (2 LEDs) - User programmable
    - Low-power CMOS design
    - Commercial Temperature Range: 0C to +70C
    - Industrial Temperature Range: -40C to +85C
    - Flexible package options available in 48-pin  (7mm x 7mm) LQFP KSZ8851-16MLL or 128-pin  PQFP KSZ8851-16/32MQL
Additional Features. In addition to offering all of the features of a Layer 2 controller, the KSZ8851-16MLL offers:
    - Flexible 8-bit and 16-bit generic host processor  interfaces with same access time and single bus  timing to any I/O registers and RX/TX FIFO buffers
    - Supports to add two-byte before frame header in order for IP frame content with double word boundary 
    - Micrel LinkMDR cable diagnostic capabilities to determine cable length, diagnose faulty cables, and determine distance to fault
    - Wake-on-LAN functionality: Incorporates Magic Packet., wake-up frame, network link state, and detection of energy signal technology
    - HP Auto MDI-X. crossover with disable/enable option
    - Ability to transmit and receive frames up to 2000 bytes
 Network Features
    - 10BASE-T and 100BASE-TX physical layer support
    - Auto-negotiation: 10/100 Mbps full and half duplex
    - Adaptive equalizer
    - Baseline wander correction
 ************************************************************************************************************************************/

#include "global.h"
#include "net_ks8851.h"
#include "drv/regs_ks8851.h"
#include "spi.h"

/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
#define KS_ERR          -1
#define MAX_BUF_SIZE    2048
#define ETH_ALEN        6

typedef struct _KS8851_TX_HDR_ {
    U8      txb[6];
    U16     txw[3];
}KS8851_TX_HDR, *PKS8851_TX_HDR;

typedef struct _NET_KS8851_INF_ {
    U16             fid;
    U8              buff[MAX_BUF_SIZE];
    KS8851_TX_HDR   txh;
}NET_KS8851_INF, *PNET_KS8851_INF;


#define __ALIGN_MASK(x,mask)    (((x)+(mask))&~(mask))
#define ALIGN(x,a)              __ALIGN_MASK(x,(__typeof__(x))(a)-1)

/* shift for byte-enable data */
#define BYTE_EN(_x) ((_x) << 2)

/* turn register number and byte-enable mask into data for start of packet */
#define MK_OP(_byteen, _reg) (BYTE_EN(_byteen) | (_reg)  << (8+2) | (_reg) >> 6)

//uchar def_mac_addr[] = {0x00, 0x10, 0xA1, 0x86, 0x95, 0x11};
uchar def_mac_addr[] = {0x00, 0x1F, 0xF2, 0x00, 0x00, 0x00};

static PNET_KS8851_INF ks = (PNET_KS8851_INF)SYS_RAM_NET_CTX_ADDR;

static void     ks_reg8_write(ushort reg, ushort val);
static void     ks_reg16_write(ushort reg, ushort val);
static void     ks_reg_read(ushort op, uchar *rxb, ushort len);
static ushort   ks_reg8_read(ushort reg);
static ushort   ks_reg16_read(ushort reg);
static uint     ks_reg32_read(ushort reg);
static void     ks_powermode_set(ushort pwrmode);
static void     ks_fifo_read(uchar *buff, ushort len);
void            ks_mac_set(void);
void            ks_mac_default_set(void);
static void     ks_config(void);

/************************************************
  *              GLOBAL FUNCTIONS                                      *
  ************************************************/



RESULTCODE  net_ks8851_init(PTR ptr)
{
    RESULTCODE  ret = 0;
    U16         chip_id;

    print_net("%s", "Network device initialization started");

    print_net("%s", " - Reset device");
    ks_reg16_write(KS_GRR, GRR_GSR);                                /* issue a global soft reset to reset the device. */
    sleep_us(500);                                                  /* wait a short time to effect reset */
    ks_reg16_write(KS_GRR, 0);
    sleep_us(500);                                                  /* wait for condition to clear */
    
    chip_id = ks_reg16_read(KS_CIDER);                              /* simple check for a valid chip being connected to the bus */
    if((chip_id & ~CIDER_REV_MASK) != CIDER_ID) {
        print_err("The ks8851 chip ID is wrong, ID=0x%x", chip_id);
        return KS_ERR;
    }
    print_net(" - ks8851 chip ID=0x%x", chip_id);

    if (sizeof(NET_KS8851_INF) > SYS_RAM_NET_CTX_SIZE) {
        print_err("Size of NET_KS8851_INF CTX (%d) is out of ranges in (%d) bytes", sizeof(NET_KS8851_INF), SYS_RAM_NET_CTX_SIZE);
        return KS_ERR;
    }
    print_net(" - Allocation of NET_KS8851_INF CTX at (0x%x), size (%d)", (unsigned int)ks, sizeof(NET_KS8851_INF));
        
    ks->fid = 0;

    ks_mac_default_set();
    //net_ks8851_mac_set("AA:BB:CC:DD:EE:FF");

    ks_config();

    
    ks_reg16_write(KS_ISR, 0xffff);
    ks_reg16_write(KS_IER, IRQ_RXI);
    print_net("%s", " - Enabled device ISR"); 
           
    return ret;
}

void        net_ks8851_halt(void)
{
    print_net("%s", "KS-8851 network device halt");

    /* shutdown RX process */
    print_net("%s", " - Shutdown RX process");
    ks_reg16_write(KS_RXCR1, 0x0000);

    /* shutdown TX process */
    print_net("%s", " - Shutdown RX process");
    ks_reg16_write(KS_TXCR, 0x0000);

    /* set powermode to soft power down to save power */
    print_net("%s", " - Disable power");    
    ks_powermode_set(PMECR_PM_SOFTDOWN);

    return;
}

RESULTCODE  net_ks8851_rx(void)
{
    print_net("--> %s -> %s : %d", __FILE__, __FUNCTION__, __LINE__);
    
    return 0;
}

RESULTCODE  net_ks8851_tx(VPTR packet, U32 length)
{
    ushort fid = 0;

    fid = ks->fid++;
    fid &= TXFR_TXFID_MASK;

    /* start header at txb[1] to align txw entries */
    ks->txh.txb[1] = KS_SPIOP_TXFIFO;
    ks->txh.txw[1] = fid;
    ks->txh.txw[2] = length;

    ks_reg16_write(KS_RXQCR, RXQCR_SDA);

    spi_txrx((char *)&ks->txh.txb[1], 5, 0, 0, SPI_START);
    spi_txrx((char *)packet, (unsigned int)ALIGN((unsigned int)length, 4), 0, 0, SPI_STOP);

    ks_reg16_write(KS_RXQCR, 0);
    
    return 0;
}

void net_ks8851_mac_set(const char *ethaddr)
{
    int i;
    uchar mac[6];
    char testmac[64];  

    print_net("%s", "------>  WARNING: here is bug in sscanf <-----");
    print_net("---> in HW MAC addr (%s)", ethaddr);

    //sys_sscanf(ethaddr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
    sys_sscanf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X", 
        &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

    //TODO: verification mac ranges

    print_net("---> split: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    sprintf(testmac, "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    if(strcmp(ethaddr, testmac)) {
        for(i = 0; i < ETH_ALEN; i++)
            ks_reg8_write(KS_MAR(i), mac[i]);

        print_net("Set HW MAC addr (%s)", testmac);
    } else {
        print_err("Incomming MAC (%s) is wrong. Resulting MAC is (%s)", ethaddr, testmac);
    }

    return;
}

/************************************************
 *              LOCAL FUNCTIONS                                        *
 ************************************************/

/* Write a 8 bit register to ks8851 chip  */
static void     ks_reg8_write(ushort reg, ushort val)
{
    ushort  txb[2];
    ushort  bit;

    bit = 1 << (reg & 3);

    txb[0] = MK_OP(bit, reg) | KS_SPIOP_WR;
    txb[1] = val;

    spi_txrx((char *)txb, 3, 0, 0, SPI_START | SPI_STOP);
}

/* Write a 16 bit register to ks8851 chip */
static void     ks_reg16_write(ushort reg, ushort val)
{
    ushort txb[2];

    txb[0] = MK_OP(reg & 2 ? 0xC : 0x03, reg) | KS_SPIOP_WR;
    txb[1] = val;

    spi_txrx((char *)txb, 4, 0, 0, SPI_START | SPI_STOP);
}

/* Issue read register command and return the data */
static void     ks_reg_read(ushort op, uchar *rxb, ushort len)
{
    ushort txb = op | KS_SPIOP_RD;

    spi_txrx((char *)&txb, 2, 0, 0, SPI_START);
    spi_txrx(0, 0, (char *)rxb, len, SPI_STOP);
}

/* Read 8 bit register from device  */
static ushort   ks_reg8_read(ushort reg)
{
    ushort rxb = 0;

    ks_reg_read(MK_OP(1 << (reg & 3), reg), (uchar *)&rxb, 1);

    return rxb;
}

/* Read 16 bit register from device  */
static ushort   ks_reg16_read(ushort reg)
{
    ushort rxb = 0;

    ks_reg_read(MK_OP(reg & 2 ? 0xC : 0x3, reg), (uchar *)&rxb, 2);

    return rxb;
}

/* Read 32 bit register from device */
static uint     ks_reg32_read(ushort reg)
{
    uint rxb = 0;

    ks_reg_read(MK_OP(0x0f, reg), (uchar *)&rxb, 4);

    return rxb;
}

/* set power mode of the device */
static void ks_powermode_set(ushort pwrmode)
{
    ushort pmecr;

    pmecr = ks_reg16_read(KS_PMECR);
    pmecr &= ~PMECR_PM_MASK;
    pmecr |= pwrmode;

    ks_reg16_write(KS_PMECR, pmecr);
}

/* read data from the receive fifo */
static void ks_fifo_read(uchar *buff, ushort len)
{
    uchar txb = KS_SPIOP_RXFIFO;

    spi_txrx((char *)&txb, 1, 0, 0, SPI_START);
    spi_txrx(0, 0, (char *)buff, len, SPI_STOP);
}

/* Set default HW MAC address*/
void ks_mac_default_set(void)
{
    int i;
    char ethaddr[64];

    for(i = 0; i < ETH_ALEN; i++)
        ks_reg8_write(KS_MAR(i), def_mac_addr[i]);

    sprintf(ethaddr, "%02X:%02X:%02X:%02X:%02X:%02X",
        def_mac_addr[0], def_mac_addr[1],
        def_mac_addr[2], def_mac_addr[3],
        def_mac_addr[4], def_mac_addr[5]);

    print_net(" - Set default HW MAC addr to (%s)", ethaddr);
    
    return;
}

/* configure network device */
static void ks_config(void)
{
    print_net("%s", " - Bring up power");
    /* bring chip out of any power saving mode it was in */
    ks_powermode_set(PMECR_PM_NORMAL);

    /* auto-increment tx data, reset tx pointer */
    ks_reg16_write(KS_TXFDPR, TXFDPR_TXFPAI);

    /* Enable QMU TxQ Auto-Enqueue frame */
    ks_reg16_write(KS_TXQCR, TXQCR_AETFE);

    print_net("%s", " - Setup transmission parameters");
    /* setup transmission parameters */
    ks_reg16_write(KS_TXCR, (TXCR_TXE |     /* enable transmit process */
                            TXCR_TCGIP |
                            TXCR_TCGTCP |
                            TXCR_TCGICMP |
                            TXCR_TXPE |     /* pad to min length */
                            TXCR_TXCRC |    /* add CRC */
                            TXCR_TXFCE));   /* enable flow control */

    print_net("%s", " - Setup reception parameters");
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

    return;    
}



