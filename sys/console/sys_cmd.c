/**
 * Universal console manager: commands storage
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


/************************************************
*              GLOBAL DEFINITIONS                                    *
************************************************/
int cmd_exit(int paramc, PCMGR_PARAM params, void *cmd_param);
int cmd_help(int paramc, PCMGR_PARAM params, void *cmd_param);


/************************************************
*              GLOBAL FUNCTIONS                                      *
************************************************/
int cmd_init(void)
{
    int rc;

    rc = cmgr_reg_handler("help",           cmd_help,       "Information about console commands.", "[<command>] OR [-d] OR [--details]", NULL);
    rc |= cmgr_reg_handler("exit",          cmd_exit,       "Application exit.", "", NULL);

    return rc;
}



/************************************************
*              LOCAL   FUNCTIONS                                      *
************************************************/

int cmd_exit(int paramc, PCMGR_PARAM params, void *cmd_param)
{
	print_log("%s", "TODO: reboot feature");
    return 0;
}

int cmd_help(int paramc, PCMGR_PARAM params, void *cmd_param)
{
    if (paramc == 1)  {
        if (params[0].type == CMGR_PARAM_TYPE_VAL_STR) {
            cmgr_find_and_print_cmd_info(params[0].value_str);
            return SUCCESS;
        } else if ((params[0].type == CMGR_PARAM_TYPE_OPT) && ((strcmp(params[0].opt_name, "details") == 0) || (strcmp(params[0].opt_name, "d") == 0))) {
            print_inf("-----------------------------\n");
            print_inf("Console version: %d.%d\n", CMGR_VERSION_R, CMGR_VERSION_RC);    
#if 0			
            print_inf("Usage: <command> [<param_value>] [-(-)<param_name>] [-(-)<param_name>( )=( )<param_value>]\n");
            print_inf("   - max length of comman line including all parameters is limited to (%d) bytes\n", CMGR_STRING_MAX_LEN);
            print_inf("   - parameters are optional\n");    
            print_inf("   - total number of supported parameters is (%d)\n", CMGR_PARAM_MAX_COUNT);        
            print_inf("   - style [<param_value>]:\n");        
            print_inf("      - recognizing as <number> or <string> parameter value\n");
            print_inf("      - spaces are separation from other parameters\n");    
            print_inf("      - string parameter value can be places into single or double brakets '<param_value>'\n");            
            print_inf("      - dec number value consists of chars 0..9\n");
            print_inf("      - hex number value has prefix '0x' and consists of chars 0..9, A..F\n");
            print_inf("   - style [--<param_name>]:\n");        
            print_inf("      - recognizing as string argument\n");
            print_inf("      - spaces are separation from other parameters\n");    
            print_inf("      - argument must going after symbols '-' or '--'\n");
            print_inf("      - spaces between symbols '-' or '--' and argument are permitted\n");        
            print_inf("      - argument name must be as single word\n");        
            print_inf("      - max argument length is (%d) bytes\n", CMGR_CMD_MAX_LEN);            
            print_inf("      - first symbol must by alphabetic char\n");
            print_inf("      - supported symbols: 'A..Z', 'a..z', 0..9, '_', '-'\n");    
            print_inf("   - style [--<param_name>=<param_value>]:\n");        
            print_inf("      - recognizing as combination of option name and value\n");
            print_inf("      - option properties same to argument properties\n");
            print_inf("      - spaces between option name, symbol '=' and value are ignoring\n");        
            print_inf("      - value has same properties to <param_value>\n");        
            print_inf("   <command>\n");
            print_inf("      - max command length is (%d) bytes\n", CMGR_CMD_MAX_LEN);    
            print_inf("      - first symbol must by alphabetic char\n");
            print_inf("      - supported symbols: 'A..Z', 'a..z', 0..9, '_', '-'\n");  
#endif            
        } else {
            print_err("%s", "expected only arguments without values");
        }
    } else if (paramc >= 2) {
        print_err("expected (1) or NO arguments instead of (%d) arguments", paramc);
    }
    cmgr_print_registered_cmds();
    
    return SUCCESS;
}     


