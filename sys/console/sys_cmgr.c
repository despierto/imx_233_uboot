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

#include "global.h"
#include "sys_cmgr.h"



/************************************************
*              GLOBAL DEFINITIONS                                    *
************************************************/

/******************************************
*
*  Constants of control symbols
*
*******************************************/
#define     CM_CR                           '\r'        /**< Carret return                                          */
#define     CM_LF                           '\n'        /**< Line feed                                              */
#define     CM_COMMAND_BEGINNER             ">"         /**< A symbol of a command beginning                        */
#define     CM_BACKSPACE                    0x8         /**< Code of a backspace symbols                            */
#define     CM_TAB                          '\t'        /**< Code of a tab symbols                                  */
#define     CM_SPACE                        0x20        /**< Code of a space symbols                             */
#define     CM_SPACE_LEX_LIST               " \t"       /**< List of space characters of user input                 */
#define     CM_BREAK_LEX_LIST               ":"         /**< List of characters that are interpreted as lexems      */
#define     CM_CTRL_C                       0x3
#define     CM_CTRL_B                       0x2
#define     CM_CTRL_Z                       26
#define     CM_CTRL_X                       24
#define     CM_DEL                          0x7F        /**< space char */

#define     CM_CTRL_UP                      0x11
#define     CM_CTRL_DOWN                    0x12
#define     CM_CTRL_RIGHT                   0x13
#define     CM_CTRL_LEFT                    0x14


/******************************************
*
*  Constants of control symbols IDs
*
*******************************************/
#define     CM_CC_NONE                      0       /**< not a control byte     */
#define     CM_CC_CTRL_C                    3
#define     CM_CC_CTRL_X                    0x18
#define     CM_CC_CTRL_Z                    0x1A
#define     CM_CC_ESC                       0x1b    /**< begin of  control char */
#define     CM_CC_ACCEPT                    0x5B    /**< accept the control char*/
#define     CM_CC_ACCEPT_EX                 0x4F    /**< accept the control char*/
#define     CM_CC_UP                        0x41    /**< up arrow               */
#define     CM_CC_DOWN                      0x42    /**< down arrow             */
#define     CM_CC_LEFT                      0x44    /**< left arrow             */
#define     CM_CC_RIGHT                     0x43    /**< right arrow            */
#define     CM_CC_HOME                      0x48    /**< home arrow             */
#define     CM_CC_END                       0x4B    /**< end arrow              */
#define     CM_CC_F1                        0x50    /**< F1 key                 */
#define     CM_CC_F2                        0x51    /**< F2 key                 */
#define     CM_CC_F3                        0x52    /**< F3 key                 */
#define     CM_CC_F4                        0x53    /**< F4 key                 */

#define     CM_CC_1                         0x31    /**< 1 key                 */
#define     CM_CC_2                         0x32    /**< 2 key                 */
#define     CM_CC_3                         0x33    /**< 3 key                 */

#define     CM_CC_TILDE                     0x7E    /**< ~ key                 */


/******************************************
*
*  Constants of printing symbols
*
*******************************************/
#define     CM_UC_ALPHABET_START            0x41    /**< start of  uppercase alphabet */
#define     CM_UC_ALPHABET_END              0x5A    /**< end of  uppercase alphabet */
#define     CM_LC_ALPHABET_START            0x61    /**< start of  lowercase alphabet */
#define     CM_LC_ALPHABET_END              0x7A    /**< end of  lowercase alphabet */
#define     CM_DIGIT_START                  0x30    /**< start of digits */
#define     CM_DIGIT_END                    0x39    /**< end of digits */
#define     CM_LC_ALPHABET_END              0x7A    /**< end of  lowercase alphabet */
#define     CM_DASH                         0x2D    
#define     CM_UNDERLINE                    0x5F
#define     CM_SINGLE_QUOTE                 0x27
#define     CM_DOUBLE_QUOTE                 0x22
#define     CM_DIGIT_ZERO                   0x30
#define     CM_HEX                          0x78
#define     CM_EQUALS_SIGN                  0x3D



#define is_alphabetic(c)    (((c >= CM_UC_ALPHABET_START) && (c <= CM_UC_ALPHABET_END)) || ((c >= CM_LC_ALPHABET_START) && (c <= CM_LC_ALPHABET_END)))
#define is_print(c)         ((c >= CM_SPACE) && (c < CM_DEL))

