/**
 * SW sys list header file
 *
 * Copyright (c) 2014 Alex Winter (eterno.despierto@gmail.com)
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

#ifndef _SYS_LIST_H_
#define _SYS_LIST_H_

#include "types.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
#define SYS_LIST_CAPTION_LEN    16

typedef struct _SYS_LIST_ITEM_ {
    U32         *next;
    U32         addr;
    U32         status;    
}SYS_LIST_ITEM, *PSYS_LIST_ITEM;

typedef struct _SYS_LIST_CTX_ 
{
    PSYS_LIST_ITEM  list;
    PSYS_LIST_ITEM  next_alloc;    
    PSYS_LIST_ITEM  next_free;        
    U32             stats_alloc;
    U32             stats_free;
    U32             stats_balance;
    U32             storage_base;    
    U32             storage_end;    
    U32             list_items_count;
    U32             list_item_size;
    U8              list_caption[SYS_LIST_CAPTION_LEN];    
}SYS_LIST_CTX, *PSYS_LIST_CTX;



/************************************************
  *             FUNCTIONS                                                   *
  ************************************************/

PSYS_LIST_CTX   sys_list_init(U32 list_items_count, U32 list_item_size, U8 *list_caption);
void            sys_list_info(PSYS_LIST_CTX pListCtx);
PTR             sys_list_alloc(PSYS_LIST_CTX pListCtx);
int             sys_list_free(PSYS_LIST_CTX pListCtx, PTR ptr);
int             sys_list_close(PSYS_LIST_CTX pListCtx);


#endif /*_SYS_LIST_H_*/

