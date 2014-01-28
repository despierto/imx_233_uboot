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
    #include "sys_utils.h"
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

#endif //__ASSERT_H
