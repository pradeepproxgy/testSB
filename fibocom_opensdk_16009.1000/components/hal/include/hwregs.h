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

#ifndef _HWREGS_H_
#define _HWREGS_H_

#include <stdint.h>
#include "hal_config.h"
#include "hwreg_access.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_SOC_8910
#include "8910/hal_irq_priority.h"
#include "hwregs/8910/globals.h"
#include "hwregs/8910/gallite_generic_config.h"
#include "hwregs/8910/adi_mst.h"
#include "hwregs/8910/analog_reg.h"
#include "hwregs/8910/aon_ifc.h"
#include "hwregs/8910/ap_ifc.h"
#include "hwregs/8910/arm_uart.h"
#include "hwregs/8910/arm_axidma.h"
#include "hwregs/8910/cp_clkrst.h"
#include "hwregs/8910/cp_idle.h"
#include "hwregs/8910/cp_mailbox.h"
#include "hwregs/8910/cp_pwrctrl.h"
#include "hwregs/8910/cp_sysreg.h"
#include "hwregs/8910/debug_uart.h"
#include "hwregs/8910/camera.h"
#include "hwregs/8910/debug_host.h"
#include "hwregs/8910/dmc400.h"
#include "hwregs/8910/efuse_ctrl.h"
#include "hwregs/8910/gic400_reg.h"
#include "hwregs/8910/gpio.h"
#include "hwregs/8910/i2c_master.h"
#include "hwregs/8910/spi.h"
#include "hwregs/8910/pwm.h"
#include "hwregs/8910/iomux.h"
#include "hwregs/8910/keypad.h"
#include "hwregs/8910/lpddr_phy.h"
#include "hwregs/8910/lzma.h"
#include "hwregs/8910/psram_phy.h"
#include "hwregs/8910/rf_registers.h"
#include "hwregs/8910/rf_rtc.h"
#include "hwregs/8910/spinlock.h"
#include "hwregs/8910/spi_flash.h"
#include "hwregs/8910/sys_ctrl.h"
#include "hwregs/8910/sys_ifc.h"
#include "hwregs/8910/timer.h"
#include "hwregs/8910/timer_ap.h"
#include "hwregs/8910/rda2720m_adc.h"
#include "hwregs/8910/rda2720m_efs.h"
#include "hwregs/8910/rda2720m_eic.h"
#include "hwregs/8910/rda2720m_global.h"
#include "hwregs/8910/rda2720m_int.h"
#include "hwregs/8910/rda2720m_rtc.h"
#include "hwregs/8910/rda2720m_psm.h"
#include "hwregs/8910/rda2720m_wdg.h"
#include "hwregs/8910/rda2720m_bltc.h"
#include "hwregs/8910/rda2720m_aud_codec.h"
#include "hwregs/8910/rda2720m_aud.h"
#include "hwregs/8910/aif.h"
#include "hwregs/8910/aud_2ad.h"
#include "hwregs/8910/sdmmc.h"
#include "hwregs/8910/gouda.h"
#include "hwregs/8910/usbc.h"
#include "hwregs/8910/wcn_rf_if.h"
#include "hwregs/8910/wcn_wlan.h"
#include "hwregs/8910/wcn_bt_modem.h"
#include "hwregs/8910/wcn_bt_link.h"
#include "hwregs/8910/wcn_comregs.h"
#include "hwregs/8910/wcn_sys_ctrl.h"
#include "hwregs/8910/aes.h"
#endif

