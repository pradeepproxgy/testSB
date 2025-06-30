/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/reent.h>
#include "hal_config.h"
#include "cmsis_core.h"
#include "osi_api.h"
#include "osi_compiler.h"
#include "osi_profile.h"
#include "osi_tick_unit.h"
#include "osi_mem.h"
#include "osi_log.h"
#include "osi_trace.h"
#include "hwregs.h"
#include "hal_chip.h"
#include "hal_mmu.h"
#include "hal_adi_bus.h"
#include "hal_cp_ctrl.h"
#include "hal_iomux.h"
#include "drv_md_ipc.h"
#include "drv_md_nvm.h"
#include "drv_ipc_at.h"
#include "drv_usb.h"
#include "hal_spi_flash.h"
#include "hal_hwspinlock.h"
#include "hal_efuse.h"
#include <string.h>
#include "hal_shmem_region.h"
#include "drv_config.h"
#include "app_config.h"

#ifdef CONFIG_TRUSTZONE_SUPPORT
#include "smc.h"
#include "smcall.h"
#endif

#define MEM_PM2_AON_DATE_NAME "pm2_aon_data"
#define IDLE_SLEEP_ENABLE_MAGIC 0x49444c45

#define AP_SRAM_BACKUP_START_SYM __sramboottext_start
#define AP_SRAM_BACKUP_END_SYM __srambss_end

#define AP_SRAM_BACKUP_START ((uintptr_t)(AP_SRAM_BACKUP_START_SYM))
#define AP_SRAM_BACKUP_END ((uintptr_t)(AP_SRAM_BACKUP_END_SYM))
#define AP_SRAM_BACKUP_SIZE (AP_SRAM_BACKUP_END - AP_SRAM_BACKUP_START)
#define AP_SRAM_BACKUP_ALLOC_SIZE (CONFIG_SRAM_SIZE - CONFIG_IRQ_STACK_SIZE - CONFIG_SVC_STACK_SIZE - CONFIG_BLUE_SCREEN_SIZE)

#ifdef CONFIG_TRUSTZONE_SUPPORT
#define PSCI_CPU_SUSPEND_POWER_STATE(lv, type, id) (((lv & 0x03) << 24) | ((type & 0x01) << 16) | (id & 0xFFFF))
#endif

// ap_awk_st
typedef union {
    uint32_t v;
    struct
    {
        uint32_t awk0_awk_stat : 1;  // [0]
        uint32_t awk1_awk_stat : 1;  // [1]
        uint32_t awk2_awk_stat : 1;  // [2]
        uint32_t awk3_awk_stat : 1;  // [3]
        uint32_t awk4_awk_stat : 1;  // [4]
        uint32_t awk5_awk_stat : 1;  // [5]
        uint32_t awk6_awk_stat : 1;  // [6]
        uint32_t awk7_awk_stat : 1;  // [7]
        uint32_t awk8_awk_stat : 1;  // [8]
        uint32_t awk9_awk_stat : 1;  // [9]
        uint32_t awk10_awk_stat : 1; // [10]
        uint32_t awk11_awk_stat : 1; // [11]
        uint32_t awk12_awk_stat : 1; // [12]
        uint32_t awk13_awk_stat : 1; // [13]
        uint32_t awk14_awk_stat : 1; // [14]
        uint32_t awk15_awk_stat : 1; // [15]
        uint32_t awk16_awk_stat : 1; // [16]
        uint32_t awk17_awk_stat : 1; // [17]
        uint32_t awk18_awk_stat : 1; // [18]
        uint32_t awk19_awk_stat : 1; // [19]
        uint32_t awk20_awk_stat : 1; // [20]
        uint32_t awk21_awk_stat : 1; // [21]
        uint32_t awk22_awk_stat : 1; // [22]
        uint32_t awk23_awk_stat : 1; // [23]
        uint32_t p1_awk_stat : 1;    // [24]
        uint32_t t1_awk_stat : 1;    // [25]
        uint32_t t2_awk_stat : 1;    // [26]
        uint32_t t3_awk_stat : 1;    // [27]
        uint32_t t4_awk_stat : 1;    // [28]
        uint32_t t5_awk_stat : 1;    // [29]
        uint32_t t6_awk_stat : 1;    // [30]
        uint32_t p2_awk_stat : 1;    // [31]
    } b;
} REG_IDLE_LPS_AP_AWK_ST_T;

typedef struct
{
    osiSuspendMode_t mode;
    uint32_t reg_sys[11]; // r4-r14
    uint32_t cpsr_sys;
    uint32_t ttbr0;
    uint32_t dacr;
    uint32_t sys_ctrl_cfg_misc_cfg;
    uint8_t sram[AP_SRAM_BACKUP_ALLOC_SIZE];
    //uint8_t sram_shmem[CONFIG_AON_SRAM_SHMEM_SIZE];
} cpuSuspendContext_t;

static cpuSuspendContext_t gCpuSuspendCtx;

extern void osiWakePm1Entry(void);
extern void osiWakePm2Entry(void);

#ifdef CONFIG_TRUSTZONE_SUPPORT
typedef void (*SML_WarmBoot_entry)(void);
extern void osiWarmbootPM1Entry(void);
extern void osiWarmbootPM2Entry(void);
#endif

#ifdef CONFIG_8850CM_TARGET
bool osiCPPmPermitted(osiSuspendMode_t mode);
#endif

/**
 * Backup SRAM to PSRAM, due to SRAM will be power off during suspend.
 */
static void prvSramBackup(void)
{
    extern uint32_t AP_SRAM_BACKUP_START_SYM[];
    extern uint32_t AP_SRAM_BACKUP_END_SYM[];
    memcpy(gCpuSuspendCtx.sram, (void *)AP_SRAM_BACKUP_START, AP_SRAM_BACKUP_SIZE);
    // memcpy(gCpuSuspendCtx.sram_shmem, (void *)CONFIG_AON_SRAM_SHMEM_ADDRESS, CONFIG_AON_SRAM_SHMEM_SIZE);
}

/**
 * Restore SRAM from PSRAM
 */
static void prvSramRestore(void)
{
    extern uint32_t AP_SRAM_BACKUP_START_SYM[];
    extern uint32_t AP_SRAM_BACKUP_END_SYM[];
    memcpy((void *)AP_SRAM_BACKUP_START, gCpuSuspendCtx.sram, AP_SRAM_BACKUP_SIZE);
    // memcpy((void *)CONFIG_AON_SRAM_SHMEM_ADDRESS, gCpuSuspendCtx.sram_shmem, CONFIG_AON_SRAM_SHMEM_SIZE);
}

