
#ifndef WIN32
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include "threadaffinity.h"

#ifdef WIN32

#include <windows.h>

int set_thread_affinity(uint8_t * cores, size_t size)
{
    HANDLE thread=0;
    DWORD_PTR mask=0;
    size_t i;
    DWORD_PTR rc;

    thread = GetCurrentThread();
    for (i = 0; i < size; i += 1)
    {
        mask |= (1 << cores[i]);
    }

    /* return the previous affinity mask */
    rc = SetThreadAffinityMask(thread, mask);
    fprintf(stderr, "[+] set mask=%X previous=%x %s %s:%d\n", mask, rc, __func__, __FILE__, __LINE__); fflush(stderr);

    return rc != 0;
}

#else

#include <pthread.h>

int get_thread_affinity(pthread_t th, uint64_t * mask)
{
    cpu_set_t st;
    CPU_ZERO(&st);
    enum {__max_cores=64,};
    uint32_t i;
    int rc;

    rc = pthread_getaffinity_np(th, sizeof(cpu_set_t), &st);
    if (rc != 0)
    {
        return -1;
    }
    for (i = 0; i < __max_cores; i += 1)
    {
        if (CPU_ISSET(i, &st))
        {
            *mask |= (1LL << i);
        }
    }
    return 0;
}

int set_thread_affinity(uint8_t * cores, size_t size)
{
    cpu_set_t  mask;
    CPU_ZERO(&mask);
    pthread_t tid = 0;
    uint64_t previous_mask = 0;
    uint64_t this_mask = 0;
    int rc;
    size_t i;

    tid = pthread_self();
    for (i = 0; i < size; i += 1)
    {
        CPU_SET(cores[i], &mask);
        this_mask |= (cores[i] << i);
    }
    rc = pthread_setaffinity_np(tid, sizeof(mask), &mask);
    if (rc != 0)
    {
        return -1;
    }
    get_thread_affinity(tid, &previous_mask);
    fprintf(stderr, "[+] set mask=%X previous=%x %s %s:%d\n", this_mask, previous_mask, __func__, __FILE__, __LINE__); fflush(stderr);
    return -1;
}

#endif