#ifdef CONFIG_SOC_8811
#include "8811/hal_irq_priority.h"
#include "hwregs/8811/globals.h"
#include "hwregs/8811/config.h"
#include "hwregs/8811/adi_mst.h"
#include "hwregs/8811/bb_ctrl.h"
#include "hwregs/8811/bb_ifc.h"
#include "hwregs/8811/ce_pub_top.h"
#include "hwregs/8811/ce_sec_top.h"
#include "hwregs/8811/debug_host_internals.h"
#include "hwregs/8811/debug_host.h"
#include "hwregs/8811/debug_uart.h"
#include "hwregs/8811/dfe.h"
#include "hwregs/8811/dma.h"
#include "hwregs/8811/efuse.h"
#include "hwregs/8811/f8.h"
#include "hwregs/8811/gpio.h"
#include "hwregs/8811/gpio1.h"
#include "hwregs/8811/gpt_lite.h"
#include "hwregs/8811/gpt.h"
#include "hwregs/8811/i2c_master.h"
#include "hwregs/8811/i2s.h"
#include "hwregs/8811/iomux1.h"
#include "hwregs/8811/iomux2.h"
#include "hwregs/8811/keypad.h"
#include "hwregs/8811/lps.h"
#include "hwregs/8811/master_ctrl_top_rf.h"
#include "hwregs/8811/med.h"
#include "hwregs/8811/mem_fw_bb_nbiot_top_rf.h"
#include "hwregs/8811/mem_fw_flash1_rf.h"
#include "hwregs/8811/mem_fw_flash2_rf.h"
#include "hwregs/8811/mem_fw_psram_rf.h"
#include "hwregs/8811/mem_fw_sys_ram0_rf.h"
#include "hwregs/8811/mem_fw_sys_ram1_rf.h"
#include "hwregs/8811/mem_fw_sys_ram2_rf.h"
#include "hwregs/8811/mem_fw_sys_ram3_rf.h"
#include "hwregs/8811/nb_acc.h"
#include "hwregs/8811/nb_ca_rx_dump.h"
#include "hwregs/8811/nb_ca_tx_dump.h"
#include "hwregs/8811/nb_cell_search.h"
#include "hwregs/8811/nb_common.h"
#include "hwregs/8811/nb_ctrl.h"
#include "hwregs/8811/nb_ds_bsel.h"
#include "hwregs/8811/nb_fft_rsrp.h"
#include "hwregs/8811/nb_intc.h"
#include "hwregs/8811/nb_locseq_gen.h"
#include "hwregs/8811/nb_meas.h"
#include "hwregs/8811/nb_sp.h"
#include "hwregs/8811/nb_tx_chsc.h"
#include "hwregs/8811/nb_tx_frontend.h"
#include "hwregs/8811/nb_tx_mdd.h"
#include "hwregs/8811/nb_tx_pusch_encoder.h"
#include "hwregs/8811/nb_viterbi.h"
#include "hwregs/8811/pmuc.h"
#include "hwregs/8811/psram8_ctrl.h"
#include "hwregs/8811/rf_dig.h"
#include "hwregs/8811/rf_if.h"
#include "hwregs/8811/rf_spi.h"
#include "hwregs/8811/rffe_reg.h"
#include "hwregs/8811/rom_patch.h"
#include "hwregs/8811/sci.h"
#include "hwregs/8811/sdmmc.h"
#include "hwregs/8811/sdmmc2.h"
#include "hwregs/8811/seg_lcd.h"
#include "hwregs/8811/slv_fw_bbifc_ahb_rf.h"
#include "hwregs/8811/slv_fw_bbifc_apb_rf.h"
#include "hwregs/8811/slv_fw_sysifc1_apb_rf.h"
#include "hwregs/8811/slv_fw_sysifc2_ahb_rf.h"
#include "hwregs/8811/slv_fw_sysifc2_apb_rf.h"
#include "hwregs/8811/spi_flash.h"
#include "hwregs/8811/spi.h"
#include "hwregs/8811/sys_ctrl.h"
#include "hwregs/8811/sys_ifc1.h"
#include "hwregs/8811/sys_ifc2.h"
#include "hwregs/8811/sys_mem.h"
#include "hwregs/8811/tcu.h"
#include "hwregs/8811/timer.h"
#include "hwregs/8811/uart.h"
#include "hwregs/8811/wdt.h"
#include "hwregs/8811/globals_1811.h"
#include "hwregs/8811/config_1811.h"
#include "hwregs/8811/pmic_adc.h"
#include "hwregs/8811/pmic_apb_efs.h"
#include "hwregs/8811/pmic_dig.h"
#include "hwregs/8811/pmic_gpio_lite.h"
#include "hwregs/8811/pmic_gpt16.h"
#include "hwregs/8811/pmic_intc.h"
#include "hwregs/8811/pmic_iomux.h"
#include "hwregs/8811/pmic_pmuc.h"
#include "hwregs/8811/pmic_rtc_timer.h"
#include "hwregs/8811/pmic_wdt16.h"
#include "hwregs/8811/psram8_ctrl.h"
#endif