void prvPm2RegBackup()
{
    const halShmemRegion_t *pm2_shmem = halShmemGetRegion(MEM_PM2_AON_DATE_NAME);
    int32_t i = 0;
    uint32_t *p = (uint32_t *)(pm2_shmem->address);
    int32_t count = *p;
    uint32_t *pointer;
    for (i = 1; i <= count; i++)
    {
        pointer = (uint32_t *)*(p + 2 * i + 1);
        *(p + 2 * i) = *pointer;
    }
}

void prvPm2RegRestore()
{
    volatile HWP_IDLE_LPS_T *idleLps = hwp_idleLps;
    if (idleLps->ap_pm2_sta == 0)
    {
        return;
    }

    const halShmemRegion_t *pm2_shmem = halShmemGetRegion(MEM_PM2_AON_DATE_NAME);
    int32_t i = 0;
    uint32_t *p = (uint32_t *)(pm2_shmem->address);
    int32_t count = *p;
    uint32_t *pointer;
    for (i = 1; i <= count; i++)
    {
        pointer = (uint32_t *)*(p + 2 * i + 1);
        *pointer = *(p + 2 * i);
    }
}

/**
 * Save context (r4-r14) to specified memory, and switch stack to
 * SRAM, and call specified function. If \p post returns, this will
 * return true, to indicate suspend fail.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
__attribute__((naked, target("arm"))) static bool prvInvokeSuspendPost(void *ctx, void (*post)(void))
{
    asm volatile("stmia  r0, {r4-r14}\n"
                 "mov    r2, sp\n"
                 "ldr    sp, =__svc_stack_end\n"
                 "push   {r2, lr}\n"
                 "blx    r1\n"
                 "pop    {r2, lr}\n"
                 "mov    sp, r2\n"
                 "mov    r0, #1\n"
                 "bx     lr\n");
}
#pragma GCC diagnostic pop

/**
 * Restore context from specified memory, and return to \p lr in the
 * context, rather then caller of this. The return value for \p lr
 * (not for caller) is false, to indicate resume.
 */
__attribute__((naked, target("arm"))) static void prvRestoreContext(void *ctx)
{
    asm volatile("ldmia   r0, {r4-r14}\n"
                 "mov     r0, #0\n"
                 "bx      lr");
}

/**
 * MMU initialization for cold boot
 */
static inline void prvMmuInit(void)
{
    // Invalidate entire Unified TLB
    __set_TLBIALL(0);

    // Invalidate entire branch predictor array
    __set_BPIALL(0);
    __DSB();
    __ISB();

    //  Invalidate instruction cache and flush branch target cache
    __set_ICIALLU(0);
    __DSB();
    __ISB();

    //  Invalidate data cache
    L1C_InvalidateDCacheAll();
    L1C_InvalidateICacheAll();

    // Create Translation Table
    halMmuCreateTable();

    // Enable MMU
    MMU_Enable();

    // Enable Caches
    L1C_EnableCaches();
    L1C_EnableBTAC();
}

/**
 * MMU initialization for wakeup. TTB is restored from RAM.
 */
static void prvMmuWakeInit(void)
{
    // Invalidate entire Unified TLB
    __set_TLBIALL(0);

    // Invalidate entire branch predictor array
    __set_BPIALL(0);
    __DSB();
    __ISB();

    //  Invalidate instruction cache and flush branch target cache
    __set_ICIALLU(0);
    __DSB();
    __ISB();

    // Invalidate data cache
    L1C_InvalidateDCacheAll();

    // Restore Translation Table
    __set_TTBR0(gCpuSuspendCtx.ttbr0);
    __ISB();
    __set_DACR(gCpuSuspendCtx.dacr);
    __ISB();

    // Enable MMU
    MMU_Enable();

    // Enable Caches
    L1C_EnableCaches();
    L1C_EnableBTAC();
}

/**
 * Initialize heap
 */
static void prvHeapInit(void)
{
    extern uint32_t __heap_start;
    extern uint32_t __heap_end;

    void *heap_start = (void *)&__heap_start;
    uint32_t heap_size = (uint32_t)&__heap_end - (uint32_t)&__heap_start;

    osiMemPool_t *ram_heap = osiBlockPoolInit(heap_start, heap_size, 0);
    osiPoolSetDefault(ram_heap);

    OSI_LOGI(0x10007aa2, "ram heap start/%p size/%d pool/%p", heap_start, heap_size, ram_heap);
}

/**
 * Cold boot, called from \p Reset_Handler.
 */
OSI_NO_RETURN void osiBootStart(uint32_t param)
{
    // I cache is enabled, and D cache is disabled
    OSI_LOAD_SECTION(sramboottext);
    OSI_LOAD_SECTION(sramtext);
    OSI_LOAD_SECTION(sramdata);
    OSI_CLEAR_SECTION(srambss);

    // sync cache after code copy
    L1C_InvalidateICacheAll();

    halClockInit(HAL_CLOCK_INIT_APP);
    halHwspinlockAcquireInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);

#if !defined(CONFIG_APP_COMPRESSED_SUPPORT)
    halRamInit();
#endif

    halHwspinlockReleaseInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);
    prvMmuInit();

    OSI_LOAD_SECTION(ramtext);
    OSI_LOAD_SECTION(data);
    OSI_CLEAR_SECTION(bss);

    // sync cache after code copy
    L1C_CleanDCacheAll();
    __DSB();
    __ISB();

    __FPU_Enable();
    _impure_ptr = _GLOBAL_REENT;

    osiTraceBufInit();
    osiProfileInit();
    osiPmInit();
    prvHeapInit();

    halSysWdtStop();
    halSysWdtConfig(CONFIG_SYS_WDT_TIMEOUT);

    halIomuxInit();
    halAdiBusInit();

    halPmuInit(HAL_CLOCK_INIT_APP);
    halPmuExtFlashPowerOn();
