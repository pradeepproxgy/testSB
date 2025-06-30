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

#include "hal_config.h"
#ifdef CONFIG_SOC_8821
#include "hal_iomux.h"
#include "hal_adi_bus.h"
#include "drv_names.h"
#include "hwregs.h"
#include "osi_api.h"
#include "osi_log.h"
#include "osi_api.h"
#include <stdlib.h>
#include "hal_adi_bus.h"

#define PMIC_GPIO_COUNT (8)
#define REG_IS_PMIC_IOMUX(r) ({ uintptr_t _r = (uintptr_t)(r); _r >= REG_PMIC_IOMUX_BASE && _r < REG_PMIC_IOMUX_BASE + sizeof(*hwp_pmicIomux); })

// Set pad force inout, common to all pads.
#define PAD_INOUT_DEF(cfg) cfg.b.pad_io_oen_frc = 0, cfg.b.pad_io_oen_reg = 0, cfg.b.pad_io_out_frc = 0, cfg.b.pad_io_out_reg = 0
#define PAD_FRC_INPUT(cfg) cfg.b.pad_io_oen_frc = 1, cfg.b.pad_io_oen_reg = 1, cfg.b.pad_io_out_frc = 0, cfg.b.pad_io_out_reg = 0
#define PAD_FRC_OUTPUT(cfg) cfg.b.pad_io_oen_frc = 1, cfg.b.pad_io_oen_reg = 0, cfg.b.pad_io_out_frc = 0, cfg.b.pad_io_out_reg = 0
#define PAD_FRC_OUTVAL(cfg, v) cfg.b.pad_io_oen_frc = 1, cfg.b.pad_io_oen_reg = 0, cfg.b.pad_io_out_frc = 1, cfg.b.pad_io_out_reg = (v)

// Set pad force pull, common to all pads.
#define PAD_PULL_DEF(cfg) cfg.b.pad_io_pull_frc = 0, cfg.b.pad_io_pull_dn = 0, cfg.b.pad_io_pull_up = 0
#define PAD_FRC_PULL_NONE(cfg) cfg.b.pad_io_pull_frc = 1, cfg.b.pad_io_pull_dn = 0, cfg.b.pad_io_pull_up = 0
#define PAD_FRC_PULL_DOWN(cfg) cfg.b.pad_io_pull_frc = 1, cfg.b.pad_io_pull_dn = 1, cfg.b.pad_io_pull_up = 0
#define PAD_FRC_PULL_UP(cfg, v) cfg.b.pad_io_pull_frc = 1, cfg.b.pad_io_pull_dn = 0, cfg.b.pad_io_pull_up = (v)

// Set pad tri state, common to all pads.
#define PAD_FRC_TRI_STATE(cfg) cfg.b.pad_io_pull_frc = 1, cfg.b.pad_io_oen_reg = 1, cfg.b.pad_io_oen_frc = 1

// Refer to iomux1.h and iomux2.h
typedef union {
    uint32_t v;
    struct
    {
        uint32_t pad_io_sel : 4;          // [3:0]
        uint32_t pad_io_out_reg : 1;      // [4]
        uint32_t pad_io_out_frc : 1;      // [5]
        uint32_t pad_io_oen_reg : 1;      // [6]
        uint32_t pad_io_oen_frc : 1;      // [7]
        uint32_t pad_io_pull_up : 2;      // [9:8]
        uint32_t pad_io_pull_dn : 1;      // [10]
        uint32_t pad_io_pull_frc : 1;     // [11]
        uint32_t __13_12 : 2;             // [13:12]
        uint32_t pad_io_drv_strength : 2; // [15:14]
        uint32_t __16_16 : 1;             // [16]
        uint32_t pad_io_ie : 1;           // [17]
        uint32_t pad_io_se : 1;           // [18]
        uint32_t __31_19 : 13;            // [31:19]
    } b;
} REG_IOMUX_PAD_IO_CFG_T;

