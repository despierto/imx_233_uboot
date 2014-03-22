/**
 * SW Sys utils file
 *
 * Copyright (c) 2014 Alex Winter 
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

#ifndef _SYS_HEAP_H_
#define _SYS_HEAP_H_

/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
typedef struct _HEAP_BLOCK_ {
	struct _HEAP_BLOCK_ *next_block;
	struct _HEAP_BLOCK_ *prev_block;
	void * 	buffer;
	U32    	size;	
	U32	   	used;
} HEAPBLOCK, *PHEAPBLOCK;

typedef struct _HEAP_HEADER_ {
	HEAPBLOCK	current_block;		
	PHEAPBLOCK	pheap_block_list;		
	void		*storage;		
	U32			blocks_count;	
	U32     	alignment;
} HEAPHEADER, *PHEAPHEADER;



/************************************************
  *             FUNCTIONS                                                   *
  ************************************************/
PHEAPHEADER	sys_heap_init(PHEAPHEADER hp, U32 addr, U32 size, U8 aligment, U32 blocks_count);
void 		*sys_heap_alloc(PHEAPHEADER hp, U32 size);
int 		sys_heap_free(PHEAPHEADER hp, void * ptr);
int  		sys_heap_close(PHEAPHEADER hp);


#endif /*_SYS_HEAP_H_*/
