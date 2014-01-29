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

/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
 
/* we use this so that we can do without the ctype library */
#define is_digit(c) ((c) >= '0' && (c) <= '9')




















/************************************************
  *              GLOBAL FUNCTIONS                                      *
  ************************************************/

/* sys_utils.h */
void    memset(void * s, unsigned int c, unsigned int count);
void    copy_filename(char *dst, char *src, int size);
int     strnlen(const char *s, unsigned int len);
void    sys_print_assert(const char* filename, const char* funcname, const int nrow);
void    sys_print_error(const char* filename, const char* funcname, const int nrow);



/* sys_vfprintf.h */
int     sprintf(char * buf, const char *fmt, ...);
int     sscanf(const char * buf, const char * fmt, ...);






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


void sys_printf(const char *fmt, ...);

#undef  printf
#define printf  sys_printf


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





#endif /*__SYS_UTILS_H__*/

