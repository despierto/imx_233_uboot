/**
 * X-Boot Operation System
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
#include "core.h"


/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
static PCORE_CTX pCoreCtx = NULL;

static int local_core_rem_from_list(PCORE_TASK_CLASS pTaskClass, PCORE_TASK_CTX pTask);
static int local_core_add_to_list(PCORE_TASK_CLASS pTaskClass, PCORE_TASK_CTX pTask);


/************************************************
  *              GLOBAL  FUNCTIONs                                     *
  ************************************************/
int core_init(void)
{
    int rc = SUCCESS;

    print_log("%s", "...");

    pCoreCtx = (PCORE_CTX)malloc(sizeof(CORE_CTX));
    assert(pCoreCtx);
    memset((void *)pCoreCtx, 0, sizeof(CORE_CTX));

    //allocate pool for priority tasks
    pCoreCtx->ptask.ctx = sys_pool_init(CORE_TASKS_NUM_PRIORITY, sizeof(CORE_TASK_CTX), (U8 *)&(CORE_TASKS_CAPTION_PRIORITY));
    assert(pCoreCtx->ptask.ctx);
    assert_rc(sys_pool_test(pCoreCtx->ptask.ctx));
    pCoreCtx->ptask.list_end = NULL;
    pCoreCtx->ptask.list_end = NULL;   

    //allocate pool for common tasks
    pCoreCtx->ctask.ctx = sys_pool_init(CORE_TASKS_NUM_COMMON, sizeof(CORE_TASK_CTX), (U8 *)&(CORE_TASKS_CAPTION_COMMON));
    assert(pCoreCtx->ctask.ctx);
    assert_rc(sys_pool_test(pCoreCtx->ctask.ctx));
    pCoreCtx->ctask.list_end = NULL;
    pCoreCtx->ctask.list_end = NULL;   
    
    //allocate pool for idle task
    pCoreCtx->itask.ctx = sys_pool_init(CORE_TASKS_NUM_IDLE, sizeof(CORE_TASK_CTX), (U8 *)&(CORE_TASKS_CAPTION_IDLE));
    assert(pCoreCtx->itask.ctx);
    assert_rc(sys_pool_test(pCoreCtx->itask.ctx));
    pCoreCtx->itask.list_end = NULL;
    pCoreCtx->itask.list_end = NULL;   

    return rc;
}

int core_close(void)
{
    int rc = SUCCESS;

    print_log("%s", "...");

    sys_pool_close(pCoreCtx->ptask.ctx);
    sys_pool_close(pCoreCtx->ctask.ctx);
    sys_pool_close(pCoreCtx->itask.ctx);    
    
    free(pCoreCtx);

    return rc;
}

void core_info(void)
{
    sys_pool_info(pCoreCtx->ptask.ctx);
    sys_pool_info(pCoreCtx->ctask.ctx);
    sys_pool_info(pCoreCtx->itask.ctx);  
    
    return;
}

void core_dispatcher_task(void)
{



    return;
}

int     core_reg_task(CORE_PROC *procedure, void * proc_param, U32 period_ms, CORE_TASK_TYPE type, U16 priority)
{
    int rc = SUCCESS;
    PCORE_TASK_CLASS pTaskClass;
    PCORE_TASK_CTX pTask;

    assert(procedure);
    
    switch(type)
    {
        case CORE_TASK_TYPE_PRIORITY:
            pTaskClass = &pCoreCtx->ptask;
                
            break;
        case CORE_TASK_TYPE_COMMON:
            pTaskClass = &pCoreCtx->ctask;

            break;
        case CORE_TASK_TYPE_IDLE:
            pTaskClass = &pCoreCtx->itask;

            break;
        default:
            print_err("unknown task type %d", type);
            return FAILURE;
            break;
    }
    assert(pTaskClass);
    
    pTask = (PCORE_TASK_CTX)sys_pool_alloc(pTaskClass->ctx);
    if (pTask == NULL) {
        print_err_cmd("%s overflow", pTaskClass->ctx->pool_caption);
        return FAILURE;
    }

    pTask->procedure = procedure;
    pTask->proc_param = proc_param;
    pTask->period_ms = period_ms;
    pTask->priority = priority;
    pTask->type = type;
       
    return local_core_add_to_list(pTaskClass, pTask);
}

