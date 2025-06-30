/* Copyright (C) 2022 FIBOCOM Technologies Limited and/or its affiliates("FIBOCOM").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * FIBOCOM assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. FIBOCOM reserves the right to make changes in the
 * software without notification.  FIBOCOM also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */


#define OSI_LOG_TAG OSI_MAKE_LOG_TAG('M', 'Y', 'A', 'P')

#include "fibo_opencpu.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
typedef __uint32_t uint32_t ;



static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

static void test_ticks()
{
    UINT32 ticks = 0;
    ticks = fibo_get_sys_tick();
    fibo_textTrace("before ticks = %u", ticks);
    fibo_taskSleep(1000);
    ticks = fibo_get_sys_tick();
    fibo_textTrace("after ticks = %u", ticks);
}

uint32_t g_size;
uint32_t g_avail_size;
uint32_t g_max_block_size;
static void test_mem()
{
    char *pt = (char *)fibo_malloc(51200);
    if (pt != NULL)
    {
        fibo_textTrace("malloc address %u", (unsigned int)pt);
        fibo_free(pt);
    }

    fibo_heapinfo_get(&g_size, &g_avail_size, &g_max_block_size);
    fibo_textTrace("g_size = %u, g_avail_size = %u, g_max_block_size = %u", g_size, g_avail_size, g_max_block_size);
}

static void test_reset(int mode)
{
    if (mode == 0)
        fibo_soft_reset();
    else
        fibo_soft_power_off();
}


/* sem test begin */
UINT32 g_test_sem;
static void testSemThread(void *param)
{
    fibo_taskSleep(2000);
    fibo_textTrace("execute sem_signal...");
    fibo_sem_signal(g_test_sem);
    fibo_taskSleep(1000);
    fibo_sem_signal(g_test_sem);
    fibo_thread_delete();
}
static void test_sem()
{
    g_test_sem = fibo_sem_new(0);

    fibo_thread_create(testSemThread, "test_sem", 1024, NULL, OSI_PRIORITY_NORMAL);

    fibo_sem_wait(g_test_sem);
    fibo_textTrace("after sem_wait...");

    fibo_sem_try_wait(g_test_sem, 5000);
    fibo_textTrace("after sem_try_wait...");

    fibo_sem_try_wait(g_test_sem, 5000);
    fibo_textTrace("after sem_try_wait...");
    fibo_sem_free(g_test_sem);
}
/* sem test end */

/* mutex test begin */
UINT32 g_test_mutex;
static void test_mutex()
{
    g_test_mutex = fibo_mutex_create();
    fibo_textTrace("after mutex create...");

    fibo_mutex_lock(g_test_mutex);
    fibo_textTrace("after mutex lock...");

    fibo_mutex_unlock(g_test_mutex);
    fibo_textTrace("after mutex unlock...");

    fibo_mutex_try_lock(g_test_mutex, 3000);
    fibo_textTrace("after mutex try lock...");

    fibo_mutex_delete(g_test_mutex);
    fibo_textTrace("after mutex delete...");
}
/* mytex test end */

/* test timer begin */
void *g_test_timer = NULL;
void *g_test_timer2 = NULL;
void *g_period_timer = NULL;
void *g_period_timer2 = NULL;

void timer_function(void *arg)
{
    fibo_textTrace("timer function execute ...");
    fibo_timer_delete(g_test_timer);
}

void test_timer()
{
    g_test_timer = fibo_timer_create(timer_function, NULL, false);
    fibo_textTrace("test_timer_creat 1...");
    fibo_timer_start(g_test_timer, 6000, false);
}

void timer_function2(void *arg)
{
    fibo_textTrace("timer function2 execute ...");
    fibo_timer_delete(g_test_timer2);
}

void test_timer2()
{
    g_test_timer2 = fibo_timer_create(timer_function2, NULL, false);
    fibo_textTrace("test_timer_creat 2");

    fibo_timer_start(g_test_timer2, 6000, false);
}