#ifdef CONFIG_MC661_CN_19_70_YINGSHI
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_19_PAD_GPIO_19);
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_19_PAD_GPIO_19, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_19_PAD_GPIO_19, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_43_PAD_I2C_M2_SDA);
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_43_PAD_I2C_M2_SDA, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_43_PAD_I2C_M2_SDA, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_21_PAD_GPIO_21);
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_21_PAD_GPIO_21, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_21_PAD_GPIO_21, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_30_PAD_I2C_M1_SDA);//6
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_30_PAD_I2C_M1_SDA, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_30_PAD_I2C_M1_SDA, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_4_PAD_GPIO_4);//19
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_4_PAD_GPIO_4, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_4_PAD_GPIO_4, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_2_PAD_GPIO_2);//16
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_2_PAD_GPIO_2, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_2_PAD_GPIO_2, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_1_PAD_GPIO_1);//20
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_1_PAD_GPIO_1, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_1_PAD_GPIO_1, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_3_PAD_GPIO_3);//21
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_3_PAD_GPIO_3, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_3_PAD_GPIO_3, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_15_PAD_UART_1_RTS);//22
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_15_PAD_UART_1_RTS, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_15_PAD_UART_1_RTS, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_14_PAD_UART_1_CTS);//23
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_14_PAD_UART_1_CTS, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_14_PAD_UART_1_CTS, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_0_PAD_GPIO_0);//25
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_0_PAD_GPIO_0, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_0_PAD_GPIO_0, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_46_PAD_I2S1_MCLK);//26
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_46_PAD_I2S1_MCLK, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_46_PAD_I2S1_MCLK, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_19_PAD_I2S1_BCK);//30
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_19_PAD_I2S1_BCK, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_19_PAD_I2S1_BCK, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_47_PAD_SPI_CAMERA_SI_0);//55
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_47_PAD_SPI_CAMERA_SI_0, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_47_PAD_SPI_CAMERA_SI_0, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_46_PAD_CAMERA_REF_CLK);//54
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_46_PAD_CAMERA_REF_CLK, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_46_PAD_CAMERA_REF_CLK, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_37_PAD_SPI_LCD_CLK);//53
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_37_PAD_SPI_LCD_CLK, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_37_PAD_SPI_LCD_CLK, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_38_PAD_SPI_LCD_CS);//52
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_38_PAD_SPI_LCD_CS, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_38_PAD_SPI_LCD_CS, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_36_PAD_SPI_LCD_SDC);//51
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_36_PAD_SPI_LCD_SDC, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_36_PAD_SPI_LCD_SDC, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_22_PAD_I2S1_SDAT_O);//33
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_22_PAD_I2S1_SDAT_O, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_22_PAD_I2S1_SDAT_O, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_21_PAD_I2S_SDAT_I);//32
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_21_PAD_I2S_SDAT_I, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_21_PAD_I2S_SDAT_I, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_20_PAD_I2S1_LRCK);//31
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_20_PAD_I2S1_LRCK, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_20_PAD_I2S1_LRCK, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_35_PAD_KEYOUT_3);//74
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_35_PAD_KEYOUT_3, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_35_PAD_KEYOUT_3, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_34_PAD_KEYOUT_2);//76
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_34_PAD_KEYOUT_2, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_34_PAD_KEYOUT_2, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_40_PAD_LCD_FMARK);//78
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_40_PAD_LCD_FMARK, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_40_PAD_LCD_FMARK, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_18_PAD_SPI_CAMERA_SCK);//80
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_18_PAD_SPI_CAMERA_SCK, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_18_PAD_SPI_CAMERA_SCK, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_45_PAD_CAMERA_PWDN);//81
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_45_PAD_CAMERA_PWDN, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_45_PAD_CAMERA_PWDN, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_32_PAD_KEYOUT_0);//83
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_32_PAD_KEYOUT_0, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_32_PAD_KEYOUT_0, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_8_PAD_KEYIN_4);//84
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_8_PAD_KEYIN_4, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_8_PAD_KEYIN_4, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_10_PAD_KEYOUT_4);//85
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_10_PAD_KEYOUT_4, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_10_PAD_KEYOUT_4, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_33_PAD_KEYOUT_1);//86
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_33_PAD_KEYOUT_1, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_33_PAD_KEYOUT_1, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_44_PAD_CAMERA_RST_L);//103
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_44_PAD_CAMERA_RST_L, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_44_PAD_CAMERA_RST_L, HAL_IOMUX_FORCE_PULL_NONE);
    halIomuxSetFunction(HAL_IOMUX_FUN_GPIO_18_PAD_GPIO_18);//101
    halIomuxSetPadInout(HAL_IOMUX_FUN_GPIO_18_PAD_GPIO_18, HAL_IOMUX_FORCE_INPUT);
    halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_18_PAD_GPIO_18, HAL_IOMUX_FORCE_PULL_NONE);
#endif
#if defined(CONFIG_FIBOCOM_MC661) && defined(CONFIG_FIBO_CUS_HZHC_FEATURE)

halIomuxSetPadPull(HAL_IOMUX_FUN_GPIO_19_PAD_GPIO_19,HAL_IOMUX_FORCE_PULL_NONE);
#endif

#ifdef CONFIG_FIBOCOM_MC660
    halIomuxSetFunction(HAL_IOMUX_FUN_UART_3_TXD_PAD_GPIO_16);
#endif
    REG_PMIC_RTC_ANA_CLK32KLESS_CTRL0_T clk32kless_ctrl0;
    clk32kless_ctrl0.v = halAdiBusRead(&hwp_pmicRtcAna->clk32kless_ctrl0);
    if (clk32kless_ctrl0.b.rtc_mode == 0)
    {
        REG_ANALOG_G3_ANALOG_OSC_26M_APLL_CTRL_T analog_ctrl;
        analog_ctrl.v = hwp_analogG3->analog_osc_26m_apll_ctrl;
        analog_ctrl.b.analog_osc_26m_osc26m_c_tune = 3;
        analog_ctrl.b.analog_osc_26m_osc26m_r_tune = 6;
        hwp_analogG3->analog_osc_26m_apll_ctrl = analog_ctrl.v;
        halClkInitCalib();
    }

    osiKernelStart();
}

/**
 * Initialize heap
 */
