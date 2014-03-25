/**
 * SW sys list file
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

#include "global.h"
#include "sys_list.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/




/************************************************
  *              GLOBAL FUNCTIONS                                      *
  ************************************************/
PSYS_LIST_CTX sys_list_init(U32 list_items_count, U32 list_item_size, U8 *list_caption)
{
    PSYS_LIST_CTX pListCtx;
    U32 storage_base;
    U32 i;

    print_eth("List %s init", list_caption);

    pListCtx = (PSYS_LIST_CTX)malloc(sizeof(SYS_LIST_CTX));
    if (pListCtx == NULL) {
        assert(pListCtx);        
        return NULL;
    }
    memset((void *)pListCtx, 0, sizeof(SYS_LIST_CTX));
    
    pListCtx->list = (PSYS_LIST_ITEM)malloc(list_items_count * sizeof(SYS_LIST_ITEM));
    if (pListCtx->list == NULL) {
        free(pListCtx);
        assert(pListCtx->list);        
        return NULL;
    }
    memset((void *)pListCtx->list, 0, list_items_count * sizeof(SYS_LIST_ITEM));

    storage_base = (U32)malloc(list_item_size * (list_items_count + 1));
    if (storage_base == 0) {
        free(pListCtx->list);
        free(pListCtx);
        assert(storage_base);        
        return NULL;
    }

    if (list_caption) {
        memcpy((void *)pListCtx->list_caption, (void *)list_caption, strnlen((char *)list_caption, SYS_LIST_CAPTION_LEN));
    } else {
        memcpy((void *)pListCtx->list_caption, (void *)"default list", 12);
    }

    //align to 0x800
    storage_base = storage_base + (list_item_size -1);    
    storage_base -= storage_base % list_item_size;

    print_sys(" - pListCtx (0x%x) list[c%d x s%x] (0x%x) storage_base (0x%x)", 
        (U32)pListCtx, list_items_count, list_item_size, (U32)pListCtx->list, storage_base);
        
    pListCtx->storage_base = storage_base;
    pListCtx->storage_end = storage_base + list_item_size * list_items_count;    
    pListCtx->list_items_count = list_items_count;
    pListCtx->list_item_size = list_item_size;    

    for (i=0; i<(list_items_count - 1); i++) {
        pListCtx->list[i].addr = (U32)(storage_base + list_item_size * i);
        pListCtx->list[i].next = (U32 *)&pListCtx->list[i+1];
        pListCtx->list[i].status = 0;
        //print_eth("   [%d] addr_%x next_%x", i, pListCtx->list[i].addr, (U32)pListCtx->list[i].next);
    }
    pListCtx->list[i].addr = (U32)(storage_base + list_item_size * i);
    pListCtx->list[i].next = NULL;
    pListCtx->list[i].status = 0;
    //print_eth("   [%d] addr_%x next_%x", i, pListCtx->list[i].addr, (U32)pListCtx->list[i].next);

    pListCtx->next_alloc = &pListCtx->list[0];
    pListCtx->next_free = &pListCtx->list[i];

    //print_eth(" - next AI[%d]_%x FI[%d]_%x", 0, pListCtx->next_alloc, i, (U32)pListCtx->next_free);

    return pListCtx;
}

void sys_list_info(PSYS_LIST_CTX pListCtx)
{
    assert(pListCtx);
    
    print_eth("(%s) status:", pListCtx->list_caption);
    print_eth(" - alloc count:  %d", pListCtx->stats_alloc);
    print_eth(" - free  count:  %d", pListCtx->stats_free);
    print_eth(" - balance:      %d", pListCtx->stats_balance);
    
    return;
}

PTR sys_list_alloc(PSYS_LIST_CTX pListCtx)
{
    PTR addr;

    assert(pListCtx);

    if (pListCtx->next_alloc) {
        addr = (PTR)pListCtx->next_alloc->addr;
        pListCtx->next_alloc->status = 1;
        pListCtx->next_alloc = (PSYS_LIST_ITEM)pListCtx->next_alloc->next;
        pListCtx->stats_alloc++;
        pListCtx->stats_balance++;
            
        //print_eth("[dbg] ---> net heap alloc: addr_%x", (U32)addr);
    } else {
        print_err("%s is full", pListCtx->list_caption);
        addr = NULL;
    }
    
    return addr;
}

int sys_list_free(PSYS_LIST_CTX pListCtx, PTR ptr)
{
    PSYS_LIST_ITEM pList;
    U32 index;

    assert(pListCtx);

    if (((U32)ptr < pListCtx->storage_base) || ((U32)ptr >= pListCtx->storage_end)) {
        print_err("ptr 0x%x is out of %s range", (unsigned int)ptr, pListCtx->list_caption);
        return FAILURE;
    }

    index = ((U32)ptr - pListCtx->storage_base)/NET_PKT_MAX_SIZE;
    pList = &pListCtx->list[index];
    
    if (!pList->status) {
        print_err("%s: double free", pListCtx->list_caption);
        return FAILURE;
    }

    //print_eth("[dbg] ---> net heap free: addr_%x", (U32)ptr);

    pList->next = NULL;
    pList->status = 0;
    
    pListCtx->next_free->next = (U32 *)pList;
    pListCtx->next_free = pList;
    pListCtx->stats_free++;
    pListCtx->stats_balance--;
    
    return SUCCESS;
}

int sys_list_close(PSYS_LIST_CTX pListCtx)
{
    int rc = SUCCESS;
    assert(pListCtx);

    rc = free((void *)pListCtx->storage_base);
    rc |= free(pListCtx->list);
    rc |= free(pListCtx);

    return rc;
}


/************************************************
  *              LOCAL FUNCTIONS                                      *
  ************************************************/