#undef is_digit
#define is_digit(c)         ((c >= CM_DIGIT_START) && (c <= CM_DIGIT_END))

#define is_cmd_head(c)      (is_alphabetic(c))
#define is_cmd(c)           (is_alphabetic(c) || (c == CM_DASH) || (c == CM_UNDERLINE) || is_digit(c))

#define is_opt_head(c)      (is_alphabetic(c))
#define is_opt(c)           (is_alphabetic(c) || (c == CM_DASH) || (c == CM_UNDERLINE) || is_digit(c))

#define is_space(c)         (c == CM_SPACE)
#define is_opt_qualifier(c) (c == CM_DASH)
#define is_equals_sing(c)   (c == CM_EQUALS_SIGN)

#define is_value(c)         (is_alphabetic(c) || (c == CM_DASH) || (c == CM_UNDERLINE) || is_digit(c))

#define is_zero(c)          (c == CM_DIGIT_ZERO)
#define is_hex(c)           (c == CM_HEX)
#define is_end_of_str(c)    (c == CM_CR)





PCMGR_CTX       pCMgrCtx;

unsigned int    cmgr_cmd_name_new_before_curr(const char *curr_cmd, const char *new_cmd);
void            cmgr_char_put (char ch);
char            cmgr_char_get (void);
void            cmgr_proc_input(void);
static unsigned int cmgr_parse_cmd (char * cmd, PCMGR_PARAM params, unsigned int *paramc, char *cmd_name);
static int      cmgr_atoi_if_digit(const char *s, unsigned int *num);

extern int 		cmd_init(void);

/************************************************
*              GLOBAL FUNCTIONS                                      *
************************************************/
int cmgr_init (void)
{
    print_sys("%s", "initialization of console");

    pCMgrCtx = (PCMGR_CTX)malloc(sizeof(CMGR_CTX));
	assert(pCMgrCtx);
    memset (pCMgrCtx, 0, sizeof (CMGR_CTX));  

    pCMgrCtx->logo = CONFIG_SYS_PROMPT;
    pCMgrCtx->menu_first = NULL;
    pCMgrCtx->menu_last = NULL;

    pCMgrCtx->rx_put = 0;
    pCMgrCtx->rx_get = 0;

    pCMgrCtx->hcmd_index = 0;

    if (cmd_init() != SUCCESS)
        return FAILURE;

    return SUCCESS;
}