void timer_function_period(void *arg)
{
    static int nr = 5;
    fibo_textTrace("timer period function execute ...");
    nr--;
    if(nr == 0)
        fibo_timer_delete(g_period_timer);
}

void test_timer_period()
{
    g_period_timer = fibo_timer_create(timer_function_period, NULL, false);
    fibo_timer_start(g_period_timer, 3000, true);
}

void timer_function_period2(void *arg)
{
    static int nr = 5;
    fibo_textTrace("timer period function2 execute cnt: %d...", nr);
    nr--;
    if(nr == 0)
    {
        fibo_timer_delete(g_period_timer2);
    }
}

void test_timer_period2()
{
    g_period_timer2 = fibo_timer_create(timer_function_period2, NULL, false);
    fibo_textTrace("test_timer_period2 ...");
    fibo_timer_start(g_period_timer2, 3000, true);
}
/* test timer end */

void test_rand()
{
    srand(1000);
    for (int i = 0; i < 10; i++)
    {
        fibo_textTrace("rand %d : %d", i, rand());
    }
}

/* test queue */
UINT32 g_queue_id = 0;
static void testQueueThread(void *param)
{
    int value = 8888;
    fibo_taskSleep(2000);
    fibo_queue_put(g_queue_id, &value, 0);
    fibo_thread_delete();
}

void test_queue()
{
    int value = 0;
    g_queue_id = fibo_queue_create(5, sizeof(int));
    fibo_thread_create(testQueueThread, "test_queue", 4096, NULL, OSI_PRIORITY_NORMAL);
    fibo_queue_get(g_queue_id, (void *)&value, 0);
    fibo_textTrace("test queue value = %d", value);
}

UINT32 g_queue_id2 = 0;
static void testQueueThread2(void *param)
{
    int i = 0;
    int value = 1111;
    fibo_taskSleep(2000);

    for (i = 1; i < 10; i++)
    {
        value = 1111 *i;
        fibo_queue_put_to_front(g_queue_id2, &value, 0);
        fibo_textTrace("test queue2 value put to front(%d) = %d", i, value);
    }

    fibo_thread_delete();
}

void test_queue2()
{
    int value = 0;
    g_queue_id2 = fibo_queue_create(10, sizeof(int));
    fibo_thread_create(testQueueThread2, "test_queue2", 4096, NULL, OSI_PRIORITY_NORMAL);

    fibo_taskSleep(5000);

    for (int i = 1; i < 10; i++)
    {
        fibo_queue_get(g_queue_id2, (void *)&value, 0);
        fibo_textTrace("test queue2 value get(%d) = %d", i, value);
    }
}

/* test queue end */

static void nullThread(void *param)
{
    for (int n = 0; n < 5; n++)
    {
        fibo_textTrace("null Thread in %d", n);
        fibo_taskSleep(1000);
    }
    fibo_thread_delete();
}


static void oc_osi_demo(void *param)
{
    fibo_textTrace("application thread enter, param 0x%x", param);
    test_ticks();
    test_mem();
    test_sem();
    test_mutex();
    test_timer();
    test_timer2();
    test_rand();
    test_queue();
    test_queue2();
    test_timer_period();
    test_timer_period2();
    test_reset(0);

    for (int n = 0; n < 10; n++)
    {
        fibo_textTrace("hello world %d", n);
        fibo_taskSleep(1000);
    }
    fibo_taskSleep(1000);
    //test_reset(0);

    UINT32 thread_id = 0;
    fibo_thread_create_ex(nullThread, "nullthread",
        1024, NULL, OSI_PRIORITY_NORMAL, &thread_id);
    fibo_textTrace("nullThread id = 0x%x", thread_id);

    fibo_thread_delete();
}

void* appimg_enter(void *param)
{
    fibo_textTrace("application image enter, param 0x%x", param);

    prvInvokeGlobalCtors();

    fibo_thread_create(oc_osi_demo, "oc_osi_demo", 1024, NULL, OSI_PRIORITY_NORMAL);
    return 0;
}

void appimg_exit(void)
{
    fibo_textTrace("application image exit");
}


