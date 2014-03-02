/**
 * SW Sys utils header file
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


#ifndef __SYS_UTILS_H__
#define __SYS_UTILS_H__

#include "types.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
 
/* we use this so that we can do without the ctype library */
#define is_digit(c) ((c) >= '0' && (c) <= '9')

#define __swab16(x) \
    ((U16)( \
        (((U16)(x) & (U16)0x00ffU) << 8) | \
        (((U16)(x) & (U16)0xff00U) >> 8) ))
#define __swab32(x) \
    ((U32)( \
        (((U32)(x) & (U32)0x000000ffUL) << 24) | \
        (((U32)(x) & (U32)0x0000ff00UL) <<  8) | \
        (((U32)(x) & (U32)0x00ff0000UL) >>  8) | \
        (((U32)(x) & (U32)0xff000000UL) >> 24) ))
#define __swab64(x) \
    ((U64)( \
        (U64)(((U64)(x) & (U64)0x00000000000000ffULL) << 56) | \
        (U64)(((U64)(x) & (U64)0x000000000000ff00ULL) << 40) | \
        (U64)(((U64)(x) & (U64)0x0000000000ff0000ULL) << 24) | \
        (U64)(((U64)(x) & (U64)0x00000000ff000000ULL) <<  8) | \
        (U64)(((U64)(x) & (U64)0x000000ff00000000ULL) >>  8) | \
        (U64)(((U64)(x) & (U64)0x0000ff0000000000ULL) >> 24) | \
        (U64)(((U64)(x) & (U64)0x00ff000000000000ULL) >> 40) | \
        (U64)(((U64)(x) & (U64)0xff00000000000000ULL) >> 56) ))

#if 1 /* little endian */
#define __cpu_to_be64(x) __swab64((x))
#define __be64_to_cpu(x) __swab64((x))
#define __cpu_to_be32(x) __swab32((x))
#define __be32_to_cpu(x) __swab32((x))
#define __cpu_to_be16(x) __swab16((x))
#define __be16_to_cpu(x) __swab16((x))
#else  /*big endian */
#define __cpu_to_be64(x) ((U64)(x))
#define __be64_to_cpu(x) ((U64)(x))
#define __cpu_to_be32(x) ((U32)(x))
#define __be32_to_cpu(x) ((U32)(x))
#define __cpu_to_be16(x) ((U16)(x))
#define __be16_to_cpu(x) ((U16)(x))
#endif


#define ___htonl(x) __cpu_to_be32(x)
#define ___htons(x) __cpu_to_be16(x)
#define ___ntohl(x) __be32_to_cpu(x)
#define ___ntohs(x) __be16_to_cpu(x)

#define htonl(x) ___htonl(x)
#define ntohl(x) ___ntohl(x)
#define htons(x) ___htons(x)
#define ntohs(x) ___ntohs(x)

















/************************************************
  *              GLOBAL FUNCTIONS                                      *
  ************************************************/

/* sys_utils.h */
void    memset(void * s, unsigned int c, unsigned int count);
void    copy_filename(char *dst, char *src, int size);
int     strnlen(const char *s, unsigned int len);
void    sys_print_assert(const char* filename, const char* funcname, const int nrow);
void    sys_print_error(const char* filename, const char* funcname, const int nrow);
int     strcmp(const char * cs,const char * ct);
int     strncmp(const char * cs,const char * ct, size_t count);
void    *memcpy(void *s1, const void *s2, int n);

/* sys_vfprintf.h */
int     sprintf(char * buf, const char *fmt, ...);
int     sys_sscanf(const char * buf, const char * fmt, ...);
unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);
int     sys_checksum_ok(uchar * ptr, int len);
uint    sys_checksum(uchar * ptr, int len);




/************************************************
  *              GLOBAL PRINTF COLLECTION                        *
  ************************************************/

#define PRINTF_HW_OK

#define PRINTF_LOG_OK
#define PRINTF_ERR_OK
#define PRINTF_DBG_OK