int cmgr_reg_handler(const char *cmd_name, TCMGR_CMD cmd_handler, char *cmd_info, char *param_info, void *cmd_param)
{
    PCMGR_MENU_ELM menu_el;

    if ((cmd_name == NULL) || (!is_alphabetic(cmd_name[0]))) {
        print_err("cmd_name is null or first symbol non alphabetic: (%s)", cmd_name);
        return FAILURE;
    }

    if (strlen(cmd_name) >= CMGR_CMD_MAX_LEN) {
        print_err("cmd_name length (%d) is out of allowed length in (%d) bytes", strlen(cmd_name), CMGR_CMD_MAX_LEN);
        return FAILURE;
    }

    if (cmd_handler == NULL) {
        print_err("%s", "cmd_handler is null");
        return FAILURE;
    }

    if (cmd_info  == NULL) {
        print_err("%s", "cmd_info is null");
        return FAILURE;
    }
    if (strlen(cmd_info) >= CMGR_STRING_MAX_LEN) {
        print_err("cmd_info length (%d) is out of allowed length in (%d) bytes", strlen(cmd_info), CMGR_STRING_MAX_LEN);
        return FAILURE;
    }

    if (strlen(param_info) >= CMGR_STRING_MAX_LEN) {
        print_err("param_info length (%d) is out of allowed length in (%d) bytes", strlen(param_info), CMGR_STRING_MAX_LEN);
        return FAILURE;
    }

    menu_el = (PCMGR_MENU_ELM)malloc(sizeof(CMGR_MENU_ELM));
    memset((void *)menu_el, 0, sizeof(CMGR_MENU_ELM));
    
    menu_el->cmd_handler = cmd_handler;
    menu_el->cmd_param = cmd_param;        

    memcpy((void *)&menu_el->cmd_name[0], (void *)&cmd_name[0], strnlen(cmd_name, CMGR_CMD_MAX_LEN));
    memcpy((void *)&menu_el->cmd_info[0], (void *)&cmd_info[0], strnlen(cmd_info, CMGR_STRING_MAX_LEN));
    memcpy((void *)&menu_el->param_info[0], (void *)&param_info[0], strnlen(param_info, CMGR_STRING_MAX_LEN));    
    //print_dbg("added: CMD (%s) CMD_INFO (%s) ARG_INFO (%s) Handler (0x%x) Param (0x%x)",
    //            cmd_name, cmd_info, param_info, (unsigned int)cmd_handler, (unsigned int)cmd_param);     

    if (pCMgrCtx->menu_first) {
        PCMGR_MENU_ELM curr_el = pCMgrCtx->menu_first;

        if (!pCMgrCtx->menu_last) {
            print_err("%s", "logic error: last menu element is null when first is present");
            return FAILURE;
        }

           
        while(curr_el) {
            if (cmgr_cmd_name_new_before_curr(curr_el->cmd_name, cmd_name)) {
                //print_dbg("CMD (%s) to be inserted before CMD (%s)", cmd_name, curr_el->cmd_name);
                break;
            }
            
            curr_el = curr_el->next;
        }

        //insert new cmd before curr_el
        if (curr_el == NULL) {
            //insert as last cmd                
            //print_dbg("insert as last cmd (%s)", cmd_name);
            menu_el->next = NULL;
            menu_el->prev =  pCMgrCtx->menu_last;
            pCMgrCtx->menu_last->next = menu_el;
            pCMgrCtx->menu_last = menu_el;
        } else {
            //insert new cmd before cure_el
            if (curr_el->prev == NULL) {
                //insert as first menu element
                //print_dbg("insert as first menu element cmd (%s)", cmd_name);
                menu_el->next = curr_el;
                menu_el->prev = NULL;
                curr_el->prev = menu_el;            
                pCMgrCtx->menu_first = menu_el;    
            } else {
                //print_dbg("insert in the middle cmd (%s)", cmd_name);
                menu_el->next = curr_el;
                menu_el->prev = curr_el->prev;
                curr_el->prev->next = menu_el;
                curr_el->prev = menu_el;            
            }
        }
    } else {
        //print_dbg("insert as single first cmd (%s)", cmd_name);
        menu_el->next = NULL;
        menu_el->prev = NULL;        
        pCMgrCtx->menu_first = menu_el;
        pCMgrCtx->menu_last = menu_el;
    }

    return SUCCESS;
}

void cmgr_logo_str(void)
{
    print_inf("%s", pCMgrCtx->logo);
    return;
}

void cmgr_input(char ch)
{
    if (ch == CM_LF) {
        print_inf("%c", ch);
        ch = CM_CR;
    }

    cmgr_char_put(ch);
    cmgr_proc_input();

    return;
}

PCMGR_MENU_ELM cmgr_find_cmd(const char *cmd)
{
    PCMGR_MENU_ELM menu_el = pCMgrCtx->menu_first;

     while(menu_el) {
         if (strcmp(cmd, menu_el->cmd_name) == 0) {

             return menu_el;
         }
         menu_el = menu_el->next;
     }

    return NULL;     
}

void cmgr_print_registered_cmds(void)
{
    PCMGR_MENU_ELM menu_el = pCMgrCtx->menu_first;
    
    print_inf("-----------------------------\n");
    print_inf("Available commands:\n");
    print_inf("-----------------------------\n");

    if (menu_el) {
        while(menu_el) {
            print_inf("%s %s\n", menu_el->cmd_name, menu_el->param_info);            
            print_inf("   %s\n", menu_el->cmd_info);            

            menu_el = menu_el->next;
        }
    } else {
        print_inf("No available commands\n");     
    }

    print_inf("-----------------------------\n");
    print_inf(" \n"); 
    
    return;
}

void cmgr_find_and_print_cmd_info(const char *cmd)
{
    PCMGR_MENU_ELM menu_el =  cmgr_find_cmd(cmd);

    if (menu_el) {
        print_inf("%s %s\n", menu_el->cmd_name, menu_el->param_info);            
        print_inf("   {%s}\n", menu_el->cmd_info);            
    } else {
        print_inf("Command (%s) not registered\n", cmd);            
    }
}

