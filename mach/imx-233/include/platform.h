/*
 * STMP37XX specific definitions
 *
 * Vladislav Buzov <vbuzov@embeddedalley.com>
 *
 * Copyright 2008 SigmaTel, Inc
 * Copyright 2008 Embedded Alley Solutions, Inc
 * Copyright 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2.  This program  is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
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

#define OCRAM_BASE          0xC0000000
#define OCRAM_SIZE          0x00010000              /* 64KB */

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


#endif /* __PLATFORM_H__ */