static void prvClockInit(void)
{
    // now: I cache is enabled, D cache is disabled, only function
    //		on flash can be executed.

    // choose apll500M, aon defalut HW ctrl
    REG_AP_CLK_CGM_AP_A5_SEL_CFG_T cgm_ap_a5_sel_cfg = {hwp_apClk->cgm_ap_a5_sel_cfg};
    cgm_ap_a5_sel_cfg.b.cgm_ap_a5_sel = 5;
    hwp_apClk->cgm_ap_a5_sel_cfg = cgm_ap_a5_sel_cfg.v;

    // close debug function, use clk_mode and clk_en control
    hwp_apApb->cgm_gate_auto_sel0 = 0xffffffff;
    hwp_apApb->cgm_gate_auto_sel1 = 0xffffffff;
    hwp_apApb->cgm_gate_auto_sel2 = 0xffffffff;
    hwp_apApb->cgm_gate_auto_sel3 = 0xffffffff;

    hwp_apApb->clk_ap_mode0 = 0; //clk_core auto gating
    hwp_apApb->clk_ap_mode1 = 0; //ip clk auto gating
    hwp_apApb->clk_ap_mode2 = 0;

#ifndef CONFIG_FIBOCOM_BASE
#ifdef CONFIG_BOARD_WITH_EXT_FLASH
    // external flash on vio, 83MHz.
    //sel spiflash clksource apa5 500M
    hwp_apClk->cgm_spiflash2_sel_cfg = 4;

    //div apa5 500M to 3 for spiflash2 500M -> 166M
    hwp_apApb->cfg_clk_spiflash2 = 0xa;

    REG_SPI_FLASH_SPI_CONFIG_T spi_config = {
        .b.quad_mode = 1,
        .b.sample_delay = 2,
        .b.clk_divider = 2,
    };
    hwp_spiFlashExt->spi_config = spi_config.v;
#endif
#endif

#ifndef CONFIG_BOARD_WITH_EXT_FLASH
#ifdef CONFIG_SPI_NAND_DRIVER_SUPPORT
    // external spi nand flash on vio
    //config spi nand flash oper mode
#define SPI_NAND_QUAD_MODE

    //enable nand sel
    REGT_FIELD_CHANGE(hwp_spiFlashExt->spi_nand_config, REG_SPI_FLASH_SPI_NAND_CONFIG_T, ram_read_cmd, 0x3, nand_sel, 1);
    //enable spiflash en
    REGT_FIELD_CHANGE(hwp_apApb->clk_ap_en0, REG_AP_APB_CLK_AP_EN0_T, clk_en_spiflash2_fr, 1);
    //sel spiflash clksource apa5 500M
    REGT_FIELD_CHANGE(hwp_apClk->cgm_spiflash2_sel_cfg, REG_AP_CLK_CGM_SPIFLASH2_SEL_CFG_T, cgm_spiflash2_sel, 0x4);
    //div apa5 500M to 2.5 200M for spiflash 500M -> 200M (0xc)
    //cfg_clk_spiflash2 :
    //0x3 =7.0, 0x4 =6.5, 0x5 =6.0, 0x6 =5.5, 0x7 =5.0, 0x8 =4.5.0, 0x9 =4
    //0xa =3.5, 0xb =3.0, 0xc =2.5, 0xd =2.0, 0xe =1.5, 0xf=1.0
    REGT_FIELD_CHANGE(hwp_apApb->cfg_clk_spiflash2, REG_AP_APB_CFG_CLK_SPIFLASH2_T, spiflash2_freq, 0xc);

#ifdef SPI_NAND_QUAD_MODE
    REGT_FIELD_CHANGE(hwp_spiFlashExt->spi_config, REG_SPI_FLASH_SPI_CONFIG_T, quad_mode, 1, sample_delay, 4, clk_divider, 3); // 200M -> 66M
#else
    REGT_FIELD_CHANGE(hwp_spiFlashExt->spi_config, REG_SPI_FLASH_SPI_CONFIG_T, quad_mode, 0, sample_delay, 4, clk_divider, 3); // 200M -> 66M
#endif

#endif

#endif
}

/**
 * PM1 wakeup, it is called from \p osiWakePm1Entry. Clock settings
 * are kept in PM1.
 */
OSI_NO_RETURN void osiWakePm1(void)
{
    prvClockInit();

#if defined(CONFIG_SOC_8850)
    REG_AP_APB_SLEEP_CTRL_T sleep_ctrl_set = {0};
    sleep_ctrl_set.b.deep_sleep_core_int_disable = 1;
    hwp_apApb->sleep_ctrl_set = sleep_ctrl_set.v;
#endif
    halHwspinlockAcquireInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);
    halIomuxG4LatchRelease();
    halRamWakeInit();
    halHwspinlockReleaseInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);
    prvMmuWakeInit();
    __FPU_Enable();

    prvSramRestore();

    // sync cache after code copy
    L1C_CleanDCacheAll();
    __DSB();
    __ISB();

    prvRestoreContext(gCpuSuspendCtx.reg_sys);

    OSI_DEAD_LOOP;
}

/**
 * PM1 wakeup, it is called from \p osiWakePm2Entry. Clock settings
 * are lost in PM2.
 */
unsigned int debug_flag = 1;
unsigned int debug_32k = 0;
OSI_NO_RETURN void osiWakePm2(void)
{
    halClockInit(HAL_CLOCK_INIT_APP);

    halHwspinlockAcquireInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);
    halRamWakeInit();
    if (hwp_idleLps->ap_pm2_sta == 1)
    {
        prvPm2RegRestore();
    }
    halIomuxResume(OSI_SUSPEND_PM2, 0);
    halIomuxG4LatchRelease();

    prvMmuWakeInit();
    __FPU_Enable();

    if (hwp_idleLps->ap_pm2_sta)
    {
        debug_32k = hwp_idleLps->lps_32k_ref;
        //		while (debug_flag);
    }

    prvSramRestore();

    // sync cache after code copy
    L1C_CleanDCacheAll();
    __DSB();
    __ISB();

    if (hwp_idleLps->ap_pm2_sta == 1)
    {
        // hwp_aonClk->cgm_adi_sel_cfg = 0x1; //xtal_26m
        halAdiBusResume();
        // pm2 relative register
        // hwp_adiMst->adi_chanel_en = 0x8;    //idst_aon
        // hwp_adiMst->event1_waddr = 0x918;   //pmic reserved reg6 write 1
        // hwp_adiMst->event1_wdata = 0x10000; //write 1

        halPmuExitPm2();
    }
    halHwspinlockReleaseInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);
    // now: I cache is enabled, D cache is disabled
    // OSI_LOAD_SECTION(sramboottext);

    // sync cache after code copy
    // L1C_InvalidateICacheAll();
    //prvClockInit();

    prvRestoreContext(gCpuSuspendCtx.reg_sys);

    OSI_DEAD_LOOP;
}