/************************************************
*              LOCAL FUNCTIONS                                         *
************************************************/
unsigned int cmgr_cmd_name_new_before_curr(const char *curr_cmd, const char *new_cmd)
{
    unsigned int i, curr_len, new_len, less_len;

    curr_len = (unsigned int)strlen(curr_cmd);
    new_len = (unsigned int)strlen(new_cmd);

    less_len = (curr_len < new_len) ? curr_len : new_len;
    
    for (i=0; i<less_len; i++)
    {
        //print_dbg("diff n(%d) vs c(%d)", new_cmd[i], curr_cmd[i]);
        if (new_cmd[i] < curr_cmd[i])
        {
            //new cmd is before current cmd in alphabetic order

            //print_dbg("%s", "new cmd is before current cmd in alphabetic order");
            return TRUE;
        }
        else if (new_cmd[i] > curr_cmd[i])
        {
            //new cmd is after current cmd in alphabetic order
            //print_dbg("%s", "new cmd is after current cmd in alphabetic order");
            return FALSE;
        }
    }

    //here both cmd are equal inside less_len, to the first to be the less
    if (new_len < curr_len)
    {
        //print_dbg("%s", "new cmd less then curr cmd");
        return TRUE;
    }

    return FALSE;
}

void cmgr_char_put (char ch)
{
    unsigned int next_put = pCMgrCtx->rx_put + 1;

    if (next_put >= CMGR_RX_BUF_SIZE)
        next_put = 0;

    if (next_put == pCMgrCtx->rx_get) {
        print_err("%s", "warning: console rx buffer overflow");
        return;
    }
    
    pCMgrCtx->rxbuf[pCMgrCtx->rx_put] = ch;
    pCMgrCtx->rx_put = next_put;
    
    return;
}

char cmgr_char_get (void)
{
    char ch;
    unsigned int get = pCMgrCtx->rx_get;

    if (get != pCMgrCtx->rx_put)
    {
        ch = pCMgrCtx->rxbuf[get];
        if (++get >= CMGR_RX_BUF_SIZE)
            pCMgrCtx->rx_get = 0;
        else
            pCMgrCtx->rx_get = get;

        return ch;
    }

    return 0;
}