#ifdef CONFIG_SOC_8821
#include "8821/hal_irq_priority.h"
#include "hwregs/8821/globals.h"
#include "hwregs/8821/config.h"
#include "hwregs/8821/adi_mst.h"
#include "hwregs/8821/bb_ctrl.h"
#include "hwregs/8821/bb_ifc.h"
#include "hwregs/8821/ce_pub_top.h"
#include "hwregs/8821/ce_sec_top.h"
#include "hwregs/8821/debug_host_internals.h"
#include "hwregs/8821/debug_host.h"
#include "hwregs/8821/debug_uart.h"
#include "hwregs/8821/dfe.h"
#include "hwregs/8821/dma.h"
#include "hwregs/8821/efuse.h"
#include "hwregs/8821/f8.h"
#include "hwregs/8821/gpio.h"
#include "hwregs/8821/gpio1.h"
#include "hwregs/8821/gpt_lite.h"
#include "hwregs/8821/gpt.h"
#include "hwregs/8821/i2c_master.h"
#include "hwregs/8821/i2s.h"
#include "hwregs/8821/iomux1.h"
#include "hwregs/8821/iomux2.h"
#include "hwregs/8821/keypad.h"
#include "hwregs/8821/lps.h"
#include "hwregs/8821/master_ctrl_top_rf.h"
#include "hwregs/8821/med.h"
#include "hwregs/8821/mem_fw_bb_nbiot_top_rf.h"
#include "hwregs/8821/mem_fw_flash1_rf.h"
#include "hwregs/8821/mem_fw_flash2_rf.h"
#include "hwregs/8821/mem_fw_psram_rf.h"
#include "hwregs/8821/mem_fw_sys_ram0_rf.h"
#include "hwregs/8821/mem_fw_sys_ram1_rf.h"
#include "hwregs/8821/mem_fw_sys_ram2_rf.h"
#include "hwregs/8821/mem_fw_sys_ram3_rf.h"
#include "hwregs/8821/nb_acc.h"
#include "hwregs/8821/nb_ca_rx_dump.h"
#include "hwregs/8821/nb_ca_tx_dump.h"
#include "hwregs/8821/nb_cell_search.h"
#include "hwregs/8821/nb_common.h"
#include "hwregs/8821/nb_ctrl.h"
#include "hwregs/8821/nb_ds_bsel.h"
#include "hwregs/8821/nb_fft_rsrp.h"
#include "hwregs/8821/nb_intc.h"
#include "hwregs/8821/nb_locseq_gen.h"
#include "hwregs/8821/nb_meas.h"
#include "hwregs/8821/nb_sp.h"
#include "hwregs/8821/nb_tx_chsc.h"
#include "hwregs/8821/nb_tx_frontend.h"
#include "hwregs/8821/nb_tx_mdd.h"
#include "hwregs/8821/nb_tx_pusch_encoder.h"
#include "hwregs/8821/nb_viterbi.h"
#include "hwregs/8821/pmuc.h"
#include "hwregs/8821/psram8_ctrl.h"
#include "hwregs/8821/rf_dig.h"
#include "hwregs/8821/rf_if.h"
#include "hwregs/8821/rf_spi.h"
#include "hwregs/8821/rffe_reg.h"
#include "hwregs/8821/rom_patch.h"
#include "hwregs/8821/sci.h"
#include "hwregs/8821/sdmmc.h"
#include "hwregs/8821/sdmmc2.h"
#include "hwregs/8821/seg_lcd.h"
#include "hwregs/8821/slv_fw_bbifc_ahb_rf.h"
#include "hwregs/8821/slv_fw_bbifc_apb_rf.h"
#include "hwregs/8821/slv_fw_sysifc1_apb_rf.h"
#include "hwregs/8821/slv_fw_sysifc2_ahb_rf.h"
#include "hwregs/8821/slv_fw_sysifc2_apb_rf.h"
#include "hwregs/8821/spi_flash.h"
#include "hwregs/8821/spi.h"
#include "hwregs/8821/sys_ctrl.h"
#include "hwregs/8821/sys_ifc1.h"
#include "hwregs/8821/sys_ifc2.h"
#include "hwregs/8821/sys_mem.h"
#include "hwregs/8821/tcu.h"
#include "hwregs/8821/timer.h"
#include "hwregs/8821/uart.h"
#include "hwregs/8821/wdt.h"
#include "hwregs/8821/globals_1811.h"
#include "hwregs/8821/config_1811.h"
#include "hwregs/8821/pmic_adc.h"
#include "hwregs/8821/pmic_apb_efs.h"
#include "hwregs/8821/pmic_dig.h"
#include "hwregs/8821/pmic_gpio_lite.h"
#include "hwregs/8821/pmic_gpt16.h"
#include "hwregs/8821/pmic_intc.h"
#include "hwregs/8821/pmic_iomux.h"
#include "hwregs/8821/pmic_pmuc.h"
#include "hwregs/8821/pmic_rtc_timer.h"
#include "hwregs/8821/pmic_wdt16.h"
#include "hwregs/8821/psram8_ctrl.h"
#endif

