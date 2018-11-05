/**
 *  use threadaffinity to test hash table concurrency 
 */
#include <stdio.h>

#include "threadaffinity.h"

int main()
{
    uint8_t cores[] = {0,1};
    set_thread_affinity(cores, sizeof(cores)/sizeof(cores[0]));
    printf("main end %s:%d\n", __FILE__, __LINE__);
    
    return 0;
}
