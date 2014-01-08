/*************************************************
 *                   X-BOOT entry                *
 *************************************************/
#include "global.h"

void  _start(void)
{
    unsigned int i, a;


    for(i=0; i<10; i++)
    {
        a = i;
        printf("i=%d\n", i);
    }
    
    //add dummy code
    assert(1);
    if (1)
    {
        print_inf("%s", "dummy code");
        print_log("%s", "dummy code");
        print_err("%s", "dummy code");
        assert(0);
    }
    
    return;
}