static int cmgr_process_char (char ch)
{
    PCMGR_HCMD      hcmd = &pCMgrCtx->hcmd[pCMgrCtx->hcmd_index];

    //print_inf("---> PrCh %d <---\n", ch);

    switch (ch)
    {
        // Run command
        case CM_CR:
            {
                PCMGR_PARAM     params = &hcmd->params[0];
                
                //CMgrCmdParamCtx cp;
                U32 nRes;

                print_inf("%c", ch);

                //AddHistory(pCtx->Cd);

                //  Breaking a user command to a list of parameters

                //print_inf("---> CMD: (%s) pos (%d)\n", hcmd->cmd, hcmd->cmd_pos);
                
                nRes = cmgr_parse_cmd (&hcmd->cmd[0], &params[0], &hcmd->paramc, &hcmd->cmd_name[0]);

                switch (nRes)
                {
                    // If command processing is OK
                    case SUCCESS:
                        {
                            unsigned int i;
                            PCMGR_MENU_ELM menu_el;
                            char *opt_name = hcmd->cmd_name;
                            unsigned int correspondence_found = 0;
                            
                            //print_inf("CMD: name (%s) argc (%d):\n", hcmd->cmd_name, hcmd->paramc);
                            //for (i=0; i<hcmd->paramc; i++)
                            //{
                            //    print_inf(" (%2d): t (%d) arg (%s) val (%d)==(%s)\n", i, params[i].type, params[i].opt_name, params[i].value_num, params[i].value_str);
                            //}

                            menu_el = pCMgrCtx->menu_first;
                            while(menu_el) {
                                if (strcmp(opt_name, menu_el->cmd_name) == 0) {
                                    correspondence_found = 1;
                                    break;
                                }
                                menu_el = menu_el->next;
                            }
                        
                            if (correspondence_found) {
                                if (menu_el->cmd_handler){
                                    int rc = menu_el->cmd_handler(hcmd->paramc, params, menu_el->cmd_param);
                                    if (rc != SUCCESS){
                                        print_err("command (%s) completed with error:", opt_name);            
                                        print_inf("--- CMD details:: name (%s) argc (%d):\n", hcmd->cmd_name, hcmd->paramc);
                                        for (i=0; i<hcmd->paramc; i++)
                                        {
                                            print_inf(" |--- Param[%02d]: type (%d) option (%s) value (%d)==(%s)\n", 
                                                i, params[i].type, params[i].opt_name, params[i].value_num, params[i].value_str);
                                        }
                                    }
                                } else {
                                    print_err("handler for command (%s) is NULL", opt_name);            
                                }
                            } else {
                                print_inf("Warning: command (%s) not present at system\n", opt_name);            
                            }

                            break;
                        }
                    case CMGR_CMD_CHECK__EMPTY:
                        {
                            break;
                        }
                    default:
                        {
                            print_err("%s", "parsing error");
                            if (hcmd->cmd_name)
                                cmgr_find_and_print_cmd_info(hcmd->cmd_name);
                            break;
                        }
                }

                //reset cmd string
                hcmd->cmd_pos = 0;
                hcmd->cmd [0] = 0x00;

                cmgr_logo_str();

                break;
            }

        case CM_BACKSPACE:
            {
                unsigned int i;

                // If it's a begin of string
                if (hcmd->cmd_pos == 0)
                    break;

#if 0 //killing all line before BS
                hcmd->cmd [--hcmd->cmd_pos] = 0x0;
                //print_inf("%c%c%c",CM_BACKSPACE, CM_SPACE,CM_BACKSPACE);
                print_inf("%c%c[K",CM_BACKSPACE, CM_CC_ESC);
#else
                //go 1 pos back and clear line after pos till end
                print_inf("%c%c[K", CM_BACKSPACE, CM_CC_ESC);
                i = hcmd->cmd_pos - 1;
                while (hcmd->cmd [i+1] != 0x0) {
                    hcmd->cmd[i] = hcmd->cmd [i+1];
                    print_inf("%c", hcmd->cmd [i++]);                    
                }
                hcmd->cmd[i] = 0;
                while (i-- >= hcmd->cmd_pos)
                    print_inf("%c%c%c",CM_CC_ESC, CM_CC_ACCEPT, CM_CC_LEFT); 
                hcmd->cmd_pos--;
#endif
                break;
            }

        // list of non-out symbols
        case CM_TAB:
        case CM_LF:
                print_inf("%c", ch);
                break;

        case CM_CTRL_UP:
            print_inf("%c%c%c",CM_CC_ESC, CM_CC_ACCEPT, CM_CC_UP);
            break;
            
        case CM_CTRL_DOWN:
            print_inf("%c%c%c",CM_CC_ESC, CM_CC_ACCEPT, CM_CC_DOWN);
            break;
                
        case CM_CTRL_RIGHT:
            if ((hcmd->cmd [hcmd->cmd_pos] == 0x0) || (hcmd->cmd_pos >= CMGR_STRING_MAX_LEN))
                break;
            
            hcmd->cmd_pos++;
            print_inf("%c%c%c",CM_CC_ESC, CM_CC_ACCEPT, CM_CC_RIGHT);
            
            break;
            
        case CM_CTRL_LEFT:
            if (hcmd->cmd_pos == 0)
                break;
            hcmd->cmd_pos--;
            
            print_inf("%c%c%c",CM_CC_ESC, CM_CC_ACCEPT, CM_CC_LEFT);
            break;

        case CM_DEL:
            {
                unsigned int i;
                // If it's an end of string
                if (hcmd->cmd [hcmd->cmd_pos] == 0x0)
                    break;

                //go 1 pos back
                print_inf("%c[K", CM_CC_ESC);
                i = hcmd->cmd_pos + 1;
                while (hcmd->cmd [i] != 0x0) {
                    hcmd->cmd[i-1] = hcmd->cmd [i];
                    print_inf("%c", hcmd->cmd [i++]);                    
                }
                hcmd->cmd[--i] = 0;
                while (i-- > hcmd->cmd_pos)
                    print_inf("%c%c%c",CM_CC_ESC, CM_CC_ACCEPT, CM_CC_LEFT);    
                
                break;
            }

        default:
            {
                if (hcmd->cmd_pos >= (CMGR_STRING_MAX_LEN - 1)) {
                    print_err("%s", "the command is out of buffer");
                    return FAILURE;
                }

                if (is_print(ch))
                {
                    if ((hcmd->cmd [hcmd->cmd_pos] == 0x0) || (hcmd->cmd_pos == 0)) {
                        //paste simbol into the end
                        print_inf("%c", ch);
                        hcmd->cmd [hcmd->cmd_pos++] = ch;
                        hcmd->cmd [hcmd->cmd_pos] = 0x0;
                    } else {
                        unsigned int i = hcmd->cmd_pos;
                        //paste symbol into the middle

                        //go to the end
                        while (hcmd->cmd [i++] != 0x0);
                        
                        //set end of cmd
                        hcmd->cmd [i+1] = 0;
                        
                        //move step by step symbols forward
                        while (i >= hcmd->cmd_pos) {
                            hcmd->cmd [i] = hcmd->cmd [i - 1];
                            i--;
                        }
                        hcmd->cmd [hcmd->cmd_pos] = ch;

                        //clean line
                        print_inf("%c[K", CM_CC_ESC);
                        
                        i = hcmd->cmd_pos;// + 1;
                        while (hcmd->cmd [i] != 0x0) {
                            print_inf("%c", hcmd->cmd [i++]);                    
                        }

                        while (--i > hcmd->cmd_pos)
                            print_inf("%c%c%c",CM_CC_ESC, CM_CC_ACCEPT, CM_CC_LEFT);    
                                   
                        hcmd->cmd_pos++;
                    }
                }

                break;
            }
    }

    return SUCCESS;
}