#ifdef CONFIG_SOC_8850
#include "8850/hal_irq_priority.h"
#include "hwregs/8850/cp_glb.h"
#include "hwregs/8850/gnss_sys.h"
#include "hwregs/8850/adi_mst.h"
#include "hwregs/8850/aon_ifc.h"
#include "hwregs/8850/ap_clk.h"
#include "hwregs/8850/ap_apb.h"
#include "hwregs/8850/ap_spi.h"
#include "hwregs/8850/ap_ifc.h"
#include "hwregs/8850/pub_apb.h"
#include "hwregs/8850/aif.h"
#include "hwregs/8850/aud_2ad.h"
#include "hwregs/8850/ap_clk.h"
#include "hwregs/8850/ce_sec.h"
#include "hwregs/8850/debug_host.h"
#include "hwregs/8850/dmc400.h"
#include "hwregs/8850/efuse.h"
#include "hwregs/8850/emmc.h"
#include "hwregs/8850/sdmmc.h"
#include "hwregs/8850/gic400_reg.h"
#include "hwregs/8850/globals.h"
#include "hwregs/8850/idle_lps.h"
#include "hwregs/8850/iomux.h"
#include "hwregs/8850/aon_spi.h"
#include "hwregs/8850/lps_apb.h"
#include "hwregs/8850/lps_ifc.h"
#include "hwregs/8850/lzma.h"
#include "hwregs/8850/med.h"
#include "hwregs/8850/pub_apb.h"
#include "hwregs/8850/mem_fw_aon_imem.h"
#include "hwregs/8850/mem_fw_ap_imem.h"
#include "hwregs/8850/mem_fw_spiflash1.h"
#include "hwregs/8850/mst_ctrl_ap.h"
#include "hwregs/8850/psram_phy.h"
#include "hwregs/8850/pwrctrl.h"
#include "hwregs/8850/slv_fw_ap_ahb.h"
#include "hwregs/8850/slv_fw_ap_ifc.h"
#include "hwregs/8850/slv_fw_lps_ifc.h"
#include "hwregs/8850/gpt.h"
#include "hwregs/8850/gpt_lite.h"
#include "hwregs/8850/spi_flash.h"
#include "hwregs/8850/spinlock.h"
#include "hwregs/8850/sys_ctrl.h"
#include "hwregs/8850/rf_sysctrl.h"
#include "hwregs/8850/sysmail.h"
#include "hwregs/8850/timer_ap.h"
#include "hwregs/8850/timer.h"
#include "hwregs/8850/uart.h"
#include "hwregs/8850/ap_apb.h"
#include "hwregs/8850/gouda.h"
#include "hwregs/8850/aon_clk_gen.h"
#include "hwregs/8850/aon_clk.h"
#include "hwregs/8850/pmic_rtc.h"
#include "hwregs/8850/pmic_eic.h"
#include "hwregs/8850/pmic_int.h"
#include "hwregs/8850/pmic_ana.h"
#include "hwregs/8850/pmic_rtc_ana.h"
#include "hwregs/8850/pmic_wdt.h"
#include "hwregs/8850/pmic_psm.h"
#include "hwregs/8850/pmic_bltc.h"
#include "hwregs/8850/pmic_efuse.h"
#include "hwregs/8850/lps_clk.h"
#include "hwregs/8850/monitor.h"
#include "hwregs/8850/analog_g1.h"
#include "hwregs/8850/analog_g2.h"
#include "hwregs/8850/analog_g3.h"
#include "hwregs/8850/i2c_master.h"
#include "hwregs/8850/camera.h"
#include "hwregs/8850/keypad.h"
#include "hwregs/8850/arm_axidma.h"
// #include "hwregs/8850/usbc.h"
#include "hwregs/8850/musb_regs.h"
#include "hwregs/8850/pmic_adc.h"
#include "hwregs/8850/gpio.h"
#include "hwregs/8850/pmic_bltc.h"
#include "hwregs/8850/pmic_psm.h"
#include "hwregs/8850/pmic_pin_reg.h"
#include "hwregs/8850/globals.h"
#include "hwregs/8850/debug_uart.h"
#include "hwregs/8850/gpio.h"
#include "hwregs/8850/rffe.h"
#include "hwregs/8850/rda2720m_adc.h"
#include "hwregs/8850/rda2720m_efs.h"
#include "hwregs/8850/rda2720m_eic.h"
#include "hwregs/8850/rda2720m_global.h"
#include "hwregs/8850/rda2720m_int.h"
#include "hwregs/8850/rda2720m_rtc.h"
#include "hwregs/8850/rda2720m_psm.h"
#include "hwregs/8850/rda2720m_wdg.h"
#include "hwregs/8850/rda2720m_bltc.h"
#include "hwregs/8850/rda2720m_aud_codec.h"
#include "hwregs/8850/rda2720m_aud.h"
#include "hwregs/8850/idle_timer.h"
#include "hwregs/8850/busmon.h"
#include "hwregs/8850/rf_dig_rtc.h"
#include "hwregs/8850/sci.h"
#endif

#ifdef __cplusplus
}
#endif
#endif
