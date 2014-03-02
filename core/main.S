/**
 * X-Boot Operation System Asm Entry
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