#ifdef CONFIG_TRUSTZONE_SUPPORT
extern void halPmuWakeFromDeep(void);
void prvRestoreContextPM1TZ(void)
{
#if defined(CONFIG_USE_PSRAM)
    if (hwp_idleLps->lps_res8 == 0)
    {
        halPmuWakeFromDeep();
#ifdef CONFIG_8850CM_TARGET
        hwp_pwrctrl->cp_pwr_ctrl = 1;
        OSI_LOOP_WAIT(hwp_pwrctrl->cp_pwr_stat == 3);
#endif
    }
    hwp_idleLps->lps_res8 = (hwp_idleLps->lps_res8 | 0x1);
#endif
    halHwspinlockReleaseInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);
    prvMmuWakeInit();
    __FPU_Enable();

    prvSramRestore();

    // sync cache after code copy
    L1C_CleanDCacheAll();
    __DSB();
    __ISB();

    prvRestoreContext(gCpuSuspendCtx.reg_sys);
    OSI_DEAD_LOOP;
}

void prvRestoreContextPM2TZ(void)
{
#if defined(CONFIG_USE_PSRAM)
    if (hwp_idleLps->lps_res8 == 0)
    {
        halPmuWakeFromDeep();
#ifdef CONFIG_8850CM_TARGET
        hwp_pwrctrl->cp_pwr_ctrl = 1;
        OSI_LOOP_WAIT(hwp_pwrctrl->cp_pwr_stat == 3);
#endif
    }
    hwp_idleLps->lps_res8 = (hwp_idleLps->lps_res8 | 0x1);
#endif

    if (hwp_idleLps->ap_pm2_sta == 1)
    {
        prvPm2RegRestore();
    }
    halIomuxResume(OSI_SUSPEND_PM2, 0);
    halIomuxG4LatchRelease();

    prvMmuWakeInit();
    __FPU_Enable();

    if (hwp_idleLps->ap_pm2_sta)
    {
        debug_32k = hwp_idleLps->lps_32k_ref;
        //		while (debug_flag);
    }

    prvSramRestore();

    // sync cache after code copy
    L1C_CleanDCacheAll();
    __DSB();
    __ISB();

    if (hwp_idleLps->ap_pm2_sta == 1)
    {
        //hwp_aonClk->cgm_adi_sel_cfg = 0x1; //xtal_26m
        halAdiBusResume();
        // pm2 relative register
        //hwp_adiMst->adi_chanel_en = 0x8;    //idst_aon
        //hwp_adiMst->event1_waddr = 0x918;   //pmic reserved reg6 write 1
        //hwp_adiMst->event1_wdata = 0x10000; //write 1

        halPmuExitPm2();
    }

    halHwspinlockReleaseInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);

    prvRestoreContext(gCpuSuspendCtx.reg_sys);
    OSI_DEAD_LOOP;
}
#endif

/**
 * Get wake source, and clear wake source
 */
static uint32_t prvGetClearWakeSource(bool aborted)
{
    REG_IDLE_LPS_AP_AWK_ST_T awk_st = {hwp_idleLps->ap_awk_st};
    REG_IDLE_LPS_AP_AWK_ST_T awk_st_clr = {
        .b.awk0_awk_stat = 1,  // pmic
        .b.awk1_awk_stat = 1,  // uart1
        .b.awk2_awk_stat = 1,  // keypad
        .b.awk3_awk_stat = 1,  // gpio1
        .b.awk4_awk_stat = 1,  // gpt1
        .b.awk5_awk_stat = 1,  // uart1_rx
        .b.awk6_awk_stat = 1,  // mailbox_ap
        .b.awk7_awk_stat = 1,  // mailbox_cp
        .b.awk8_awk_stat = 1,  // uart2
        .b.awk9_awk_stat = 1,  // uart3
        .b.awk10_awk_stat = 1, // gpio2
        .b.awk11_awk_stat = 1, // gpt2_irq0
        .b.awk12_awk_stat = 1, // gpt2_irq1
        .b.awk13_awk_stat = 1, // gpt2_irq2
        .b.awk14_awk_stat = 1, // uart2_rx
        .b.awk15_awk_stat = 1, // uart3_rx
        .b.awk16_awk_stat = 1, // usb
        .b.awk17_awk_stat = 1, // spi2
        .b.awk18_awk_stat = 1, // usb se0
        .b.awk19_awk_stat = 1, // rtc_timer
        .b.p1_awk_stat = 1,
        .b.t1_awk_stat = 1,
        .b.t2_awk_stat = 1,
        .b.t3_awk_stat = 1,
        .b.t4_awk_stat = 1,
        .b.t5_awk_stat = 1,
        .b.t6_awk_stat = 1,
        .b.p2_awk_stat = 1};
    hwp_idleLps->ap_awk_st = awk_st_clr.v;

    REG_IDLE_LPS_AP_AWK_ST1_T awk_st1 = {hwp_idleLps->ap_awk_st1};
    REG_IDLE_LPS_AP_AWK_ST1_T awk_st1_clr = {
        .b.ap_awk_sta1 = 7};
    hwp_idleLps->ap_awk_st1 = awk_st1_clr.v;

    uint32_t source = 0;
    if (awk_st.b.awk0_awk_stat)
        source |= HAL_RESUME_SRC_PMIC;
    if (awk_st.b.awk1_awk_stat)
        source |= HAL_RESUME_SRC_UART1;
    if (awk_st.b.awk2_awk_stat)
        source |= HAL_RESUME_SRC_KEYPAD;
    if (awk_st.b.awk3_awk_stat)
        source |= HAL_RESUME_SRC_GPIO1;
    if (awk_st.b.awk4_awk_stat)
        source |= HAL_RESUME_SRC_GPT1;
    if (awk_st.b.awk5_awk_stat)
        source |= HAL_RESUME_SRC_UART1_RX;
    if (awk_st.b.awk6_awk_stat)
        source |= HAL_RESUME_SRC_MAILBOX_AP;
    if (awk_st.b.awk7_awk_stat)
        source |= HAL_RESUME_SRC_MAILBOX_CP;
    if (awk_st.b.awk8_awk_stat)
        source |= HAL_RESUME_SRC_UART2;
    if (awk_st.b.awk9_awk_stat)
        source |= HAL_RESUME_SRC_UART3;
    if (awk_st.b.awk10_awk_stat)
        source |= HAL_RESUME_SRC_GPIO2;
    if (awk_st.b.awk11_awk_stat)
        source |= HAL_RESUME_SRC_GPT2_IRQ0;
    if (awk_st.b.awk12_awk_stat)
        source |= HAL_RESUME_SRC_GPT2_IRQ1;
    if (awk_st.b.awk13_awk_stat)
        source |= HAL_RESUME_SRC_GPT2_IRQ2;
    if (awk_st.b.awk14_awk_stat)
        source |= HAL_RESUME_SRC_UART2_RX;
    if (awk_st.b.awk15_awk_stat)
        source |= HAL_RESUME_SRC_UART3_RX;
    if (awk_st.b.awk16_awk_stat)
        source |= HAL_RESUME_SRC_USB;
    if (awk_st.b.awk17_awk_stat)
        source |= HAL_RESUME_SRC_SPI2;
    if (awk_st.b.awk18_awk_stat)
        source |= HAL_RESUME_SRC_USB_SE0;
    if (awk_st.b.awk19_awk_stat)
        source |= HAL_RESUME_SRC_RTC_TIMER;
    if (awk_st.b.p1_awk_stat)
        source |= HAL_RESUME_SRC_P1;
    if (awk_st.b.t1_awk_stat)
        source |= HAL_RESUME_SRC_T1;
    if (awk_st.b.t2_awk_stat)
        source |= HAL_RESUME_SRC_T2;
    if (awk_st.b.t3_awk_stat)
        source |= HAL_RESUME_SRC_T3;
    if (awk_st.b.t4_awk_stat)
        source |= HAL_RESUME_SRC_T4;
    if (awk_st.b.t5_awk_stat)
        source |= HAL_RESUME_SRC_T5;
    if (awk_st.b.t6_awk_stat)
        source |= HAL_RESUME_SRC_T6;
    if (awk_st.b.p2_awk_stat)
        source |= HAL_RESUME_SRC_P2;
    // source = awk_st.v;

    if ((awk_st1.b.ap_awk_sta1 & 0x1) == 1)
        source |= HAL_RESUME_SRC_T7;
    if ((awk_st1.b.ap_awk_sta1 & 0x2) == 2)
        source |= HAL_RESUME_SRC_T8;
    if ((awk_st1.b.ap_awk_sta1 & 0x4) == 4)
        source |= HAL_RESUME_SRC_T9;
    return source;
}

