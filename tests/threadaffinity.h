#pragma once

#include <stdint.h>

/* set cores array affinity 
 * return 0 for success
*/
int set_thread_affinity(uint8_t * cores, size_t size);
