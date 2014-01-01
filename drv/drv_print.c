/*************************************************
 *               X-BOOT print Driver             *
 *************************************************/
#include <stdio.h>
#include "drv_print.h"

int  drv_print_init(void)
{
    unsigned int i, a;

    for(i=0; i<10; i++)
    {
        a = i;
        printf("i=%d\n", i);
    }

    return 0;
}