// Refer to pmic_iomux.h
typedef union {
    uint32_t v;
    struct
    {
        uint32_t pad_io_sel : 4;      // [3:0]
        uint32_t pad_io_out_reg : 1;  // [4]
        uint32_t pad_io_out_frc : 1;  // [5]
        uint32_t pad_io_oen_reg : 1;  // [6]
        uint32_t pad_io_oen_frc : 1;  // [7]
        uint32_t pad_io_pull_up : 1;  // [8]
        uint32_t pad_io_pull_dn : 1;  // [9]
        uint32_t pad_io_pull_frc : 1; // [10]
        uint32_t __31_11 : 21;        // [31:11]
    } b;
} REG_PMIC_IOMUX_PAD_IO_CFG_T;

typedef struct
{
    volatile uint32_t *reg;
} halIomuxPadProp_t;

typedef struct
{
    uint32_t suspend_regs[(sizeof(HWP_IOMUX1_T) + sizeof(HWP_IOMUX2_T)) / 4];
    halIomuxPadPsmMode_t pmic_pad_mode[PMIC_GPIO_COUNT];
    uint32_t psm_gpint_ctrl_r;
    uint32_t psm_gpint_ctrl_f;
    uint32_t psm_gpint_ctrl_mode;
    uint32_t psm_gpio_oen_val;
    uint32_t psm_iomux[PMIC_GPIO_COUNT];
} halIomuxContext_t;

static halIomuxContext_t gHalIomuxCtx;

/**
 * Set pad driving strength
 */
static bool prvSetPadDriving(const halIomuxPadProp_t *pad_prop, uint8_t driving)
{
    if (REG_IS_PMIC_IOMUX(pad_prop->reg))
    {
        REG_PMIC_PMUC_PAD_CTRL_2_T pmuc_pad_ctrl_2 = {halAdiBusRead(&hwp_pmicPmuc->pad_ctrl_2)};
        unsigned sel = ((uintptr_t)pad_prop->reg - REG_PMIC_IOMUX_BASE) / 4;
        pmuc_pad_ctrl_2.b.ibit_gpio0 &= ~(1 << sel);
        pmuc_pad_ctrl_2.b.ibit_gpio1 &= ~(1 << sel);

        switch (driving)
        {
        case 0:
            break;
        case 1:
            pmuc_pad_ctrl_2.b.ibit_gpio0 |= (1 << sel);
            break;
        case 2:
            pmuc_pad_ctrl_2.b.ibit_gpio1 |= (1 << sel);
            break;
        case 3:
            pmuc_pad_ctrl_2.b.ibit_gpio0 |= (1 << sel);
            pmuc_pad_ctrl_2.b.ibit_gpio1 |= (1 << sel);
            break;
        default:
            return false;
        }

        halAdiBusWrite(&hwp_pmicPmuc->pad_ctrl_2, pmuc_pad_ctrl_2.v);
    }
    else
    {
        if (driving > 3)
            return false;

        REG_IOMUX_PAD_IO_CFG_T cfg = {*pad_prop->reg};
        cfg.b.pad_io_drv_strength = driving;
        *pad_prop->reg = cfg.v;
    }
    return true;
}

/**
 * Set pad force pull
 */
