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

#include "global.h"
#include "sys_heap.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
#define SYS_HEAM_MIN_STORAGE_SIZE(aligment, blocks) (aligment + ROUND_UP(blocks*sizeof(HEAPBLOCK), aligment))



/************************************************
  *              GLOBAL FUNCTIONS                                      *
  ************************************************/
PHEAPHEADER	sys_heap_init(PHEAPHEADER hp, U32 storage_addr, U32 storage_size, 
							U8 alignment, U32 blocks_count)
{
	PHEAPBLOCK	pheap_block;	
	U32 heap_blocks_size;

	print_sys("Heap initialization: addr_%x size_%d alignment_%d blocks_%d", 
		storage_addr, storage_size, alignment, blocks_count);

	if (!blocks_count) {
		print_err("%s", "blocks count is null");
		return NULL;
	}

	if (!storage_addr) {
		print_err("%s", "storage address is null");
		return NULL;
	}

	if (storage_size < SYS_HEAM_MIN_STORAGE_SIZE(alignment, blocks_count)) {
		print_err("storage size (%d) is less than min size (%d)", 
			storage_size, SYS_HEAM_MIN_STORAGE_SIZE(alignment, blocks_count));
		return NULL;
	}
	
	//init heap list
	hp->pheap_block_list = (PHEAPBLOCK)storage_addr;
	heap_blocks_size = ROUND_UP(blocks_count * sizeof(HEAPBLOCK), alignment);
	hp->storage = (void *)(storage_addr + heap_blocks_size);
	hp->blocks_count = blocks_count;
	hp->alignment = alignment;

	memset((void *)hp->pheap_block_list, 0, heap_blocks_size);

	pheap_block = hp->pheap_block_list;

	//build head list chain
	pheap_block[blocks_count-1].next_block = 0;

	while (--blocks_count)
		pheap_block[blocks_count-1].next_block = &pheap_block[blocks_count];

	//init head block
	pheap_block = &hp->current_block;

	pheap_block->next_block = pheap_block;
	pheap_block->prev_block = pheap_block;
	pheap_block->buffer = hp->storage;	
	pheap_block->size = storage_size - heap_blocks_size;
	pheap_block->used = FALSE;

	return hp;
}

void *sys_heap_alloc(PHEAPHEADER hp, U32 size)
{
	PHEAPBLOCK	head_block = &hp->current_block;
	PHEAPBLOCK	current_block = head_block;	
	PHEAPBLOCK 	new_block, next_block;
	void *ptr = NULL;

	size = ROUND_UP(size, hp->alignment);

	do 
	{
		if (!current_block->used && current_block->size >= size) 
		{

			ptr = current_block->buffer;

			if (current_block->size != size) 
			{
				// split current block for two new blocks due its size is bigger the required block size
				new_block = hp->pheap_block_list->next_block;
				if (new_block) {
					hp->pheap_block_list->next_block = new_block->next_block;
				} else {
					print_err("%s", "no blocks");
					return ptr;
				}
		
				next_block = current_block->next_block;
				new_block->next_block = next_block;
				new_block->prev_block = current_block;
				new_block->size = current_block->size - size;
				new_block->buffer = (void *)((U8 *)ptr + size);
				new_block->used = FALSE;

				current_block->next_block = new_block;
				current_block->size = size;
				current_block->used = TRUE;

				next_block->prev_block = new_block;
			} else {
				current_block->used = TRUE;
			}
			
			break;
		}
		
		current_block = current_block->next_block;

	} while ((U32)current_block != (U32)head_block);

	if (ptr == NULL) {
		print_err("%s", "no mem");
	}

	return ptr;
}

int sys_heap_free(PHEAPHEADER hp, void * ptr)
{
	PHEAPBLOCK	head_block = &hp->current_block;
	PHEAPBLOCK	current_block = head_block;	
	PHEAPBLOCK 	prev_block, next_block;
	int rc = FAILURE;

	if (ptr == NULL) {
		//print_err("%s", "incomming ptr is null");
		return rc;
	}

	do
	{
		if ((U32)current_block->buffer == (U32)ptr)
		{
			if (current_block->used)
			{
				// in case current block and next are free let's merge them
				next_block = current_block->next_block;
				if ((next_block->used == FALSE) && ((U32)next_block != (U32)head_block))
				{
					current_block->next_block = next_block->next_block;
					current_block->size += next_block->size;

					//release next block
					next_block->next_block = hp->pheap_block_list->next_block;
					hp->pheap_block_list->next_block = next_block;

					next_block = current_block->next_block;
					next_block->prev_block = current_block;
				}

				// in case current block and previous are free let's merge them
				prev_block = current_block->prev_block;
				if ((prev_block->used == FALSE) && ((U32)current_block != (U32)head_block))
				{
					prev_block->next_block = current_block->next_block;
					prev_block->size += current_block->size;
					
					next_block = current_block->next_block;

					//release current block
					current_block->next_block = hp->pheap_block_list->next_block;
					hp->pheap_block_list->next_block = current_block;

					next_block->prev_block = prev_block;
				} else {
					current_block->used = FALSE;
				}
			}
			
			rc = SUCCESS;
			
			break;
		}

		current_block = current_block->next_block;
		
	} while ((U32)current_block != (U32)head_block);

	if (rc != SUCCESS) {
		print_err("%s", "operation not completed");
	}

	return rc;
}

int  sys_heap_close(PHEAPHEADER hp)
{
	int rc = SUCCESS;

	//nothing to do
	
	return rc;
}


/************************************************
  *              LOCAL FUNCTIONS                                      *
  ************************************************/