void cmgr_proc_input(void)
{
    char ch = 0;
    static unsigned int cmgr_proc_input_state = CMGR_PROC_INPUT_STATE_NONE;

    while ((ch = cmgr_char_get()) != 0)
    {
        //printf("[%d]", ch);
    
        switch (cmgr_proc_input_state)
        {
            case CMGR_PROC_INPUT_STATE_ESC:
                {
                    if (ch == CM_CC_ACCEPT) {
                        //printf("<acpt>");
                        cmgr_proc_input_state = CMGR_PROC_INPUT_STATE_ACCEPT;
                        continue;
                    }
                    
                    cmgr_proc_input_state = CMGR_PROC_INPUT_STATE_NONE;
                }
                break;

            case CMGR_PROC_INPUT_STATE_ACCEPT:
                {
                    if (ch == CM_CC_3) {
                        //printf("<d3>");
                        cmgr_proc_input_state = CMGR_PROC_INPUT_STATE_DIGIT3;
                        continue;
                    } else if (ch == CM_CC_UP) {
                        ch = CM_CTRL_UP;
                    } else if (ch == CM_CC_DOWN) {
                        ch = CM_CTRL_DOWN;
                    } else if (ch == CM_CC_RIGHT) {
                        ch = CM_CTRL_RIGHT;
                    } else if (ch == CM_CC_LEFT) {
                        ch = CM_CTRL_LEFT;
                    }
                    
                    cmgr_proc_input_state = CMGR_PROC_INPUT_STATE_NONE;
                }
                break;

            case CMGR_PROC_INPUT_STATE_DIGIT3:
                {
                    if (ch == CM_CC_TILDE) {
                        ch = CM_DEL;
                        //printf("<del>");
                    }

                    cmgr_proc_input_state = CMGR_PROC_INPUT_STATE_NONE;
                }
                break;

            case CMGR_PROC_INPUT_STATE_NONE:
            default:
                {
                    if (ch == CM_CC_ESC) {
                        //printf("<e>");
                        cmgr_proc_input_state = CMGR_PROC_INPUT_STATE_ESC;
                        continue;
                    } else if (ch == CM_DEL) {
                        ch = CM_BACKSPACE;
                    }
                }
                break;
        }

        if (cmgr_process_char(ch) != SUCCESS)
            return;
    }

    return;
}

