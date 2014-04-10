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

static void local_core_proc_pending_tasks(PCORE_TASK_CLASS pTaskClass);
static void local_core_task_add_pending(PCORE_TASK_CLASS pTaskClass, PCORE_TASK_CTX pTask);
static void local_core_task_rem_pending(PCORE_TASK_CLASS pTaskClass, PCORE_TASK_CTX pTask);

static void local_core_task_add(PCORE_TASK_CLASS pTaskClass, PCORE_TASK_CTX pTask);
static void local_core_task_rem(PCORE_TASK_CLASS pTaskClass, PCORE_TASK_CTX pTask);



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
    pCoreCtx->ptask.list_head= NULL;
    pCoreCtx->ptask.list_end = NULL;   
    pCoreCtx->ptask.list_pending_add = NULL;
    pCoreCtx->ptask.list_pending_rem = NULL;   
    
    //allocate pool for common tasks
    pCoreCtx->ctask.ctx = sys_pool_init(CORE_TASKS_NUM_COMMON, sizeof(CORE_TASK_CTX), (U8 *)&(CORE_TASKS_CAPTION_COMMON));
    assert(pCoreCtx->ctask.ctx);
    assert_rc(sys_pool_test(pCoreCtx->ctask.ctx));
    pCoreCtx->ctask.list_head= NULL;
    pCoreCtx->ctask.list_end = NULL;   
    pCoreCtx->ctask.list_pending_add = NULL;
    pCoreCtx->ctask.list_pending_rem = NULL;   
    
    //allocate pool for idle task
    pCoreCtx->itask.ctx = sys_pool_init(CORE_TASKS_NUM_IDLE, sizeof(CORE_TASK_CTX), (U8 *)&(CORE_TASKS_CAPTION_IDLE));
    assert(pCoreCtx->itask.ctx);
    assert_rc(sys_pool_test(pCoreCtx->itask.ctx));
    pCoreCtx->itask.list_head= NULL;
    pCoreCtx->itask.list_end = NULL;   
    pCoreCtx->itask.list_pending_add = NULL;
    pCoreCtx->itask.list_pending_rem = NULL;   

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

void core_dispatcher_check_prio_task(void)
{
    PCORE_TASK_CTX  pCurrTask = pCoreCtx->ptask.list_head;

    // priority task doesn't support period or repeat: 
    // all priority tasks according to its priority are checking every time
    
    //process priority tasks
    while (pCurrTask) {
        //run procedure
        pCurrTask->procedure(pCurrTask->proc_param);
        pCurrTask = pCurrTask->next;
    }

    return;
}

void core_dispatcher(void)
{
    PCORE_TASK_CLASS pTaskClass = &pCoreCtx->ctask;
    PCORE_TASK_CTX  pCurrTask = pTaskClass->list_head;

    core_dispatcher_check_prio_task();

    //process common tasks
    pCurrTask = pTaskClass->list_head;
    while (pCurrTask) {
        U32 curr_time = (U32)get_time_ms();
        U32 time_diff = get_time_diff(pCurrTask->reg_time, curr_time);
		PCORE_TASK_CTX pNextTask = pCurrTask->next;
				
        if (time_diff >= pCurrTask->period_ms) {

            //print_inf("\n------\nPROC_CT: (c_%x cp_%x cn_%x: pr_%d r_%d ms_%d)\n", (U32)pCurrTask, (U32)pCurrTask->prev, (U32)pCurrTask->next,
            //        pCurrTask->priority, pCurrTask->repeat, pCurrTask->period_ms);    

            pCurrTask->procedure(pCurrTask->proc_param);
            
            //run procedure
            if (pCurrTask->repeat == 0) {
				//task is already obsolete
                local_core_task_rem(pTaskClass, pCurrTask);
             } else if (pCurrTask->repeat == 0xFF) {
                //infinite task: update reg time
                pCurrTask->reg_time = curr_time;  
            } else {
                //finite task: update reg time
                pCurrTask->reg_time = curr_time;  
                pCurrTask->repeat--;  
            }
        }
        
        pCurrTask = pNextTask;
        
        //check prio tasks before processing of next common task or idle task
        core_dispatcher_check_prio_task();
    }

    local_core_proc_pending_tasks(&pCoreCtx->ptask);    
    local_core_proc_pending_tasks(pTaskClass);

    pCurrTask = pCoreCtx->itask.list_head;
    if (pCurrTask) {
        //process idle task - it is a single task
        if (pCurrTask->procedure) {
            //run procedure           
            pCurrTask->procedure(pCurrTask->proc_param);
        }
    }
    
    return;
}

