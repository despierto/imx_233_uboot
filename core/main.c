/*************************************************
 *                   X-BOOT entry                *
 *************************************************/
#include <stdio.h>
#include "sys_console.h"
#include "drv_print.h"

void  main(void)
{
    unsigned int i, a;

    sys_console_init();
    drv_print_init();

    for(i=0; i<10; i++)
    {
        a = i;
        printf("i=%d\n", i);
    }
    
    return;
}