/**
 * Suspend after stack is switched to SRAM
 */
static void prvSuspendPost(void)
{
    osiSuspendMode_t mode = gCpuSuspendCtx.mode;

    if (mode == OSI_SUSPEND_PM1)
    {
        halPmuEnterPm1();
#ifndef CONFIG_TRUSTZONE_SUPPORT
        halSetWarmBootEntry(osiWakePm1Entry);
#endif
    }
    else
    {
        halPmuEnterPm2();
#ifndef CONFIG_TRUSTZONE_SUPPORT
        halSetWarmBootEntry(osiWakePm2Entry);
#endif
        hwp_idleLps->ap_pm2_mode_en = 1;

        halHwspinlockAcquireInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);
        //if ((hwp_idleLps->lps_res8 | (0x1)) == 0x1)
        {
            prvPm2RegBackup();
        }
        halHwspinlockReleaseInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);
    }

    prvSramBackup();

    // go to sleep, really
    L1C_CleanInvalidateDCacheAll();
    __DSB();

    //disable cache
    L1C_DisableBTAC();
    L1C_DisableCaches();

    // Disable MMU
    MMU_Disable();
#if defined(CONFIG_TRUSTZONE_SUPPORT) && !defined(CONFIG_8850CM_TARGET)
    if (mode == OSI_SUSPEND_PM1)
    {
        smc(SMC_SC_PSCI_CPU_SUSPEND, PSCI_CPU_SUSPEND_POWER_STATE(0, 1, 0), (unsigned long)osiWarmbootPM1Entry, 0);
    }
    else
    {
        smc(SMC_SC_PSCI_CPU_SUSPEND, PSCI_CPU_SUSPEND_POWER_STATE(0, 1, 0), (unsigned long)osiWarmbootPM2Entry, 0);
    }
#else
    halHwspinlockAcquireInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);

#if defined(CONFIG_USE_PSRAM) && defined(CONFIG_PSRAM_LP_HALF_SLEEP)
    // When half sleep is enabled, PSRAM can't be accessed from here.
    // CP shouldn't access PSRAM also. It can't work by just disable
    // CP clock.
    /*
    sctlr = __get_SCTLR();
    sctlr &= ~SCTLR_Z_Msk; // disable branch prediction
    sctlr &= ~SCTLR_M_Msk; // disable MMU, due to TTB is on PSRAM
    __set_SCTLR(sctlr);
    __ISB();
    */
#if 0
    if ((hwp_idleLps->lps_res8 | (0x1)) == 0x1)
        halRamSuspend();
#endif
#endif
#ifdef CONFIG_8850CM_TARGET
    if (!osiCPPmPermitted(mode))
    {
        halHwspinlockReleaseInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);
        goto SleepAbort;
    }

    hwp_idleLps->lps_res8 = (hwp_idleLps->lps_res8 & (~0x1));

    if (hwp_idleLps->lps_res8 == 0)
    {
#ifdef CONFIG_TRUSTZONE_SUPPORT
        if (mode == OSI_SUSPEND_PM1)
        {
            smc(SMC_SC_PSCI_CPU_SUSPEND, PSCI_CPU_SUSPEND_POWER_STATE(0, 1, 0), (unsigned long)osiWarmbootPM1Entry, 0);
        }
        else
        {
            smc(SMC_SC_PSCI_CPU_SUSPEND, PSCI_CPU_SUSPEND_POWER_STATE(0, 1, 0), (unsigned long)osiWarmbootPM2Entry, 0);
        }
        //usually, smc will not return
        goto SleepAbort;
#else
        hwp_pwrctrl->cp_pwr_ctrl = 0;
        OSI_LOOP_WAIT(hwp_pwrctrl->cp_pwr_stat == 2);

        hwp_pwrctrl->pub_pwr_ctrl = 0x3;
        //wait stable
        OSI_POLL_WAIT(hwp_pwrctrl->pub_pwr_stat == 2);
        //hwp_idleLps->cp_sig_en = 0;//need double check ?
        hwp_idleLps->lps_ctrl_cp = IDLE_SLEEP_ENABLE_MAGIC;
#endif
    }
    else
    {
        hwp_idleLps->lps_res8 |= 0x1;
        halHwspinlockReleaseInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);
        //hwp_pwrctrl->pub_pwr_ctrl |= 0x1;
        goto SleepAbort;
    }
