/*************************************************
 *                   X-BOOT entry                *
 *************************************************/
#include "global.h"

void  main(void)
{
    unsigned int i, a;


    for(i=0; i<10; i++)
    {
        a = i;
        printf("i=%d\n", i);
    }
    
    return;
}

