/* --------------------------------------------------------------------------
 * Copyright (c) 2013-2019 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * --------------------------------------------------------------------------
 *
 * $Revision:   V10.2.0
 *
 * Project:     CMSIS-FreeRTOS
 * Title:       FreeRTOS configuration definitions
 *
 * --------------------------------------------------------------------------*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

#include <stdint.h>
#include "kernel_config.h"

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

//  <o>Minimal stack size [words] <0-65535>
//  <i> Stack for idle task and default task stack in words.
//  <i> Default: 128
#define configMINIMAL_STACK_SIZE                ((uint16_t)(1024))

//  <o>Total heap size [bytes] <0-0xFFFFFFFF>
//  <i> Heap memory size in bytes.
//  <i> Default: 8192
#define configTOTAL_HEAP_SIZE                   ((size_t)8192)

//  <o>Kernel tick frequency [Hz] <0-0xFFFFFFFF>
//  <i> Kernel tick rate in Hz.
//  <i> Default: 1000
#define configTICK_RATE_HZ                      ((TickType_t)CONFIG_KERNEL_TICK_HZ)

//  <o>Timer task stack depth [words] <0-65535>
//  <i> Stack for timer task in words.
//  <i> Default: 80
#define configTIMER_TASK_STACK_DEPTH            1024

//  <o>Timer task priority <0-56>
//  <i> Timer task priority.
//  <i> Default: 40 (High)
#define configTIMER_TASK_PRIORITY               40

//  <o>Timer queue length <0-1024>
//  <i> Timer command queue length.
//  <i> Default: 5
#define configTIMER_QUEUE_LENGTH                5

//  <o>Preemption interrupt priority
//  <i> Maximum priority of interrupts that are safe to call FreeRTOS API.
//  <i> Default: 16
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    16

//  <q>Use time slicing
//  <i> Enable setting to use timeslicing.
//  <i> Default: 1
#ifdef CONFIG_8850CM_TARGET
#define configUSE_TIME_SLICING                  0
#else
#define configUSE_TIME_SLICING                  1
#endif

//  <q>Idle should yield
//  <i> Control Yield behaviour of the idle task.
//  <i> Default: 1
#define configIDLE_SHOULD_YIELD                 1

//  <o>Check for stack overflow
//    <0=>Disable <1=>Method one <2=>Method two
//  <i> Enable or disable stack overflow checking.
//  <i> Callback function vApplicationStackOverflowHook implementation is required when stack checking is enabled.
//  <i> Default: 0
#define configCHECK_FOR_STACK_OVERFLOW          2

//  <q>Use idle hook
//  <i> Enable callback function call on each idle task iteration.
//  <i> Callback function vApplicationIdleHook implementation is required when idle hook is enabled.
//  <i> Default: 0
#define configUSE_IDLE_HOOK                     1
#define configUSE_TICKLESS_IDLE                 1
#define vApplicationIdleHook                    osiIdleHook

//  <q>Use tick hook
//  <i> Enable callback function call during each tick interrupt.
//  <i> Callback function vApplicationTickHook implementation is required when tick hook is enabled.
//  <i> Default: 0
#define configUSE_TICK_HOOK                     0

//  <q>Use deamon task startup hook
//  <i> Enable callback function call when timer service starts.
//  <i> Callback function vApplicationDaemonTaskStartupHook implementation is required when deamon task startup hook is enabled.
//  <i> Default: 0
#define configUSE_DAEMON_TASK_STARTUP_HOOK      1
#define vApplicationDaemonTaskStartupHook       osiAppStart

//  <q>Use malloc failed hook
//  <i> Enable callback function call when out of dynamic memory.
//  <i> Callback function vApplicationMallocFailedHook implementation is required when malloc failed hook is enabled.
//  <i> Default: 0
#define configUSE_MALLOC_FAILED_HOOK            0

//  <o>Queue registry size
//  <i> Define maximum number of queue objects registered for debug purposes.
//  <i> The queue registry is used by kernel aware debuggers to locate queue and semaphore structures and display associated text names.
//  <i> Default: 0
#define configQUEUE_REGISTRY_SIZE               0

//  <o>Interrupt controller base address
//  <i> Must be set to the base address of the ARM Generic Interrupt Controller (GIC).
//  <i> Default: 0x2C000000
#define configINTERRUPT_CONTROLLER_BASE_ADDRESS CONFIG_GIC_BASE_ADDRESS

//  <o>Interrupt controller CPU interface offset
//  <i> Must be set to the offset from interrupt controller base address at which the CPU interface starts.
//  <i> Default: 0x1000
#define configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET CONFIG_GIC_CPU_INTERFACE_OFFSET

//  <o>Interrupt controller unique priorities
//  <i> The number of unique priorities that can be specified in the ARM Generic Interrupt Controller (GIC).
//  <i> Default: 32
#ifdef CONFIG_TRUSTZONE_SUPPORT
#define configUNIQUE_INTERRUPT_PRIORITIES       16
#else
#define configUNIQUE_INTERRUPT_PRIORITIES       32
#endif

//  <o>Maximum API call interrupt priority
//  <i> Interrupts assigned a priority at or below this priority can call interrupt safe FreeRTOS API function, and will nest.
//  <i> Interrupts assigned a priority above this priority will not be effected by RTOS critical sections, and will nest,
//  <i> but cannot call any FreeRTOS API functions.
//  <i> Default: 32
#ifdef CONFIG_TRUSTZONE_SUPPORT
#define configMAX_API_CALL_INTERRUPT_PRIORITY   16
#else
#define configMAX_API_CALL_INTERRUPT_PRIORITY   32
#endif

#define configUSE_NEWLIB_REENTRANT              1

#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 1

#define configUSE_TASK_FPU_SUPPORT              2

//------------- <<< end of configuration section >>> ---------------------------

/* Defines needed by FreeRTOS to implement CMSIS RTOS2 API. Do not change! */
#undef configCPU_CLOCK_HZ
#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configUSE_PREEMPTION                    1
#define configUSE_TIMERS                        1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_TRACE_FACILITY                1
#define configUSE_16_BIT_TICKS                  0
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configMAX_PRIORITIES                    56
#define configKERNEL_INTERRUPT_PRIORITY         255

/* Defines that include FreeRTOS functions which implement CMSIS RTOS2 API. Do not change! */
#define INCLUDE_xEventGroupSetBitsFromISR       1
#define INCLUDE_xSemaphoreGetMutexHolder        1
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_xTaskGetCurrentTaskHandle       1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_eTaskGetState                   1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_xTimerPendFunctionCall          1

/* BEGIN: Added by liuchong, 2021/3/22   Promblem Number:SN-20210312009
   Description    : Add cpu usage function. */
#define configGENERATE_RUN_TIME_STATS           CONFIG_GENERATE_RUN_TIME_STATS
#define configUSE_STATS_FORMATTING_FUNCTIONS    CONFIG_USE_STATS_FORMATTING_FUNCTIONS

#if ( configGENERATE_RUN_TIME_STATS == 1 )
#define xTaskGetIdleRunTimeCounter    ulTaskGetIdleRunTimeCounter
#define INCLUDE_xTaskGetIdleTaskHandle     1
extern uint32_t didi_32K_tick(void);
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#define portGET_RUN_TIME_COUNTER_VALUE()  didi_32K_tick()
#endif
/* END:   Added by liuchong, 2021/3/22 */

#endif /* FREERTOS_CONFIG_H */
