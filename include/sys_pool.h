/**
 * SW sys pool header file
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

#ifndef _SYS_POOL_H_
#define _SYS_POOL_H_

#include "types.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
#define SYS_POOL_CAPTION_LEN    16

typedef struct _SYS_POOL_ITEM_ {
    U32         *next;
    U32         addr;
    U32         status;    
}SYS_POOL_ITEM, *PSYS_POOL_ITEM;

typedef struct _SYS_POOL_CTX_ 
{
    PSYS_POOL_ITEM  pool;
    PSYS_POOL_ITEM  next_alloc;    
    PSYS_POOL_ITEM  next_free;        
    U32             stats_alloc;
    U32             stats_free;
    U32             stats_balance;
    U32             storage_base;    
    U32             storage_end;    
    U32             pool_items_count;
    U32             pool_item_size;
    U8              pool_caption[SYS_POOL_CAPTION_LEN];    
}SYS_POOL_CTX, *PSYS_POOL_CTX;



/************************************************
  *             FUNCTIONS                                                   *
  ************************************************/

PSYS_POOL_CTX   sys_pool_init(U32 pool_items_count, U32 pool_item_size, U8 *pool_caption);
void            sys_pool_info(PSYS_POOL_CTX pPoolCtx);
PTR             sys_pool_alloc(PSYS_POOL_CTX pPoolCtx);
int             sys_pool_free(PSYS_POOL_CTX pPoolCtx, PTR ptr);
int             sys_pool_close(PSYS_POOL_CTX pPoolCtx);


#endif /*_SYS_POOL_H_*/

