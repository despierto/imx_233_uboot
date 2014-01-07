/**
 * HW Print Driver header file
 *
 * Copyright (C) 2013 X-boot GITHUB team
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

#ifndef _DRV_PRINT_H_
#define _DRV_PRINT_H_

#include <stdio.h>
#include <stdarg.h>
#include "types.h"
#include "error.h"

void drv_print_printf(const char *fmt, ...);
void drv_print_assert(const char* filename, const char* funcname, const int nrow);
void drv_print_error(const char* filename, const char* funcname, const int nrow);


#undef  printf
#define printf  drv_print_printf

#define PRINTF_LOG_OK
#define PRINTF_ERR_OK
#define PRINTF_DBG_OK
#define PRINTF_INF_OK

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

#ifdef PRINTF_ERR_OK
#define print_err(fmt, args...) {drv_print_error(__FILE__, __FUNCTION__, __LINE__); printf(fmt "\r\n\r\n", ## args);}
#else  /* PRINTF_ERR_OK */
#define print_err(fmt, args...)
#endif  /* PRINTF_ERR_OK */

#ifdef PRINTF_INF_OK
#define print_inf   printf
#else  /* PRINTF_INF_OK */
#define print_inf
#endif  /* PRINTF_INF_OK */


#endif /*_DRV_PRINT_H_*/