#else
    hwp_idleLps->lps_res8 = (hwp_idleLps->lps_res8 & (~0x1));
    hwp_pwrctrl->pub_pwr_ctrl |= 0x1;
#endif

    halHwspinlockReleaseInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);
    __DSB();
    __ISB();
    hwp_idleLps->lps_ctrl_ap = IDLE_SLEEP_ENABLE_MAGIC;
#endif //CONFIG_TRUSTZONE_SUPPORT

    __WFI();
    OSI_DEAD_LOOP;

#ifdef CONFIG_8850CM_TARGET
SleepAbort:
#endif
    // Delay a while. There are no clear reason. Not sure whether it will
    // happen: CPU exit WFI, and then system enter PM1.
    osiDelayLoops(50);

    if (mode == OSI_SUSPEND_PM1)
    {
        OSI_POLL_WAIT(hwp_idleLps->lps_ctrl_ap == 0);
    }
    else
    {
        hwp_idleLps->ap_pm2_mode_en = 1;
        OSI_POLL_WAIT(hwp_idleLps->ap_pm2_sta == 0);
    }

    halSetWarmBootEntry(NULL);

    // Enable MMU
    MMU_Enable();

    // Enable Caches
    L1C_EnableCaches();
    L1C_EnableBTAC();

#if defined(CONFIG_USE_PSRAM) && defined(CONFIG_PSRAM_LP_HALF_SLEEP)
    //    halRamSuspendAbort();
    unsigned sctlr;
    sctlr = __get_SCTLR();
    sctlr |= SCTLR_Z_Msk; // enable branch prediction
    sctlr |= SCTLR_M_Msk; // enable MMU
    sctlr |= SCTLR_AFE_Msk;
    sctlr &= ~SCTLR_TRE_Msk;
    __set_SCTLR(sctlr);
    __ISB();
#endif
}

/**
 * Process after suspend fail (must be no-inline), return wake source.
 */
OSI_NO_INLINE static uint32_t prvSuspendFailed(void)
{
    // come here when suspend failed.
    uint32_t source = prvGetClearWakeSource(true);
    source |= HAL_RESUME_ABORT;

    if (gCpuSuspendCtx.mode == OSI_SUSPEND_PM1)
        halPmuAbortPm1();
    else
        halPmuAbortPm2();

    // At suspend fail, it is possible that AON_LP is already power off and
    // power on. And sysmail is needed to be cleared at power on.
    //OSI_LOOP_WAIT((hwp_pwrctrl->aon_lp_pwr_stat & 0x3) == 0x3);
    for (uintptr_t address = (uintptr_t)&hwp_mailbox->sysmail0;
         address <= (uintptr_t)&hwp_mailbox->sysmail95; address += 4)
        *(volatile unsigned *)address = 0;

    return source;
}

/**
 * Process after wakeup (must be no-inline), return wake source.
 */
OSI_NO_INLINE static uint32_t prvSuspendResume(void)
{
    // come here from wakeup handler
    __set_CPSR(gCpuSuspendCtx.cpsr_sys);

    // hwp_idleLps->ap_awk_en1_clr = 2; // disable it
    hwp_idleLps->lps_t8_en = 0; // disable it

    //halAdiBusResume();
    if (gCpuSuspendCtx.mode == OSI_SUSPEND_PM1)
        halPmuExitPm1();
    else
    {
        //halPmuExitPm2();
    }

    if (gCpuSuspendCtx.mode == OSI_SUSPEND_PM2)
    {
        hwp_apApb->misc_cfg = gCpuSuspendCtx.sys_ctrl_cfg_misc_cfg;

        hwp_idleLps->ap_pm2_mode_en = 0;
#if defined(CONFIG_SOC_8850) && defined(CONFIG_8850CM_TARGET)
        hwp_idleLps->cp_pm2_mode_en = 0;
#endif
    }

    extern uint32_t __svc_stack_start;
    extern uint32_t __svc_stack_end;
    extern uint32_t __irq_stack_start;
    extern uint32_t __irq_stack_end;

    for (uint32_t *p = &__svc_stack_start; p < &__svc_stack_end; ++p)
        *p = 0xA5A5A5A5;

    for (uint32_t *p = &__irq_stack_start; p < &__irq_stack_end; ++p)
        *p = 0xA5A5A5A5;

    halSysWdtConfig(CONFIG_SYS_WDT_TIMEOUT);
    halSysWdtStart();

    // wait cp sleep request again after resume
    ipc_cp_wake_lock();
    return prvGetClearWakeSource(false);
}

uint32_t osiMailboxCheck(void)
{
    volatile HWP_CP_MAILBOX_T *mailbox = hwp_mailbox;

    if (mailbox->intgr0 || mailbox->intgr1 || mailbox->intgr2 || mailbox->intgr3 || mailbox->intgr4 || mailbox->intgr5)
    {
        return 1;
    }

    return 0;
}

/**
 * CPU suspend. It may return either from suspend fail, or from resume.
 */
