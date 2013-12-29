/*************************************************
 *                   X-BOOT entry                *
 *************************************************/
#include <stdio.h>

void  main(voir)
{
    unsigned int i, a;

    for(i=0; i<10; i++)
    {
        a = i;
        printf("i=%d\n", i);
    }
    
    testit1();

    return;
}