static bool prvSetPadPull(const halIomuxPadProp_t *pad_prop, halIomuxPullType_t pull)
{
    if (REG_IS_PMIC_IOMUX(pad_prop->reg))
    {
        REG_PMIC_IOMUX_PAD_IO_CFG_T cfg = {halAdiBusRead(pad_prop->reg)};
        switch (pull)
        {
        case HAL_IOMUX_PULL_DEFAULT:
            PAD_PULL_DEF(cfg);
            break;
        case HAL_IOMUX_FORCE_PULL_NONE:
            PAD_FRC_PULL_NONE(cfg);
            break;
        case HAL_IOMUX_FORCE_PULL_DOWN:
            PAD_FRC_PULL_DOWN(cfg);
            break;
        case HAL_IOMUX_FORCE_PULL_UP:
            PAD_FRC_PULL_UP(cfg, 1);
            break;
        default:
            return false;
        }

        halAdiBusWrite(pad_prop->reg, cfg.v);
    }
    else
    {
        REG_IOMUX_PAD_IO_CFG_T cfg = {*pad_prop->reg};
        switch (pull)
        {
        case HAL_IOMUX_PULL_DEFAULT:
            PAD_PULL_DEF(cfg);
            break;
        case HAL_IOMUX_FORCE_PULL_NONE:
            PAD_FRC_PULL_NONE(cfg);
            break;
        case HAL_IOMUX_FORCE_PULL_DOWN:
            PAD_FRC_PULL_DOWN(cfg);
            break;
        case HAL_IOMUX_FORCE_PULL_UP_1:
            PAD_FRC_PULL_UP(cfg, 2); // 20K @3.3v
            break;
        case HAL_IOMUX_FORCE_PULL_UP_2:
            PAD_FRC_PULL_UP(cfg, 1); // 4.7K @3.3v
            break;
        case HAL_IOMUX_FORCE_PULL_UP_3:
            PAD_FRC_PULL_UP(cfg, 3); // 1.8K @3.3v
            break;
        default:
            return false;
        }

        *pad_prop->reg = cfg.v;
    }

    return true;
}

/**
 * Set pad force inout
 */
static bool prvSetPadInout(const halIomuxPadProp_t *pad_prop, uint8_t inout)
{
    if (REG_IS_PMIC_IOMUX(pad_prop->reg))
    {
        REG_PMIC_IOMUX_PAD_IO_CFG_T cfg = {halAdiBusRead(pad_prop->reg)};
        switch (inout)
        {
        case HAL_IOMUX_INOUT_DEFAULT:
            PAD_INOUT_DEF(cfg);
            break;
        case HAL_IOMUX_FORCE_INPUT:
            PAD_FRC_INPUT(cfg);
            break;
        case HAL_IOMUX_FORCE_OUTPUT:
            PAD_FRC_OUTPUT(cfg);
            break;
        case HAL_IOMUX_FORCE_OUTPUT_HI:
            PAD_FRC_OUTVAL(cfg, 1);
            break;
        case HAL_IOMUX_FORCE_OUTPUT_LO:
            PAD_FRC_OUTVAL(cfg, 0);
            break;
        default:
            return false;
        }

        halAdiBusWrite(pad_prop->reg, cfg.v);
    }
    else
    {
        REG_IOMUX_PAD_IO_CFG_T cfg = {*pad_prop->reg};
        switch (inout)
        {
        case HAL_IOMUX_INOUT_DEFAULT:
            PAD_INOUT_DEF(cfg);
            break;
        case HAL_IOMUX_FORCE_INPUT:
            PAD_FRC_INPUT(cfg);
            break;
        case HAL_IOMUX_FORCE_OUTPUT:
            PAD_FRC_OUTPUT(cfg);
            break;
        case HAL_IOMUX_FORCE_OUTPUT_HI:
            PAD_FRC_OUTVAL(cfg, 1);
            break;
        case HAL_IOMUX_FORCE_OUTPUT_LO:
            PAD_FRC_OUTVAL(cfg, 0);
            break;
        default:
            return false;
        }

        *pad_prop->reg = cfg.v;
    }
    return true;
}

/**
 * Set pad configuration, change to closest property if not supported
 */
static void prvSetPad(const halIomuxPadProp_t *pad_prop, uint8_t sel, uint8_t inout, halIomuxPullType_t pull, uint8_t driving)
{
    if (pad_prop->reg == NULL)
        return;

    if (driving > 3)
        driving = 3;

    if (REG_IS_PMIC_IOMUX(pad_prop->reg))
    {
        if (pull > HAL_IOMUX_FORCE_PULL_UP_1)
            pull = HAL_IOMUX_FORCE_PULL_UP_1;

        REG_PMIC_IOMUX_PAD_IO_CFG_T cfg = {halAdiBusRead(pad_prop->reg)};
        cfg.b.pad_io_sel = sel;
        halAdiBusWrite(pad_prop->reg, cfg.v);
    }
    else
    {
        REG_IOMUX_PAD_IO_CFG_T cfg = {*pad_prop->reg};
        cfg.b.pad_io_sel = sel;
        *pad_prop->reg = cfg.v;
    }

    prvSetPadInout(pad_prop, inout);
    prvSetPadPull(pad_prop, pull);
    prvSetPadDriving(pad_prop, driving);
}