int     core_reg_task(CORE_PROC *procedure, void * proc_param, U32 period_ms, CORE_TASK_TYPE type, U8 priority, U8 repeat)
{
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
	//print_inf("--> AllocTaskCtx[%d]: 0x%x\n",  type, (U32)pTask);
	
    if (pTask == NULL) {
        print_err_cmd("%s overflow", pTaskClass->ctx->pool_caption);
        return FAILURE;
    }

	//fill up task
    pTask->procedure = procedure;
    pTask->proc_param = proc_param;
    pTask->period_ms = period_ms;
    pTask->priority = priority;
    pTask->repeat = repeat;    
    pTask->type = type;
    pTask->prev = NULL;
    pTask->next = NULL;
    pTask->next_pending = NULL;

    //print_inf("ADD PENDING to %s: (c_%x cp_%x cn_%x: pr_%d r_%d ms_%d ex_%x exp_%x t_%d)\n", pTaskClass->ctx->pool_caption,
    //                (U32)pTask, (U32)pTask->prev, (U32)pTask->next,
    //                pTask->priority, pTask->repeat, pTask->period_ms,
    //                (U32)pTask->procedure, (U32)pTask->proc_param, pTask->type);    

    if (type != CORE_TASK_TYPE_IDLE) {
        local_core_task_add_pending(pTaskClass, pTask);        
    } else {
        //idle task is single
        local_core_task_add(pTaskClass, pTask);
    }
       
    return SUCCESS;
}

int     core_unreg_task(CORE_PROC *procedure, CORE_TASK_TYPE type)
{
    PCORE_TASK_CLASS pTaskClass;
    PCORE_TASK_CTX pTask;
	U32 task_rem_counter = 0;

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
			//print_inf("REM PENDING from %s: (c_%x cp_%x cn_%x: pr_%d r_%d ms_%d ex_%x exp_%x t_%d)\n", pTaskClass->ctx->pool_caption,
			//				  (U32)pTask, (U32)pTask->prev, (U32)pTask->next,
			//				  pTask->priority, pTask->repeat, pTask->period_ms,
			//				  (U32)pTask->procedure, (U32)pTask->proc_param, pTask->type);	  

			if (type != CORE_TASK_TYPE_IDLE) {
				local_core_task_rem_pending(pTaskClass, pTask);
			} else {
				//idle task is single
				if (task_rem_counter) {
					print_err_cmd("idle task is already killed, unknown task 0x%x", (U32)procedure);
					return FAILURE;
				}
				local_core_task_rem(pTaskClass, pTask);
			}
			task_rem_counter++;				
        }
        pTask = pTask->next;
    }

    if (task_rem_counter == 0) {
        print_err_cmd("has no task 0x%x", (U32)procedure);
        return FAILURE;
    } else if (task_rem_counter >= 2) {
        print_wrn_cmd("killed all %d copies of task 0x%x", task_rem_counter, (U32)procedure);
    }

   
    return SUCCESS;
}


/************************************************
  *              LOCAL  FUNCTIONs                                      *
  ************************************************/
void local_core_proc_pending_tasks(PCORE_TASK_CLASS pTaskClass)
{
    PCORE_TASK_CTX  pCurrTask = pTaskClass->list_pending_rem;
    
    if (pCurrTask) {
        while(pCurrTask) {
            local_core_task_rem(pTaskClass, pCurrTask);
            pCurrTask = pCurrTask->next_pending;
        }
        pTaskClass->list_pending_rem = NULL;
    }

    pCurrTask = pTaskClass->list_pending_add;
    if (pCurrTask) {
        while(pCurrTask) {
            local_core_task_add(pTaskClass, pCurrTask);
            pCurrTask = pCurrTask->next_pending;
        }
        pTaskClass->list_pending_add = NULL;        
    }
    
    return;
}

