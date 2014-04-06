/**
 * SW sys pool file
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
#include "sys_pool.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/




/************************************************
  *              GLOBAL FUNCTIONS                                      *
  ************************************************/
PSYS_POOL_CTX sys_pool_init(U32 pool_items_count, U32 pool_item_size, U8 *pool_caption)
{
    PSYS_POOL_CTX pPoolCtx;
    U32 storage_base;
    U32 i;

    print_eth("List %s init", pool_caption);

    pPoolCtx = (PSYS_POOL_CTX)malloc(sizeof(SYS_POOL_CTX));
    if (pPoolCtx == NULL) {
        assert(pPoolCtx);        
        return NULL;
    }
    memset((void *)pPoolCtx, 0, sizeof(SYS_POOL_CTX));
    
    pPoolCtx->pool = (PSYS_POOL_ITEM)malloc(pool_items_count * sizeof(SYS_POOL_ITEM));
    if (pPoolCtx->pool == NULL) {
        free(pPoolCtx);
        assert(pPoolCtx->pool);        
        return NULL;
    }
    memset((void *)pPoolCtx->pool, 0, pool_items_count * sizeof(SYS_POOL_ITEM));

    storage_base = (U32)malloc(pool_item_size * (pool_items_count + 1));
    if (storage_base == 0) {
        free(pPoolCtx->pool);
        free(pPoolCtx);
        assert(storage_base);        
        return NULL;
    }

    if (pool_caption) {
        memcpy((void *)pPoolCtx->pool_caption, (void *)pool_caption, strnlen((char *)pool_caption, SYS_POOL_CAPTION_LEN));
    } else {
        memcpy((void *)pPoolCtx->pool_caption, (void *)"default pool", 12);
    }

    //align to 0x800
    storage_base = storage_base + (pool_item_size -1);    
    storage_base -= storage_base % pool_item_size;

    print_sys(" - pPoolCtx (0x%x) pool[c%d x s%x] (0x%x) storage_base (0x%x)", 
        (U32)pPoolCtx, pool_items_count, pool_item_size, (U32)pPoolCtx->pool, storage_base);
        
    pPoolCtx->storage_base = storage_base;
    pPoolCtx->storage_end = storage_base + pool_item_size * pool_items_count;    
    pPoolCtx->pool_items_count = pool_items_count;
    pPoolCtx->pool_item_size = pool_item_size;    

    for (i=0; i<(pool_items_count - 1); i++) {
        pPoolCtx->pool[i].addr = (U32)(storage_base + pool_item_size * i);
        pPoolCtx->pool[i].next = (U32 *)&pPoolCtx->pool[i+1];
        pPoolCtx->pool[i].status = 0;
        //print_eth("   [%d] addr_%x next_%x", i, pPoolCtx->pool[i].addr, (U32)pPoolCtx->pool[i].next);
    }
    pPoolCtx->pool[i].addr = (U32)(storage_base + pool_item_size * i);
    pPoolCtx->pool[i].next = NULL;
    pPoolCtx->pool[i].status = 0;
    //print_eth("   [%d] addr_%x next_%x", i, pPoolCtx->pool[i].addr, (U32)pPoolCtx->pool[i].next);

    pPoolCtx->next_alloc = &pPoolCtx->pool[0];
    pPoolCtx->next_free = &pPoolCtx->pool[i];

    //print_eth(" - next AI[%d]_%x FI[%d]_%x", 0, pPoolCtx->next_alloc, i, (U32)pPoolCtx->next_free);

    return pPoolCtx;
}

void sys_pool_info(PSYS_POOL_CTX pPoolCtx)
{
    assert(pPoolCtx);
    
    print_eth("(%s) status:", pPoolCtx->pool_caption);
    print_eth(" - alloc count:  %d", pPoolCtx->stats_alloc);
    print_eth(" - free  count:  %d", pPoolCtx->stats_free);
    print_eth(" - balance:      %d", pPoolCtx->stats_balance);
    
    return;
}

PTR sys_pool_alloc(PSYS_POOL_CTX pPoolCtx)
{
    PTR addr;

    assert(pPoolCtx);

    if (pPoolCtx->next_alloc) {
        addr = (PTR)pPoolCtx->next_alloc->addr;
        pPoolCtx->next_alloc->status = 1;
        pPoolCtx->next_alloc = (PSYS_POOL_ITEM)pPoolCtx->next_alloc->next;
        pPoolCtx->stats_alloc++;
        pPoolCtx->stats_balance++;
            
        //print_eth("[dbg] ---> net heap alloc: addr_%x", (U32)addr);
    } else {
        print_err_cmd("%s is full", pPoolCtx->pool_caption);
        addr = NULL;
    }
    
    return addr;
}

int sys_pool_free(PSYS_POOL_CTX pPoolCtx, PTR ptr)
{
    PSYS_POOL_ITEM pPool;
    U32 index;

    assert(pPoolCtx);

    //print_eth("[dbg] ---> net heap free: addr_%x", (U32)ptr);

    if (((U32)ptr < pPoolCtx->storage_base) || ((U32)ptr >= pPoolCtx->storage_end)) {
        print_err("ptr 0x%x is out of %s range", (unsigned int)ptr, pPoolCtx->pool_caption);
        return FAILURE;
    }

    index = ((U32)ptr - pPoolCtx->storage_base)/pPoolCtx->pool_item_size;
    pPool = &pPoolCtx->pool[index];
    
    if (!pPool->status) {
        print_err("%s: double free", pPoolCtx->pool_caption);
        return FAILURE;
    }

    pPool->next = NULL;
    pPool->status = 0;
    
    pPoolCtx->next_free->next = (U32 *)pPool;
    pPoolCtx->next_free = pPool;
    pPoolCtx->stats_free++;
    pPoolCtx->stats_balance--;
    
    return SUCCESS;
}

int sys_pool_close(PSYS_POOL_CTX pPoolCtx)
{
    int rc = SUCCESS;
    assert(pPoolCtx);

    rc = free((void *)pPoolCtx->storage_base);
    rc |= free(pPoolCtx->pool);
    rc |= free(pPoolCtx);

    return rc;
}

int sys_pool_test(PSYS_POOL_CTX pPoolCtx)
{
    PTR addr;
    int rc;
    
    addr = sys_pool_alloc(pPoolCtx);
    if (!addr) {
        print_err("%s alloc", pPoolCtx->pool_caption);
        return FAILURE;
    }
    rc = sys_pool_free(pPoolCtx, addr);
    if (rc) {
        print_err("%s free", pPoolCtx->pool_caption);
        return rc;
    }
    
    return SUCCESS;
}


/************************************************
  *              LOCAL FUNCTIONS                                      *
  ************************************************/



