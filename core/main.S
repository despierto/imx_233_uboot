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

  .file     "main.S"
    .text

    .globl	__div0
    .globl  __aeabi_unwind_cpp_pr0
    .globl  __aeabi_unwind_cpp_pr1    
    .globl  raise

/*kiss gcc's ass to make it happy
//! void __aeabi_unwind_cpp_pr0() {}
//! void __aeabi_unwind_cpp_pr1() {}
////////////////////////////////*/

__div0:
    mov     pc, lr
    
__aeabi_unwind_cpp_pr0:
    mov     pc, lr

__aeabi_unwind_cpp_pr1:
    mov     pc, lr    

raise:
    mov     pc, lr