int     core_unreg_task(CORE_PROC *procedure, CORE_TASK_TYPE type)
{
    int rc = SUCCESS;
    PCORE_TASK_CLASS pTaskClass;
    PCORE_TASK_CTX pTask;

    assert(procedure);
    
    switch(type)
    {
        case CORE_TASK_TYPE_PRIORITY:
            pTaskClass = &pCoreCtx->ptask;
                
            break;
        case CORE_TASK_TYPE_COMMON:
            pTaskClass = &pCoreCtx->ctask;

            break;
        case CORE_TASK_TYPE_IDLE:
            pTaskClass = &pCoreCtx->itask;

            break;
        default:
            print_err("unknown task type %d", type);
            return FAILURE;
            break;
    }
    assert(pTaskClass);

    pTask = pTaskClass->list_head;
    while (pTask) {
        if ((U32)pTask->procedure == (U32)procedure) {
            break;
        }
        pTask = pTask->next;
    }

    if (pTask == NULL) {
        print_err_cmd("%s has no task 0x%x", (U32)procedure);
        return FAILURE;
    }

    assert_rc(local_core_rem_from_list(pTaskClass, pTask));
    assert_rc(sys_pool_free(pTaskClass->ctx, pTask));
   
    return local_core_add_to_list(pTaskClass, pTask);
}


/************************************************
  *              LOCAL  FUNCTIONs                                      *
  ************************************************/
static int local_core_add_to_list(PCORE_TASK_CLASS pTaskClass, PCORE_TASK_CTX pTask)
{
    assert(pTask);
    pTask->reg_time = (U32)get_time_ms();    
    
    print_inf("CT_ADD BEFORE: h_%x e_%x ", (U32)pTaskClass->list_head, (U32)pTaskClass->list_end);
    if (pTaskClass->list_head) {
        print_inf("hp_%x hn_%x ", (U32)pTaskClass->list_head->prev,  (U32)pTaskClass->list_head->next);
    }
    if (pTaskClass->list_end) {
        print_inf("ep_%x en_%x ", (U32)pTaskClass->list_end->prev,  (U32)pTaskClass->list_end->next);
    }
    print_inf("new_%x\n", (U32)pTask);    

    pTask->next = NULL;
    if (pTaskClass->list_head == NULL) {
        pTask->prev = NULL;
        pTaskClass->list_head = pTask; 
        pTaskClass->list_end = pTask;
    } else {
        pTask->prev = pTaskClass->list_end;
        pTaskClass->list_end->next = pTask;
        pTaskClass->list_end = pTask;
    }

    print_inf("CT_ADD AFTER: h_%x e_%x ", (U32)pTaskClass->list_head, (U32)pTaskClass->list_end);
    if (pTaskClass->list_head) {
        print_inf("hp_%x hn_%x ", (U32)pTaskClass->list_head->prev,  (U32)pTaskClass->list_head->next);
    }
    if (pTaskClass->list_end) {
        print_inf("ep_%x en_%x ", (U32)pTaskClass->list_end->prev,  (U32)pTaskClass->list_end->next);
    }
    print_inf("%s", "\n");    

    return SUCCESS;
}

static int local_core_rem_from_list(PCORE_TASK_CLASS pTaskClass, PCORE_TASK_CTX pTask)
{
   assert(pTask);

    print_inf("CT_REM BEFORE: h_%x e_%x ", (U32)pTaskClass->list_head, (U32)pTaskClass->list_end);
    if (pTaskClass->list_head) {
        print_inf("hp_%x hn_%x ", (U32)pTaskClass->list_head->prev,  (U32)pTaskClass->list_head->next);
    }
    if (pTaskClass->list_end) {
        print_inf("ep_%x en_%x ", (U32)pTaskClass->list_end->prev,  (U32)pTaskClass->list_end->next);
    }
    print_inf("old_%x\n", (U32)pTask);    
       
    if ((U32)pTask == (U32)pTaskClass->list_head) {
        pTaskClass->list_head = pTask->next;
        if (pTaskClass->list_head) {
            pTaskClass->list_head->prev = NULL;
        }
    } else if ((U32)pTask == (U32)pTaskClass->list_end) {
    
        pTaskClass->list_end = pTask->prev;
        pTaskClass->list_end->next = NULL;
    } else {
        //somewhere at the middle
        pTask->prev = pTask->next;
    }

    print_inf("CT_REM AFTER: h_%x e_%x ", (U32)pTaskClass->list_head, (U32)pTaskClass->list_end);
    if (pTaskClass->list_head) {
        print_inf("hp_%x hn_%x ", (U32)pTaskClass->list_head->prev,  (U32)pTaskClass->list_head->next);
    }
    if (pTaskClass->list_end) {
        print_inf("ep_%x en_%x ", (U32)pTaskClass->list_end->prev,  (U32)pTaskClass->list_end->next);
    }
    print_inf("%s", "\n");    

    return SUCCESS;
}