#define PRINTF_INF_OK
#define PRINTF_NET_OK
#define PRINTF_ETH_OK
#define PRINTF_CLK_OK
#define PRINTF_SPI_OK
#define PRINTF_PIN_OK

#undef  printf

#if 0 //light printf
void drv_print_printf(const char *fmt, ...);
#define printf  drv_print_printf
#else
void sys_printf(const char *fmt, ...);
#define printf  sys_printf
#endif

/********************************************************************************
 *          Hardware Initialization type of printing                                                                        *
 ********************************************************************************/
#ifdef PRINTF_HW_OK
#define print_hw(fmt, args...) printf("[hw] " fmt "\r\n", ## args)
#else  /* PRINTF_HW_OK */
#define print_hw(fmt, args...)
#endif  /* PRINTF_HW_OK */


/********************************************************************************
 *          X-BOOT Initialization type of printing                                                                          *
 *              Policy: [xxx] - 3 symbols are describing device/module type                                   *
 ********************************************************************************/
#ifdef PRINTF_LOG_OK
#define print_log(fmt, args...) printf("%s: " fmt "\r\n", __FUNCTION__, ## args)
#else  /* PRINTF_LOG_OK */
#define print_log(fmt, args...)
#endif  /* PRINTF_LOG_OK */

#ifdef PRINTF_DBG_OK
#define print_dbg(fmt, args...) printf("[dbg] %s: " fmt "\r\n", __FUNCTION__, ## args)
#else  /* PRINTF_DBG_OK */
#define print_dbg(fmt, args...)
#endif  /* PRINTF_DBG_OK */

#ifdef PRINTF_CLK_OK
#define print_clk(fmt, args...) printf("[clk] " fmt "\r\n", ## args)
#else  /* PRINTF_CLK_OK */
#define print_clk(fmt, args...)
#endif  /* PRINTF_CLK_OK */

#ifdef PRINTF_NET_OK
#define print_net(fmt, args...) printf("[net] " fmt "\r\n", ## args)
#else  /* PRINTF_NET_OK */
#define print_net(fmt, args...)
#endif  /* PRINTF_NET_OK */

#ifdef PRINTF_ETH_OK
#define print_eth(fmt, args...) printf("[eth] " fmt "\r\n", ## args)
#else  /* PRINTF_ETH_OK */
#define print_eth(fmt, args...)
#endif  /* PRINTF_ETH_OK */

#ifdef PRINTF_SPI_OK
#define print_spi(fmt, args...) printf("[spi] " fmt "\r\n", ## args)
#else  /* PRINTF_SPI_OK */
#define print_spi(fmt, args...)
#endif  /* PRINTF_SPI_OK */

#ifdef PRINTF_PIN_OK
#define print_pin(fmt, args...) printf("[pin] " fmt "\r\n", ## args)
#else  /* PRINTF_PIN_OK */
#define print_pin(fmt, args...)
#endif  /* PRINTF_PIN_OK */

#ifdef PRINTF_ERR_OK
#define print_err(fmt, args...) {sys_print_error(__FILE__, __FUNCTION__, __LINE__); printf(fmt "\r\n\r\n", ## args);}
#else  /* PRINTF_ERR_OK */
#define print_err(fmt, args...)
#endif  /* PRINTF_ERR_OK */

#ifdef PRINTF_INF_OK
#define print_inf   printf
#else  /* PRINTF_INF_OK */
#define print_inf
#endif  /* PRINTF_INF_OK */


#ifdef CODEPROTENA
#if defined (__linux__)
    #define     SystemHalt sys_print_assert
#elif defined (__THUMB)
    #define     SystemHalt(fl, fn, ln) __asm(" .half 0xdead")
#else
    #define     SystemHalt(fl, fn, ln) __asm(" .word 0xdeadc0de")
#endif

#define assert(x) if (!(x)) {SystemHalt(__FILE__, __FUNCTION__, __LINE__); while(1);}

#else /*CODEPROTENA*/
#define assert(x)

#endif /*CODEPROTENA*/


#endif /*__SYS_UTILS_H__*/