#ifdef CONFIG_8850CM_TARGET
extern CP_SH_LPS_INFO cpShareLpsInfo;
#endif
uint32_t osiPmCpuSuspend(osiSuspendMode_t mode, int64_t sleep_ms)
{
    // Check whether cp is ready for sleep, and acquire the spinlock to
    // prevent cp go forward after WFI by unexpected reasons.

    REG_IDLE_LPS_AP_LPS_STA_T ap_lps_sta = {hwp_idleLps->ap_lps_sta};
    uint32_t clk32k = hwp_idleLps->lps_32k_ref;
    if (ap_lps_sta.b.ap_perip_awk_stat == 1)
    {
        ap_lps_sta.b.ap_perip_awk_stat = 1;
        hwp_idleLps->ap_lps_sta = ap_lps_sta.v;
        while (clk32k + 2 > (hwp_idleLps->lps_32k_ref))
        {
            __NOP();
        }
        return OSI_RESUME_ABORT;
    }

#ifdef CONFIG_8850CM_TARGET
    REG_IDLE_LPS_CP_LPS_STA_T cp_lps_sta = {hwp_idleLps->cp_lps_sta};
    uint32_t clk_32k = hwp_idleLps->lps_32k_ref;
    if (cp_lps_sta.b.cp_perip_awk_stat == 1)
    {
        cp_lps_sta.b.cp_perip_awk_stat = 1;
        hwp_idleLps->cp_lps_sta = cp_lps_sta.v;
        while (clk_32k + 2 > (hwp_idleLps->lps_32k_ref))
        {
            __NOP();
        }
        return OSI_RESUME_ABORT;
    }
#endif
    //enable se0 wakeup sources on usbattach,and usb suspend
    REG_PWRCTRL_USB_PWR_STAT_T usb_pwr_stat;
    usb_pwr_stat.v = hwp_pwrctrl->usb_pwr_stat;
    bool isUsbAttach = ((usb_pwr_stat.v) & 0x3) == 0x3;

    if (!isUsbAttach)
    {
        hwp_lpsApb->cfg_por_usb_phy = 0x3;
    }
    else if (drvUsbIsSuspend())
    {
        REGT_FIELD_CHANGE(hwp_idleLps->ap_awk_en_set, REG_IDLE_LPS_AP_AWK_EN_T, ap_awk18_en, 1);
    }

    if (osiMailboxCheck())
        return OSI_RESUME_ABORT;

    gCpuSuspendCtx.mode = mode;
    gCpuSuspendCtx.cpsr_sys = __get_CPSR();
    gCpuSuspendCtx.ttbr0 = __get_TTBR0();
    gCpuSuspendCtx.dacr = __get_DACR();
    gCpuSuspendCtx.sys_ctrl_cfg_misc_cfg = hwp_apApb->misc_cfg;

    // REG_IDLE_LPS_AP_INTEN_T ap_inten = {
    //     .b.ap_p1_irq_en = 1,
    //     .b.ap_p2_irq_en = 1,
    //     .b.ap_t1_irq_en = 1,
    //     .b.ap_t2_irq_en = 1,
    //     .b.ap_t3_irq_en = 1,
    //     .b.ap_t4_irq_en = 1,
    //     .b.ap_t5_irq_en = 1,
    //     .b.ap_t6_irq_en = 1,
    //     .b.cp_sys_awk_irq_to_ap_en = 1,
    //     .b.ap_sys_awk_irq_to_ap_en = 1,
    //     .b.ap_load_irq_en = 1,
    //     .b.ap_tstamp_irq_en = 1,
    //     .b.ap_t7_irq_en = 1,
    //     .b.ap_t8_irq_en = 1,
    //     .b.ap_t9_irq_en = 1};

    REG_IDLE_LPS_LPS_T8_EN_T lps_t8_en = {0};

    // REG_IDLE_LPS_AP_AWK_EN1_T ap_awk_en1 = {0};

    if (sleep_ms != INT64_MAX)
    {
        int64_t ticks = OSI_MS_TO_TICK32K(sleep_ms);
        // ap_awk_en1.b.ap_t8_awk_en = 1;
        ticks = (ticks >= 0x7fffffff) ? 0x7fffffff : ticks;
#ifdef CONFIG_8850CM_TARGET
        uint32_t SlpTicks = INT32_MAX;
        uint32_t SlpDurPs = INT32_MAX;
#ifdef CONFIG_FIBOCOM_BASE
       uint32_t MinDurTicks = INT32_MAX;
       uint32_t P1timeDurTicks = hwp_idleLps->cp_p1_time;
#endif
        extern uint32_t tgl_GetPsLpsKeyTimerRemainLenV2AP(uint8_t psMode);
        SlpDurPs = tgl_GetPsLpsKeyTimerRemainLenV2AP(cpShareLpsInfo.sh_psmode);
#ifdef CONFIG_FIBOCOM_BASE
        if (SlpDurPs != UINT32_MAX)
#else
        if (SlpDurPs != INT32_MAX)
#endif
        {
            SlpTicks = OSI_MS_TO_TICK32K(SlpDurPs);
        }
#ifdef CONFIG_FIBOCOM_BASE       /*SPCSS01331416 Solve T8timer wake-up exception*/
        P1timeDurTicks -= hwp_idleLps->lps_32k_ref;
        MinDurTicks = OSI_MIN(uint32_t, SlpTicks,P1timeDurTicks);
        MinDurTicks = OSI_MIN(uint32_t, MinDurTicks,ticks);
        hwp_idleLps->lps_t_time8 = MinDurTicks + hwp_idleLps->lps_32k_ref;
#else
        SlpTicks += hwp_idleLps->lps_32k_ref;

        uint32_t p1time = hwp_idleLps->cp_p1_time;
        ticks += hwp_idleLps->lps_32k_ref;

        ticks = (ticks >= 0x7fffffff) ? 0x7fffffff : ticks;

        if ((p1time >= ticks) && (SlpTicks >= ticks))
            hwp_idleLps->lps_t_time8 = ticks;
        else if ((ticks >= p1time) && (SlpTicks >= p1time))
            hwp_idleLps->lps_t_time8 = p1time;
        else
            hwp_idleLps->lps_t_time8 = SlpTicks;
#endif
#else
        hwp_idleLps->lps_t_time8 = ticks + hwp_idleLps->lps_32k_ref;
#endif
        lps_t8_en.b.lps_t8_en = 1;
    }
    hwp_idleLps->lps_t8_en = lps_t8_en.v;
    // hwp_idleLps->ap_awk_en1 = ap_awk_en1.v;
#ifdef CONFIG_MC661_CN_19_70_JIEKE
    //010
    hwp_gpio1->gpio_oen_set_out = (1 << 0 | 1 << 14 | 1 << 15);
    hwp_gpio1->gpio_clr_reg = (1 << 0 | 1 << 15 );
    hwp_gpio1->gpio_set_reg = ( 1 << 14);
#endif
    bool failed = prvInvokeSuspendPost(gCpuSuspendCtx.reg_sys, prvSuspendPost);
#ifdef CONFIG_MC661_CN_19_70_JIEKE
//011
    hwp_gpio1->gpio_oen_set_out = (1 << 0 | 1 << 14 | 1 << 15);
    hwp_gpio1->gpio_clr_reg = ( 1 << 15 );
    hwp_gpio1->gpio_set_reg = ( 1 << 0 | 1 << 14);
#endif
    // We come here either from suspend fail, or from wakeup. To avoid
    // unwanted compiler optimization, no-inline functions are used.
    OSI_BARRIER();
    unsigned source = failed ? prvSuspendFailed() : prvSuspendResume();

    // Release spinlock, and cp can go forward after WFI.
    // halHwspinlockReleaseInCritical(HAL_HWSPINLOCK_ID_CPSLEEP);
    return source;
    // return OSI_RESUME_ABORT;
}

uint64_t osiCpDeepSleepTime()
{
    return 0xFFFFFF;
    ; // TODO
}