static void local_core_task_add_pending(PCORE_TASK_CLASS pTaskClass, PCORE_TASK_CTX pTask)
{
    PCORE_TASK_CTX pCurrTask = pTaskClass->list_pending_add;

    assert(pTask);

	print_dbg("0x%x %d", (U32)pTask, pTask->priority);

    pTask->reg_time = (U32)get_time_ms();    
    pTask->next_pending = NULL;

    if (pCurrTask == NULL) {
        pTaskClass->list_pending_add = pTask;
    } else {
        while(pCurrTask->next_pending) {
            pCurrTask = pCurrTask->next_pending;
        }
        pCurrTask->next_pending = pTask;
    }

    return;
}

static void local_core_task_rem_pending(PCORE_TASK_CLASS pTaskClass, PCORE_TASK_CTX pTask)
{
    PCORE_TASK_CTX pCurrTask = pTaskClass->list_pending_rem;

    //WARNING: task to be freed at palre of remmoving from the task list
    
    assert(pTask);

	print_dbg("0x%x %d", (U32)pTask, pTask->priority);

    pTask->next_pending = NULL;
  
    if (pCurrTask == NULL) {
        pTaskClass->list_pending_rem = pTask;
    } else {
        while(pCurrTask->next_pending) {
            pCurrTask = pCurrTask->next_pending;
        }
        pCurrTask->next_pending = pTask;
    }
    
    return;
}

static void local_core_task_add(PCORE_TASK_CLASS pTaskClass, PCORE_TASK_CTX pTask)
{
    U16 priority = pTask->priority;

	//print_dbg("type_%d task_0x%x\n",  pTask->type, (U32)pTask);
	print_dbg("0x%x %d", (U32)pTask, pTask->priority);

#if 0    
    print_inf("CORE ADD BEFORE: (h_%x e_%x) ", (U32)pTaskClass->list_head, (U32)pTaskClass->list_end);
    if (pTaskClass->list_head) {
        print_inf("(hp_%x hn_%x) ", (U32)pTaskClass->list_head->prev,  (U32)pTaskClass->list_head->next);
    }
    if (pTaskClass->list_end) {
        print_inf("(ep_%x en_%x) ", (U32)pTaskClass->list_end->prev,  (U32)pTaskClass->list_end->next);
    }
    print_inf("new_%x prio_%d\n", (U32)pTask, pTask->priority);    
#endif
    
    if (pTaskClass->list_head == NULL) {
        //just add as first item
        pTask->prev = NULL;
        pTask->next = NULL;
        pTaskClass->list_head = pTask; 
        pTaskClass->list_end = pTask;
    } else {
        //some items are already in list
        PCORE_TASK_CTX pCurrTask = pTaskClass->list_head;

        while(pCurrTask && (pCurrTask->priority > priority)){
            pCurrTask = pCurrTask->next;
        }

        if (pCurrTask == NULL) {
            //all existing tasks has higher priority than new one => add to the end
            pTask->next = NULL;
            pTask->prev = pTaskClass->list_end;
            pTaskClass->list_end->next = pTask;
            pTaskClass->list_end = pTask;
        } else if (pCurrTask == pTaskClass->list_head) {
            //add as head
            pTask->next = pTaskClass->list_head;
            pTask->prev = NULL;
            pCurrTask->prev = pTask;
            pTaskClass->list_head = pTask; 
        } else {
            //add somewhere at the middle before CurrTask
            pTask->next = pCurrTask;
            pTask->prev = pCurrTask->prev;
            pCurrTask->prev->next = pTask;
            pCurrTask->prev = pTask;
        }
    }

#if 0
    {
        print_inf("CORE ADD to %s: (h_%x e_%x) ", pTaskClass->ctx->pool_caption, (U32)pTaskClass->list_head, (U32)pTaskClass->list_end);
        if (pTaskClass->list_head) {
            PCORE_TASK_CTX pCurrTask;
            
            print_inf("(hp_%x hn_%x: pr_%d r_%d ms_%d) ", (U32)pTaskClass->list_head->prev,  (U32)pTaskClass->list_head->next, 
                pTaskClass->list_head->priority, pTaskClass->list_head->repeat, pTaskClass->list_head->period_ms);
    
            pCurrTask = pTaskClass->list_head->next;
            
            while(pCurrTask && (pCurrTask != pTaskClass->list_end)){
                print_inf("(c_%x cp_%x cn_%x: pr_%d r_%d ms_%d) ", (U32)pCurrTask, (U32)pCurrTask->prev, (U32)pCurrTask->next,
                    pCurrTask->priority, pCurrTask->repeat, pCurrTask->period_ms);    
                pCurrTask = pCurrTask->next;
            }
        
        }
        
        if (pTaskClass->list_end) {
            print_inf("(ep_%x en_%x: pr_%d r_%d ms_%d) ", (U32)pTaskClass->list_end->prev,  (U32)pTaskClass->list_end->next, 
                pTaskClass->list_end->priority, pTaskClass->list_end->repeat, pTaskClass->list_end->period_ms);
        }
    }
    print_inf("%s", "\n");    
#endif

    return;
}

