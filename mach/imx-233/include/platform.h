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

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

/*******************************************************************************/
/*                                               I.MX23 MEMORY MAP                                                          */
/*******************************************************************************/
#define OCRAM_BASE          0x00000000
#define OCRAM_SIZE          0x00008000              /* 32KB */

#define OCRAM_ALIAS_BASE    0x00008000
#define OCRAM_ALIAS_SIZE    0x3FFF8000

#define SDRAM_BASE          0x40000000
#define SDRAM_SIZE          0x04000000              /* 64MB of real DDR memory*/
#define SDRAM_REGION_SIZE   0x20000000              /* 512MB */

#define DEFAULT_SLAVE1_BASE 0x60000000
#define DEFAULT_SLAVE1_SIZE 0x20000000              /* 512MB */

#define REGS_BASE           0x80000000
#define REGS_SIZE           0x00100000              /* 1MB */

#define DEFAULT_SLAVE2_BASE 0x80100000
#define DEFAULT_SLAVE2_SIZE 0x3FF00000

#define OCROM_BASE          0xC0000000
#define OCROM_SIZE          0x00010000              /* 64KB */

#define OCROM_ALIAS_BASE    0xC0010000
#define OCROM_ALIAS_SIZE    0xFFFFFFFF



/*******************************************************************************/
/*                                              REGS MAPPING:  REGS_BASE..  REGS_SIZE                         */
/*******************************************************************************/
/* STMP 37xx RTC persistent register 1 bit 10 indicates
  *    that system is being resumed from suspend mode */
#define RTC_BASE_ADDR           0x8005C000
#define PERSISTENT_SLEEP_REG    0x8005C070
#define PERSISTENT_SLEEP_BIT    10


/*******************************************************************************/
/*                                              OTHER MAPPING                                                                 */
/*******************************************************************************/

#define ATAGS_BASE_ADDRESS  (SDRAM_BASE + 0x100)
#define KERNEL_BASE_ADDRESS (SDRAM_BASE + 0x8000)

#define SLEEP_STATE_FINGERPRINT 0xdeadbeef
#define FINGERPRINT             0x00                /* fingerprint offset */


/*====================*/
/* Network definitions                 */
/*====================*/
#define NET_PKT_MAX_SIZE        (2048)
#define NET_PKT_COUNT           (1000)


/*******************************************************************************/
/*                                              64MB SDRAM INTERNAL MAPPING                                        */
/*******************************************************************************/

/* MAP: 0 - base of system RAM*/
#define SYS_RAM_BASE            (SDRAM_BASE)                            /* 0x40000000 */
#define SYS_RAM_SIZE            (SDRAM_SIZE)                            /* 0x04000000 */

#define SYS_RAM_LINUX_BOOT_PARAM_ADDR   (SDRAM_BASE + 0x100)            /* 0x40000100 */




/* MAP: 32 KB - offset for kernel image loading */
#define SYS_RAM_LOAD_ADDR       (SDRAM_BASE + 0x8000)                   /* 0x40008000 */
#define SYS_RAM_LOAD_SIZE       (0x7F8000)                              /* 8MB - 32KB */

//GAP: 0x40808000...0x41FFFFFF: ~25MB

/* MAP: 32 MB - offset for devices queues */
#define SYS_RAM_ETH_STORAGE_ADDR    (SDRAM_BASE + 0x2000000)                                /* 0x42000000 */
#define SYS_RAM_ETH_STORAGE_SIZE    (NET_PKT_MAX_SIZE * NET_PKT_COUNT)                     /* 0x1F4000: 2MB*/

#define SYS_RAM_ETH_HEAP_ADDR       (SYS_RAM_ETH_STORAGE_ADDR + SYS_RAM_ETH_STORAGE_SIZE)   /* 0x421F4000 */
#define SYS_RAM_ETH_HEAP_SIZE       (0x3000)                                                /*  12288*/

#define SYS_RAM_ETH_CTX_ADDR        (SYS_RAM_ETH_HEAP_ADDR + SYS_RAM_ETH_HEAP_SIZE)         /* 0x421F7000 */
#define SYS_RAM_ETH_CTX_SIZE        (0x1000)                                                 /* 256KB*/

#define SYS_RAM_NET_CTX_ADDR        (SYS_RAM_ETH_CTX_ADDR + SYS_RAM_ETH_CTX_SIZE)           /* 0x421F8000 */
#define SYS_RAM_NET_CTX_SIZE        (0x100)                                                 /* 256KB*/

//GAP: 0x421F8100...0x43FFFFFF: ~31MB

#define SYS_RAM_END             (SDRAM_BASE + SDRAM_SIZE)               /* 0x44000000 */



/*
 * Most of 378x SoC registers are associated with four addresses
 * used for different operations - read/write, set, clear and toggle bits.
 *
 * Some of registers do not implement such feature and, thus, should be
 * accessed/manipulated via single address in common way.
 */
#define REG_RD(x)       (*(volatile unsigned int *)(x))
#define REG_WR(x, v)    ((*(volatile unsigned int *)(x)) = (v))
#define REG_SET(x, v)   ((*(volatile unsigned int *)((x) + 0x04)) = (v))
#define REG_CLR(x, v)   ((*(volatile unsigned int *)((x) + 0x08)) = (v))
#define REG_TOG(x, v)   ((*(volatile unsigned int *)((x) + 0x0c)) = (v))


/*====================*/
/* SPI Driver info */
/*====================*/
#define CONFIG_SSP_CLK      48000000
#define CONFIG_SPI_CLK      3000000
#define CONFIG_SPI_SSP1

#define CPU_CLK_DEVIDER (18) //480MHz

/*====================*/
/* ETH configuration                    */
/*====================*/
#define CONFIG_BOOTFILE         "zlinux"	                    /* Boot file name */
#define CONFIG_BOOTFILE_SIZE    128
#define CONFIG_SYS_PROMPT       "x-boot> "
#define CONFIG_NETMASK          "255.255.255.0"
#define CONFIG_IPADDR           "192.168.0.33"                /* 10.1.184.112 192.168.0.136 */
#define CONFIG_SERVERIP         "192.168.0.104"                /* 10.1.184.188 192.168.0.2 */
#define CONFIG_GATEWAYIP        "192.168.0.1"
#define CONFIG_DNSIP            "1.1.1.1"
#define CONFIG_VLANIP           "127.0.0.1"
#define CONFIG_VLANNETMASK      "255.0.0.0"
#define CONFIG_BOOTDELAY        3                               /* sec */
#define CONFIG_HW_MAC_ADDR      "00:1F:F2:00:00:00"             /* default HW MAC address */


/*====================*/
/* UART configuration                */
/*====================*/
#define CONFIG_SYS_CBSIZE   1024                                        /* Console I/O Buffer Size  */
#define CONFIG_SYS_PBSIZE   (CONFIG_SYS_CBSIZE+sizeof(CONFIG_SYS_PROMPT)+16)


#endif /* __PLATFORM_H__ */