/**
 * Get pad selection, -1 on no register for the pad
 */
static int prvGetPadSel(const halIomuxPadProp_t *pad_prop)
{
    if (pad_prop->reg == NULL)
        return -1;

    if (REG_IS_PMIC_IOMUX(pad_prop->reg))
    {
        REG_PMIC_IOMUX_PAD_IO_CFG_T cfg = {halAdiBusRead(pad_prop->reg)};
        return cfg.b.pad_io_sel;
    }

    REG_IOMUX_PAD_IO_CFG_T cfg = {*pad_prop->reg};
    return cfg.b.pad_io_sel;
}

/**
 * Set pad as gpio output at psm sleep
 */
static void prvSetPmicGpioPsmOutput(volatile uint32_t *iomux_reg, unsigned mask, bool out_val)
{
    halAdiBusChange(&hwp_pmicGpio->gpio_oen_val, mask, 0); // output

    REG_PMIC_IOMUX_PAD_IO_CFG_T cfg = {halAdiBusRead(iomux_reg)};
    cfg.b.pad_io_sel = 0;
    PAD_FRC_OUTVAL(cfg, out_val);
    PAD_FRC_PULL_NONE(cfg);
    halAdiBusWrite(iomux_reg, cfg.v);
}

/**
 * Set pad as gpio input at psm sleep. \p pull can only be one of
 * \p HAL_IOMUX_FORCE_PULL_UP, \p HAL_IOMUX_FORCE_PULL_DOWN or
 * \p HAL_IOMUX_FORCE_PULL_NONE.
 */
static void prvSetPmicGpioPsmInput(volatile uint32_t *iomux_reg, unsigned mask,
                                   unsigned fall, unsigned rise, unsigned pull)
{
    halAdiBusChange(&hwp_pmicGpio->gpint_ctrl_r, mask, rise);
    halAdiBusChange(&hwp_pmicGpio->gpint_ctrl_f, mask, fall);
    halAdiBusChange(&hwp_pmicGpio->gpint_ctrl_mode, mask, 0); // 0 for edge
    halAdiBusChange(&hwp_pmicGpio->gpio_oen_val, mask, mask); // input

    REG_PMIC_IOMUX_PAD_IO_CFG_T cfg = {halAdiBusRead(iomux_reg)};
    cfg.b.pad_io_sel = 0; // 0 for GPIO
    PAD_FRC_INPUT(cfg);
    if (pull == HAL_IOMUX_FORCE_PULL_UP)
        PAD_FRC_PULL_UP(cfg, 1);
    else if (pull == HAL_IOMUX_FORCE_PULL_DOWN)
        PAD_FRC_PULL_DOWN(cfg);
    else
        PAD_FRC_PULL_NONE(cfg);
    halAdiBusWrite(iomux_reg, cfg.v);
}

/**
 * Set single pad tri state
 */
static int8_t setPadTriStateSingle(int8_t n)
{
    if (n >= PMIC_GPIO_COUNT)
        return -1;

    volatile uint32_t *iomux_reg = (volatile uint32_t *)hwp_iomux1 + n;
    REG_IOMUX_PAD_IO_CFG_T cfg = {0};
    PAD_FRC_TRI_STATE(cfg);
    *iomux_reg = cfg.v;

    return 0;
}

/**
 * Set pad tri state when enter PM2, save GPIO0 and GPIO6 for wakeup
 */
static void prvSetPadTriState(void)
{
    for (int n = 0; n < PMIC_GPIO_COUNT; n++)
    {
        if ((n == 0) || (n == 6))
            continue;
        setPadTriStateSingle(n);
    }
}