static void local_core_task_rem(PCORE_TASK_CLASS pTaskClass, PCORE_TASK_CTX pTask)
{
	//print_dbg("type_%d task_0x%x\n",  pTask->type, (U32)pTask);
	print_dbg("0x%x %d", (U32)pTask, pTask->priority);

#if 0
    print_inf("CORE REM BEFORE: h_%x e_%x ", (U32)pTaskClass->list_head, (U32)pTaskClass->list_end);
    if (pTaskClass->list_head) {
        print_inf("hp_%x hn_%x ", (U32)pTaskClass->list_head->prev,  (U32)pTaskClass->list_head->next);
    }
    if (pTaskClass->list_end) {
        print_inf("ep_%x en_%x ", (U32)pTaskClass->list_end->prev,  (U32)pTaskClass->list_end->next);
    }
    print_inf("old_%x\n", (U32)pTask);    
#endif
       
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

#if 0
    {
        print_inf("CORE REM from %s: (h_%x e_%x) ", pTaskClass->ctx->pool_caption, (U32)pTaskClass->list_head, (U32)pTaskClass->list_end);
        if (pTaskClass->list_head) {
            PCORE_TASK_CTX pCurrTask;
            
            print_inf("(hp_%x hn_%x: pr_%d r_%d ms_%d) ", (U32)pTaskClass->list_head->prev,  (U32)pTaskClass->list_head->next, 
                pTaskClass->list_head->priority, pTaskClass->list_head->repeat, pTaskClass->list_head->period_ms);
    
            pCurrTask = pTaskClass->list_head->next;
            
            while(pCurrTask && (pCurrTask != pTaskClass->list_end)){
                print_inf("(c_%x cp_%x cn_%x: pr_%d r_%d ms_%d) ", (U32)pCurrTask, (U32)pCurrTask->prev, (U32)pCurrTask->next,
                    pCurrTask->priority, pCurrTask->repeat, pCurrTask->period_ms);    
                pCurrTask = pCurrTask->next;
            }
        
        }
        
        if (pTaskClass->list_end) {
            print_inf("(ep_%x en_%x: pr_%d r_%d ms_%d) ", (U32)pTaskClass->list_end->prev,  (U32)pTaskClass->list_end->next, 
                pTaskClass->list_end->priority, pTaskClass->list_end->repeat, pTaskClass->list_end->period_ms);
        }
    }
    print_inf("%s", "\n");     
#endif

    //print_dbg("--> FreeTaskCtx[%d]: 0x%x\n",  pTask->type, (U32)pTask);
    assert_rc(sys_pool_free(pTaskClass->ctx, pTask));
   
    return;
}