static unsigned int cmgr_parse_cmd (char * cmd, PCMGR_PARAM params, unsigned int *paramc, char *cmd_name)
{
    unsigned int cmd_pos = 0;
    unsigned int buf_pos = 0;
    char ch;
    unsigned int state = CMGR_STATE_CMD_HEAD;
    PCMGR_PARAM  curr_param = NULL;
    unsigned int param_count = 0;

    cmd_name[0] = 0;
    *paramc = 0;

    //check the command line from begin to the end
    while((ch = cmd[cmd_pos++]) != 0)
    {
        //print_inf("<state=%d buf_pos=%d pos=%d ch=%c>\n", state, buf_pos, cmd_pos, ch);
        switch (state)
        {
            case CMGR_STATE_CMD_HEAD:
                {
                    if (is_cmd_head(ch)) {
                        cmd_name[buf_pos++] = ch;
                        cmd_name[buf_pos] = 0;                        
                        state = CMGR_STATE_CMD_BODY; 
                    } else {
                        print_err("unsupported symbol ( %c ) at begin of command name", ch);
                        return CMGR_CMD_CHECK__FORMAT_ERROR;
                    }
                }
                break;

            case CMGR_STATE_CMD_BODY:
                {
                    if (is_cmd(ch)) {
                        if (buf_pos >= CMGR_CMD_MAX_LEN) {
                            print_err("command name is out of ranges in (%d) bytes", CMGR_CMD_MAX_LEN);
                            return CMGR_CMD_CHECK__FORMAT_ERROR;
                        }
                        cmd_name[buf_pos++] = ch;
                        cmd_name[buf_pos] = 0;                        
                    } else if (is_space(ch)) {
                        //print_inf("Detected command (%s)\n", cmd_name);
                        state = CMGR_STATE_PARAM_DETECT; 
                    } else {
                        print_err("unsupported symbol ( %c ) between command and first parameter", ch);
                        return CMGR_CMD_CHECK__FORMAT_ERROR;
                    }
                }
                break;

            case CMGR_STATE_PARAM_DETECT:
                {
                    if (is_space(ch)) {
                        continue;
                    } else if (is_opt_qualifier(ch)) {  
                        state = CMGR_STATE_PARAM_OPT_START; 
                    } else if (is_value(ch)) {
                        //here param_count == 0
                        buf_pos = 0;
                        curr_param = &params[param_count++];
                        curr_param->opt_name[0] = 0;
                        curr_param->value_str[buf_pos++] = ch;
                        curr_param->value_str[buf_pos] = 0;
                        curr_param->type = CMGR_PARAM_TYPE_NONE;
                        state = CMGR_STATE_PARAM_GET_VALUE; 
                    } else {
                        print_err("unsupported symbol ( %c ) at parameter #%d", ch, param_count);
                        return CMGR_CMD_CHECK__FORMAT_ERROR;
                    }
                }
                break;
            
            case CMGR_STATE_PARAM_GET_VALUE:
                {
                    if (is_value(ch)) {  
                        curr_param->value_str[buf_pos++] = ch;
                        curr_param->value_str[buf_pos] = 0;
                    } else if (is_space(ch) || is_end_of_str(ch)) {
                        curr_param->type = (cmgr_atoi_if_digit(&curr_param->value_str[0], &curr_param->value_num))
                                ? CMGR_PARAM_TYPE_VAL_NUM : CMGR_PARAM_TYPE_VAL_STR;
                                                
                        //print_inf("Detected: opt (%s) type (%d) value_str (%s) value_num (%d)\n", 
                        //    curr_param->opt_name,  curr_param->type, curr_param->value_str,  curr_param->value_num);
                                                
                        state = CMGR_STATE_PARAM_DETECT; 
                    } else {
                        print_err("unsupported symbol ( %c ) at parameter #%d", ch, param_count);
                        return CMGR_CMD_CHECK__FORMAT_ERROR;
                    }
                }
                break;

            case CMGR_STATE_PARAM_OPT_START:
                {
                    if (is_opt_qualifier(ch)) {  
                        ch = cmd[cmd_pos++];
                        if (ch == 0) {  
                            print_err("option not completred at parameter #%d", param_count);
                            return CMGR_CMD_CHECK__FORMAT_ERROR;
                        } else if (is_opt_qualifier(ch)) {  
                            print_err("the 3rd dash symbol ( %c ) is unsupported at parameter #%d", ch, param_count);
                            return CMGR_CMD_CHECK__FORMAT_ERROR;
                        }
                    }

                    if (is_opt_head(ch)) {  
                        buf_pos = 0;
                        curr_param = &params[param_count++];
                        curr_param->opt_name[buf_pos++] = ch;
                        curr_param->opt_name[buf_pos] = 0;
                        curr_param->type = CMGR_PARAM_TYPE_OPT;                        
                        state = CMGR_STATE_PARAM_OPT_BODY; 
                    } else {
                        print_err("unsupported symbol ( %c ) at begin of option name at parameter #%d", ch, param_count);
                        return CMGR_CMD_CHECK__FORMAT_ERROR;
                    }
                }
                break;

            case CMGR_STATE_PARAM_OPT_BODY:
                {
                    if (is_opt(ch)) {
                        if (buf_pos >= CMGR_CMD_MAX_LEN) {
                            print_err("option name is out of ranges in (%d) bytes at parameter #%d", CMGR_CMD_MAX_LEN, param_count);
                            return CMGR_CMD_CHECK__FORMAT_ERROR;
                        }
                        curr_param->opt_name[buf_pos++] = ch;
                        curr_param->opt_name[buf_pos] = 0;
                    } else if (is_space(ch)) {
                        unsigned int j = cmd_pos;
                        
                        while((ch = cmd[j++]) == CM_SPACE);
#if 1 //between option and value any space or symbol '='                        
                        if (is_opt_qualifier(ch)) {
                            cmd_pos = j - 1;
                            //print_inf("Detected option (%s), no value\n", curr_param->opt_name);
                            state = CMGR_STATE_PARAM_DETECT; 
                        } else if (is_equals_sing(ch)) {
                            cmd_pos = j;
                            //print_inf("Detected option (%s), check value\n", curr_param->opt_name);
                            state = CMGR_STATE_PARAM_DETECT_VALUE; 
                        } else {
                            cmd_pos = j - 1;
                            //print_inf("Detected option (%s), no value\n", curr_param->opt_name);
                            state = CMGR_STATE_PARAM_DETECT_VALUE;
                        }
#else //between option and value must be symbol '='                       
                        if (is_equals_sing(ch)) {
                            cmd_pos = j;
                            //print_inf("Detected option (%s), check value\n", curr_param->opt_name);
                            state = CMGR_STATE_PARAM_DETECT_VALUE; 
                        } else {
                            cmd_pos = j - 1;
                            //print_inf("Detected option (%s), no value\n", curr_param->opt_name);
                            state = CMGR_STATE_PARAM_DETECT; 
                        }
#endif                        
                    } else if (is_equals_sing(ch)) {
                        //print_inf("Detected option (%s), check value\n", curr_param->opt_name);
                        state = CMGR_STATE_PARAM_DETECT_VALUE; 
                    } else {
                        print_err("unsupported symbol ( %c ) at option name of parameter #%d", ch, param_count);
                        return CMGR_CMD_CHECK__FORMAT_ERROR;
                    }

                }
                break;

            case CMGR_STATE_PARAM_DETECT_VALUE:
                {
                    if (is_space(ch)) {
                        continue;
                    } else if (is_value(ch)) {
                        buf_pos = 0;
                        curr_param->value_str[buf_pos++] = ch;
                        curr_param->value_str[buf_pos] = 0;
                        state = CMGR_STATE_PARAM_GET_VALUE; 
                    } else {
                        print_err("unsupported symbol ( %c ) at parameter #%d", ch, param_count);
                        return CMGR_CMD_CHECK__FORMAT_ERROR;
                    }
                }
                break;
                                
            default:
                break;
        }

    }

    if (cmd_name[0] == 0) {
        //print_inf("Empty command\n");
        return CMGR_CMD_CHECK__EMPTY;
    }

    if (state == CMGR_STATE_PARAM_GET_VALUE){
        
        curr_param->type |= (cmgr_atoi_if_digit(&curr_param->value_str[0], &curr_param->value_num))
                ? CMGR_PARAM_TYPE_VAL_NUM : CMGR_PARAM_TYPE_VAL_STR;
                                
        //print_inf("end: Detected: opt (%s) type (%d) value_str (%s) value_num (%d)\n", 
        //    curr_param->opt_name,  curr_param->type, curr_param->value_str,  curr_param->value_num);
                                
        state = CMGR_STATE_PARAM_DETECT; 
    }
    
    *paramc = param_count;    
    //print_inf("Exit: cmd_name (%s) paramc (%d)\n", cmd_name, param_count);

    return SUCCESS;
}

static int cmgr_atoi_if_digit(const char *s, unsigned int *num)
{
    int all_digits = 1;
    unsigned int rez = 0;
    char ch;

    if (is_zero(*(s))) {
        if (is_hex(*(s+1))) {           
            *num = strtoul(s, NULL, 16);
            return 1;
        }
    }

    while((ch = *(s++)) != 0) {
        if (is_digit(ch)) {
            rez = rez*10 + ch - '0';
        } else {
            all_digits = 0;
            break;
        }
    }

    if (all_digits)
        *num = rez;
   
    return all_digits;
}




