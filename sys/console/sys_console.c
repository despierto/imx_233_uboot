/*************************************************
 *                   X-BOOT console              *
 *************************************************/
#include <stdio.h>
#include "sys_console.h"

int  sys_console_init(void)
{
    unsigned int i, a;

    for(i=0; i<10; i++)
    {
        a = i;
        printf("i=%d\n", i);
    }

    return 0;
}


