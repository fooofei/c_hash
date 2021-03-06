#pragma once

#include <stdint.h>

/* set cores array affinity 
 * return 0 for success
*/
static int set_thread_affinity(uint8_t * cores, size_t size);




#ifndef WIN32
#define _GNU_SOURCE
#endif
#include <stdio.h>

/** 
  pthread_getaffinity_np 与DPDK关系
  1 DPDK 的各core直接是分core 运行的
  2 以 -l 3-5 传递命令行参数运行的，在 rte_eal_mp_remote_launch 之后，获得的 core mask 分别为 3,4,5
    并不是 4,5， core3在代码中标记为 MASTER 还是运行了的
  3 运行的 rte_eal_mp_remote_launch(,, SKIP_MASTER) 还是 rte_eal_mp_remote_launch(,,CALL_MASTER) 
    都会被重新set affinity
*/

#ifdef WIN32

#include <windows.h>

static int set_thread_affinity(uint8_t * cores, size_t size)
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

static int get_thread_affinity(pthread_t th, uint64_t * mask)
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

static int set_thread_affinity(uint8_t * cores, size_t size)
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