/**
 * Set pmic pad tri state
 */
void halSetPmicPadTriState(void)
{
    for (int n = 0; n < PMIC_GPIO_COUNT; n++)
    {
        volatile uint32_t *iomux_reg = (volatile uint32_t *)hwp_pmicIomux + n;
        REG_PMIC_IOMUX_PAD_IO_CFG_T cfg = {0};
        PAD_FRC_TRI_STATE(cfg);
        halAdiBusWrite(iomux_reg, cfg.v);
    }
    REGT_ADI_FIELD_CHANGE(hwp_pmicPmuc->pad_ctrl_1,
                          REG_PMIC_PMUC_PAD_CTRL_1_T,
                          gpio_ie, 0);
}

/**
 * Callback before entering psm sleep
 */
void halIomuxEnterPsmSleep(void)
{
    halIomuxContext_t *d = &gHalIomuxCtx;
    d->psm_gpint_ctrl_r = halAdiBusRead(&hwp_pmicGpio->gpint_ctrl_r);
    d->psm_gpint_ctrl_f = halAdiBusRead(&hwp_pmicGpio->gpint_ctrl_f);
    d->psm_gpint_ctrl_mode = halAdiBusRead(&hwp_pmicGpio->gpint_ctrl_mode);
    d->psm_gpio_oen_val = halAdiBusRead(&hwp_pmicGpio->gpio_oen_val);

    for (int n = 0; n < PMIC_GPIO_COUNT; n++)
    {
        volatile uint32_t *iomux_reg = (volatile uint32_t *)hwp_pmicIomux + n;
        d->psm_iomux[n] = halAdiBusRead(iomux_reg);

        unsigned mask = REG_BIT(n);
        unsigned mask_one = REG_BIT(n);
        switch (d->pmic_pad_mode[n])
        {
        case HAL_IOMUX_PAD_PSM_NORMAL:
            continue;

        case HAL_IOMUX_PAD_PSM_NOT_USE:
            prvSetPmicGpioPsmInput(iomux_reg, mask, 0, 0, HAL_IOMUX_FORCE_PULL_NONE);
            break;

        case HAL_IOMUX_PAD_PSM_OUT_KEEP:
            prvSetPmicGpioPsmOutput(iomux_reg, mask, hwp_gpio1->gpio_val & mask ? 1 : 0);
            break;

        case HAL_IOMUX_PAD_PSM_OUT_HI:
            prvSetPmicGpioPsmOutput(iomux_reg, mask, 1);
            break;

        case HAL_IOMUX_PAD_PSM_OUT_LO:
            prvSetPmicGpioPsmOutput(iomux_reg, mask, 0);
            break;

        case HAL_IOMUX_PAD_PSM_WAKE_FALL_EDGE:
            prvSetPmicGpioPsmInput(iomux_reg, mask, mask_one, 0, HAL_IOMUX_FORCE_PULL_UP);
            break;

        case HAL_IOMUX_PAD_PSM_WAKE_RISE_EDGE:
            prvSetPmicGpioPsmInput(iomux_reg, mask, 0, mask_one, HAL_IOMUX_FORCE_PULL_DOWN);
            break;

        case HAL_IOMUX_PAD_PSM_WAKE_ANY_EDGE:
            prvSetPmicGpioPsmInput(iomux_reg, mask, mask_one, mask_one, HAL_IOMUX_FORCE_PULL_NONE);
            break;

        case HAL_IOMUX_PAD_PSM_WAKE_FALL_EDGE_NO_PULLUP:
            prvSetPmicGpioPsmInput(iomux_reg, mask, mask_one, 0, HAL_IOMUX_FORCE_PULL_NONE);
            break;

        case HAL_IOMUX_PAD_PSM_WAKE_RISE_EDGE_NO_PULLDOWN:
            prvSetPmicGpioPsmInput(iomux_reg, mask, 0, mask_one, HAL_IOMUX_FORCE_PULL_NONE);
            break;
        }
    }
    REGT_ADI_FIELD_CHANGE(hwp_pmicPmuc->pad_ctrl_1,
                          REG_PMIC_PMUC_PAD_CTRL_1_T,
                          gpio_ie, 0xff);
}

