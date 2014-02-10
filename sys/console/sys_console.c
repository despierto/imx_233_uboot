/*************************************************
 *                   X-BOOT console              *
 *************************************************/
//nclude "global.h"
#include "sys_console.h"

int  sys_console_init(void)
{
    unsigned int i, a;

    for(i=0; i<10; i++)
    {
        a = i;
        //intf("i=%d\n", i);
    }

    return 0;
}


