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

#ifndef _CORE_H_
#define _CORE_H_

#include "sys_pool.h"

/************************************************
 *              DEFINITIONS                                                *
 ************************************************/
//system tasks priority: priority tasks
#define CORE_TASK_PRIO__UART_RX     50

//system tasks priority: common tasks
#define CORE_TASK_PRIO__ARP         60
#define CORE_TASK_PRIO__NET_RX      50



#define CORE_TASKS_NUM_PRIORITY     10      //must be fast tasks w/o printf etc. according to priority
#define CORE_TASKS_NUM_COMMON       10      //common tasks according to priority
#define CORE_TASKS_NUM_IDLE         1       //it is a single low priority idle task that contains console processing

#define CORE_TASKS_CAPTION_PRIORITY "Priority tasks"
#define CORE_TASKS_CAPTION_COMMON   "Common tasks"
#define CORE_TASKS_CAPTION_IDLE     "Idle tasks"

typedef void CORE_PROC(void *param);

typedef enum {
    CORE_TASK_TYPE_PRIORITY = 0,
    CORE_TASK_TYPE_COMMON,     
    CORE_TASK_TYPE_IDLE    

} CORE_TASK_TYPE;

typedef struct _CORE_TASK_ {
    struct _CORE_TASK_  *prev;
    struct _CORE_TASK_  *next;
    CORE_PROC           *procedure;
    void                *proc_param;    
    U32                 reg_time;
    U32                 period_ms;
    U8                  priority;
    U8                  repeat;     //0 - no repeat, 0xFF - infinite, rest 1..0xFE is number of repeat
    U16                 type;
    
}CORE_TASK_CTX, *PCORE_TASK_CTX;


typedef struct _CORE_TASK_CLASS_ {
    PSYS_POOL_CTX   ctx;         // priority tasks ctx
    PCORE_TASK_CTX  list_head;
    PCORE_TASK_CTX  list_end;  

} CORE_TASK_CLASS, *PCORE_TASK_CLASS;

typedef struct _CORE_CTX_ {
    CORE_TASK_CLASS ptask;      // priority tasks ctx
    CORE_TASK_CLASS ctask;      // common tasks ctx    
    CORE_TASK_CLASS itask;      // idle tasks ctx

} CORE_CTX, *PCORE_CTX;


/************************************************
  *             FUNCTIONS                                                   *
  ************************************************/
int     core_init(void);
int     core_close(void);
void    core_dispatcher_task(void);
void    core_info(void);
int     core_reg_task(CORE_PROC *procedure, void *proc_param, U32 period_ms, CORE_TASK_TYPE type, U8 priority, U8 repeat);
int     core_unreg_task(CORE_PROC *procedure, CORE_TASK_TYPE type);


#endif /*_CORE_H_ */