/**
 * Callback when psm sleep failed
 */
void halIomuxAbortPsmSleep(void)
{
    REGT_ADI_FIELD_CHANGE(hwp_pmicPmuc->pad_ctrl_1,
                          REG_PMIC_PMUC_PAD_CTRL_1_T,
                          gpio_ie, 0x40);
    halIomuxContext_t *d = &gHalIomuxCtx;
    halAdiBusWrite(&hwp_pmicGpio->gpint_ctrl_r, 0);
    halAdiBusWrite(&hwp_pmicGpio->gpint_ctrl_f, 0);

    for (int n = 0; n < PMIC_GPIO_COUNT; n++)
    {
        volatile uint32_t *iomux_reg = (volatile uint32_t *)hwp_pmicIomux + n;
        halAdiBusWrite(iomux_reg, d->psm_iomux[n]);
    }

    halAdiBusWrite(&hwp_pmicGpio->gpint_ctrl_mode, d->psm_gpint_ctrl_mode);
    halAdiBusWrite(&hwp_pmicGpio->gpint_ctrl_r, d->psm_gpint_ctrl_r);
    halAdiBusWrite(&hwp_pmicGpio->gpint_ctrl_f, d->psm_gpint_ctrl_f);
    halAdiBusWrite(&hwp_pmicGpio->gpio_oen_val, d->psm_gpio_oen_val);
}

/**
 * Suspend callback.
 */
void halIomuxSuspend(osiSuspendMode_t mode)
{
    if (mode == OSI_SUSPEND_PM2)
    {
        halIomuxContext_t *d = &gHalIomuxCtx;
        volatile uint32_t *reg = (volatile uint32_t *)hwp_iomux1;
        for (int n = 0; n < (sizeof(HWP_IOMUX1_T) + sizeof(HWP_IOMUX2_T)) / 4; n++)
        {
            if (n == sizeof(HWP_IOMUX1_T) / 4)
                reg = (volatile uint32_t *)hwp_iomux2;
            d->suspend_regs[n] = *reg++;
        }
        prvSetPadTriState();
    }
}

/**
 * Resume callback.
 */
void halIomuxResume(osiSuspendMode_t mode, uint32_t source)
{
    if (source & OSI_RESUME_ABORT)
    {
        halIomuxContext_t *d = &gHalIomuxCtx;
        volatile uint32_t *reg = (volatile uint32_t *)hwp_iomux1;
        for (int n = 0; n < sizeof(HWP_IOMUX1_T) / 4; n++)
            *reg++ = d->suspend_regs[n];
        return;
    }

    if (mode == OSI_SUSPEND_PM2)
    {
        halIomuxContext_t *d = &gHalIomuxCtx;
        volatile uint32_t *reg = (volatile uint32_t *)hwp_iomux1;
        for (int n = 0; n < (sizeof(HWP_IOMUX1_T) + sizeof(HWP_IOMUX2_T)) / 4; n++)
        {
            if (n == sizeof(HWP_IOMUX1_T) / 4)
                reg = (volatile uint32_t *)hwp_iomux2;
            *reg++ = d->suspend_regs[n];
        }
    }
}

/**
 * Set pad psm sleep mode.
 */
bool halIomuxSetPadPsmMode(unsigned fun_pad, halIomuxPadPsmMode_t mode)
{
    unsigned pad = PAD_INDEX(fun_pad);
    if (!(pad >= PAD_INDEX(HAL_IOMUX_PAD_PMIC_GPIO_0) &&
          pad <= PAD_INDEX(HAL_IOMUX_PAD_PMIC_GPIO_7)))
        return false;

    halIomuxContext_t *d = &gHalIomuxCtx;
    d->pmic_pad_mode[pad - PAD_INDEX(HAL_IOMUX_PAD_PMIC_GPIO_0)] = mode;
    return true;
}

#endif
