/*
 * Copyright 2009 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef __ASSERT_H
#define __ASSERT_H

#ifdef CODEPROTENA
#if defined (__linux__)
#include "drv_print.h"
static inline void _assert(int rc, const char* filename, const char* funcname, const int nRow)
{
     printf("\r\n---- ASSERT---\r\n\r\n");
     printf("       RC:         0x%08x\r\n", rc);
     printf("       File:       %s\r\n", filename);
     printf("       Function:   %s\r\n", filename);     
     printf("       Row:        %d\r\n", nRow);
 }

    #define     SystemHalt(x) _assert(x, __FILE__, __FUNCTION__,__LINE__)
#elif defined (__THUMB)
    #define     SystemHalt(x) __asm(" .half 0xdead")
#else
    #define     SystemHalt(x) __asm(" .word 0xdeadc0de");
#endif

#define assert(x) do {if(!(x)) SystemHalt(x);} while(0)

#else /*CODEPROTENA*/
#define assert(x)

#endif /*CODEPROTENA*/

#endif //__ASSERT_H
