/**
 * Universal console manager
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

#ifndef __SYS_CMGR_H__
#define __SYS_CMGR_H__

#include "types.h"


/************************************************
*              DEFINITIONS                                                 *
************************************************/

#define CMGR_VERSION_R      1
#define CMGR_VERSION_RC     0

#define CMGR_STATE_CMD_HEAD                 0
#define CMGR_STATE_CMD_BODY                 1
#define CMGR_STATE_PARAM_DETECT             2
#define CMGR_STATE_PARAM_OPT_START          3
#define CMGR_STATE_PARAM_OPT_BODY           4
#define CMGR_STATE_PARAM_GET_VALUE          5
#define CMGR_STATE_PARAM_DETECT_VALUE       6

#define CMGR_CMD_CHECK__EMPTY               1
#define CMGR_CMD_CHECK__FORMAT_ERROR        2


/***********************************************
* Example of command: {<command string>}
* where <command string> == {[<cmd_name>] [-<arg>] [-<arg>=<value>] [--<arg>]...  [--<arg>=<value>]}
* - max length of <command string> is 256 bytes
* - max length of <cmd_name> is 20 bytes
* --- command name is a single word starting from alphabetic char
* - every argument is going after symbol "-" or "--"
* - dec number value is recognizing after symbol "=" till first space symbol
* --- dec number consists of number chars 0..9 only
* - hex number value is recognizing after symbol "=" till first space symbol
* --- hex number has prefix "0x"
* - string value is a sequence of non dec & non hex values from symbol "=" till first space
* --- string line with spaces is recognizing as string value inside single brackets " '<string line>' "
* - spaces between symbol "=" and value are ignoring
* - total number of supported arguments is 32
*
* - rest combinations will return error with wrong sequence at the line
************************************************/

typedef enum _CMGR_CMD_PARAM_TYPE_
{
    CMGR_PARAM_TYPE_NONE          = (0),                                                /* nothing set */
    CMGR_PARAM_TYPE_OPT           = (1<<2),                                             /* only string option */
    CMGR_PARAM_TYPE_VAL_NUM       = (1<<0),                                             /* only value:  value_str contains string value, value_num contains converted number value */
    CMGR_PARAM_TYPE_VAL_STR       = (1<<1),                                             /* only value:  value_str contains string value, value_num not available */
    CMGR_PARAM_TYPE_OPT_VAL_NUM   = (CMGR_PARAM_TYPE_VAL_NUM | CMGR_PARAM_TYPE_OPT),    /* string option and value:  value_str contains string value, value_num contains converted number value */
    CMGR_PARAM_TYPE_OPT_VAL_STR   = (CMGR_PARAM_TYPE_VAL_STR | CMGR_PARAM_TYPE_OPT)     /* string option and value:  value_str contains string value, value_num not available */
} CMGR_CMD_PARAM_TYPE;


#define CMGR_STRING_MAX_LEN     128
#define CMGR_CMD_MAX_LEN        20
#define CMGR_PARAM_MAX_COUNT    32

#define CMGR_RX_BUF_SIZE        128
#define CMGR_CMD_HISTORY_LEN    16

#define CMGR_PROC_INPUT_STATE_NONE      0
#define CMGR_PROC_INPUT_STATE_ESC       1
#define CMGR_PROC_INPUT_STATE_ACCEPT    2
#define CMGR_PROC_INPUT_STATE_DIGIT3    3


typedef struct _CMGR_PARAM_
{
    CMGR_CMD_PARAM_TYPE type;         
    char                opt_name[CMGR_CMD_MAX_LEN];      /* pointer to option or argument name */
    unsigned int        value_num;      /* number value */
    char                value_str[CMGR_STRING_MAX_LEN];     /* pointer to string value */
}CMGR_PARAM, *PCMGR_PARAM;

typedef int (*TCMGR_CMD) (int paramc, PCMGR_PARAM params, void *cmd_param);

typedef struct _CMGR_MENU_ELM_
{
    struct _CMGR_MENU_ELM_  *next;          /* pointer to the next menu element in alphabetic order by name             */
    struct _CMGR_MENU_ELM_  *prev;          /* pointer to the previous menu element in alphabetic order  by name      */
    const char              cmd_name[CMGR_CMD_MAX_LEN];     /* pointer to the name of comand                              */
    TCMGR_CMD               cmd_handler;                    /* pointer to function of executing menu command      */
    char                    cmd_info[CMGR_STRING_MAX_LEN];  /* information about command                                  */
    char                    param_info[CMGR_STRING_MAX_LEN];  /* information about parameters of command             */    
    void                    *cmd_param;                     /* pointer to a command parameters                          */
}CMGR_MENU_ELM, *PCMGR_MENU_ELM;

typedef struct _CMGR_HISTORY_CMD_
{
    char            cmd[CMGR_STRING_MAX_LEN];
    unsigned int    cmd_pos;    
    
    char            cmd_name[CMGR_CMD_MAX_LEN];
    unsigned int    paramc;
    CMGR_PARAM      params[CMGR_PARAM_MAX_COUNT];
}CMGR_HCMD, *PCMGR_HCMD;

typedef struct __CMGR_CTX__
{
    PCMGR_MENU_ELM          menu_first; /* first menu element: a..-> z */
    PCMGR_MENU_ELM          menu_last;  /* last menu element */
    char                    *logo;

    unsigned int            rx_put;
    unsigned int            rx_get;    
    char                    rxbuf [CMGR_RX_BUF_SIZE];

    CMGR_HCMD               hcmd[CMGR_CMD_HISTORY_LEN];
    unsigned int            hcmd_index;    

}CMGR_CTX, *PCMGR_CTX;


/************************************************
*              FUNCTIONS                                                   *
************************************************/
int             cmgr_reg_handler(const char *cmd_name, TCMGR_CMD cmd_handler, char *cmd_info, char *param_info, void *cmd_param);
int             cmgr_init (void);
void            cmgr_logo_str(void);
void            cmgr_input(char ch);
PCMGR_MENU_ELM  cmgr_find_cmd(const char *cmd);
void            cmgr_print_registered_cmds(void);
void            cmgr_find_and_print_cmd_info(const char *cmd);
void            cmd_draw_graph_entry(U32 arr_size, U32 *arr, U32 adjcols, U32 adjrows, char *GRAPH_TITLE, char *W_TITLE);

int             cmd_help(int paramc, PCMGR_PARAM params, void *cmd_param);


#endif /* __SYS_CMGR_H__ */