/************************************************
*              GRAPH DRAWING                                          *
************************************************/
int cmd_draw_graph(U32 arr_size, U32 *Arr, U32 Size, U32 GRAPH_ROWS, U32 GRAPH_COLS, char *GRAPH_TITLE, char *W_TITLE)
{
#if 0
    V32 graph[GRAPH_COLS][GRAPH_ROWS];
    U32 min_t, max_t, i, j, k, m, val, val_us;
    U32 min_us, max_us, avg_us;

    U32 graph_min_row;
    U32 graph_max_row, graph_max_col;
    U32 graph_row_dx, graph_col_dx;

    if (Size == 0)
    {
        print_inf("Err: Size is NULL\n");
        return FAILURE;
    }
    
    if ((U32)Arr == 0)
    {
       print_inf("Err: PTR is NULL\n");
       return FAILURE;
    }

    //initialization local vars
    min_t = 0xFFFFFFFF;
    max_t = 0;
    graph_max_col = 0;
    for (i=0; i< GRAPH_COLS; i++)
            for (j=0; j< GRAPH_ROWS; j++)
                graph[i][j] = 0xFFFFFFFF;
    
    //finding boundaries        
    for (i=0; i< arr_size; i++)
    {
        val = (U32)Arr[i];
        if (val)
        {
            if (val > max_t)
                max_t = val;

            if (val < min_t)
                min_t = val;
        }
    }
    min_us = (U32)TICKS_TO_US(min_t);
    max_us = (U32)TICKS_TO_US(max_t);

    ////pack by cols
    if (Size <= GRAPH_COLS)
    {
        graph_col_dx = 1000;
        graph_max_col = Size; 
    }
    else
    {
        graph_max_col = GRAPH_COLS;
        graph_col_dx = (Size*1000)/GRAPH_COLS;
    }
    
    m = graph_col_dx;
    j = 0;
    k=0;
    avg_us = 0;
    
    for (i=0; i< arr_size; i++)
    {
        val = (U32)Arr[i];
        if (val)
        {
            val_us = (U32)TICKS_TO_US(val);        

            avg_us +=val_us;
            j++;

            if ((m - ((i+1)*1000)) <= graph_col_dx)
            {
                if (k == (GRAPH_COLS -1)) 
                    continue;

                m +=graph_col_dx;

                graph[k][0] = avg_us/j;

                //printf("<ADD> --> graph[%d][0]=%d, avg_tmp=%d, j=%d, i=%d <> graph_min_col=%d, i=%d\n", k, graph[k][0], avg_us, j, i*1000, m, i);                    
                
                avg_us = 0;
                j=0;

                if (k < (GRAPH_COLS -1))
                    k++;
                
                if (k == graph_max_col)
                    break;
            }
        }
        if (i == (arr_size-1))
        {
            graph[k][0] = avg_us/j;
            //printf("<ADDL> --> graph[%d][0]=%d, avg_tmp=%d, j=%d, i=%d <> graph_min_col=%d, i=%d\n", k, graph[k][0], avg_us, j, i*1000, m, i);                    
        }
    }

    //pack by rows
    graph_min_row = min_us * 1000;
    graph_max_row = max_us * 1000;
    graph_row_dx = (graph_max_row - graph_min_row)/GRAPH_ROWS;
    i = 0;
    
    for (j=0; j < graph_max_col; j++)
    {
        val_us = graph[j][0];

        if (graph_row_dx && val_us)
        {
            i = (val_us*1000) / graph_row_dx;
            if (i>GRAPH_ROWS)
                i = GRAPH_ROWS;

            if (i)
                i--; //array 0..24 == 25 items
        }
        else
            i = 0;

        //printf("<j=%d> --> graph[%d][%d]=%d\n", j, j, i, val_us);                    

        if (i)
        {
            graph[j][0] = 0xFFFFFFFF;
            graph[j][i] = val_us;
        }

    }   

    //draw table
    print_inf("GRAPH scaling to [%d rows x %d cols] from last [%d entry]\n", GRAPH_ROWS, GRAPH_COLS, Size);
    print_inf("Title: %s\n", GRAPH_TITLE);    

    //first line
    for (i=0; i <= (graph_max_col+5); i++)
    {
        if ((i==4) || (i==(graph_max_col+5)))
            print_inf("|");    
        else
            print_inf("-");    
    }
    print_inf(" \n");    

    //print row

    k = GRAPH_ROWS -1;   
    for (i=0; i < GRAPH_ROWS; i++)
    {
        //print col
        for (j=0; j <= (graph_max_col+5); j++)
        {
            if (j<=2)       
                __NOP();
            else if (j==3)                        
                print_inf("%4d", graph_max_row/1000);    
            else if ((j==4) || (j==(graph_max_col+5)))
                print_inf("|");    
            else
            {
                val_us = graph[j-5][k];
                    
                if (val_us == 0xFFFFFFFF)
                    print_inf(" ");    
                else
                    print_inf("*"); 
            }
           
        }
        print_inf(" \n");    

        if (k)
            k--;
        graph_max_row = (graph_max_row - graph_row_dx);
    }       
    
    //end line
    for (i=0; i <= (graph_max_col+5); i++)
    {
        if (i==4)
            print_inf("^");    
        else if (i==(graph_max_col+5))
            print_inf("|");    
        else
            print_inf("-");    
    }
    print_inf(" \n");    
    print_inf("usec| %s\n", W_TITLE);    
    print_inf(" \n");    
#endif
    return 0;
}

void cmd_draw_graph_entry(U32 arr_size, U32 *arr, U32 adjcols, U32 adjrows, char *GRAPH_TITLE, char *W_TITLE)
{
#if 0
    U32 min_t, max_t, avg_t, i, val, counter;
    U32 min_us, max_us, avg_us;

    min_t = 0xFFFFFFFF;
    max_t = 0;
    avg_t = 0;
    counter = 0;
            
    for (i=0; i< arr_size; i++)
    {
        val = (U32)arr[i];

        if (val)
        {
            if (val > max_t)
                max_t = val;

            if (val < min_t)
                min_t = val;

            avg_t += val;
            counter++;
        }
    }
    
    if (counter == 0)
    {
        print_inf("\r\n");
        print_inf("No information to draw\n");
        print_inf("------------\n\n");        
        return;
    }
    
    avg_t = avg_t/counter;

    min_us = (U32)TICKS_TO_US(min_t);
    avg_us = (U32)TICKS_TO_US(avg_t);
    max_us = (U32)TICKS_TO_US(max_t);

    cmd_draw_graph(arr_size, &arr[0], counter, adjrows, adjcols, GRAPH_TITLE, W_TITLE);
#endif
}



