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

// This will link an arm elf for uimage. The definitions needed:
// * RAMRUN_IMAGE_START: The sram start address, including uimage header.
// * RAMRUN_IMAGE_SIZE: The sram size.
// * RAMRUN_FLASH_SIZE: The maximum romable size. When defined, it is only
//      used to check romable size.
// * CONFIG_SIMAGE_HEADER_SIZE: uimage header size.
//
// Address between [__flash_start, __flash_end] is romable.

#include "link_defs.ld"
#include "hal_config.h"
#include "boot_config.h"

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(bootEntry)
EXTERN(bootEntry)

#define text1_contents      *(BOOT_ENTRY) \
                            SORT(*)(.sramboottext .sramboottext.*) \
                            SORT(*)(.sramtext .sramtext.*) \
                            SORT(*)(.ramtext .ramtext.*)

#define text2_contents	    SORT(*)(.ramtext .ramtext.*) \
                            SORT(*)(DEF_RO)
#define data_contents       SORT(*)(.sramdata .sramdata.*) \
                            SORT(*)(DEF_RW)
#define bss_contents        *(.srambss .srambss.*) \
                            *(DEF_ZI)
#define warmboot_contents   *(WARMBOOTENTRY) \
                            SORT(*)(.wbramtext .wbramtext.*)

MEMORY {
    syssram(rwx): ORIGIN = CONFIG_SRAM_PHY_ADDRESS, LENGTH = CONFIG_SRAM_SIZE
    aonsram(rwx): ORIGIN = CONFIG_AON_SRAM_PHY_ADDRESS, LENGTH = CONFIG_AON_SRAM_SIZE
    aonsram1(rwx): ORIGIN = CONFIG_AON_SRAM_PHY_ADDRESS, LENGTH = 0x1000
    aonsramwarmboot(rwx): ORIGIN = CONFIG_AON_SRAM_PHY_ADDRESS + 0x1000, LENGTH = 0xa80
    aonsram2(rwx): ORIGIN = CONFIG_AON_SRAM_PHY_ADDRESS + 0x1a80, LENGTH = CONFIG_AON_SRAM_SIZE - 0x1a80
    ram(rw):      ORIGIN = CONFIG_RAM_PHY_ADDRESS, LENGTH = CONFIG_RAM_SIZE
}

SECTIONS {
    NOLOAD_SIZE_SECT(rom_data, CONFIG_ROM_SRAM_DATA_SIZE) > syssram
    NOLOAD_SIZE_SECT(simage, CONFIG_SIMAGE_HEADER_SIZE) > aonsram1

    ANCHOR_SECT(flash_start) > aonsram1
    NOAT_SECT(text1) > aonsram1
    NOAT_SECT(warmboot) > aonsramwarmboot
    NOAT_SECT(text2) > aonsram2
    NOAT_SECT(data) > aonsram2
    ANCHOR_SECT(flash_end) > aonsram2
    NOLOAD_SECT(bss) > aonsram2
#ifdef RAMRUN_MMU_ON_SRAM
    NOLOAD_END_SECT(aonsram_skip, ORIGIN(aonsram) + LENGTH(aonsram) - 0x4000) > aonsram2
    NOLOAD_SIZE_SECT(mmu_ttbl1, 0x4000) > aonsram2
#endif
    NOLOAD_SIZE_SECT(sys_stack, CONFIG_BOOT_SYS_STACK_SIZE) > syssram
    NOLOAD_SIZE_SECT(irq_stack, CONFIG_BOOT_IRQ_STACK_SIZE) > syssram
    NOLOAD_SIZE_SECT(blue_screen, CONFIG_BLUE_SCREEN_SIZE) > syssram
    NOLOAD_END_SECT(sram_heap, ORIGIN(syssram) + LENGTH(syssram)) > syssram
#ifndef RAMRUN_MMU_ON_SRAM
    NOLOAD_SIZE_SECT(mmu_ttbl1, 0x4000) > ram
#endif
    NOLOAD_END_SECT(ram_heap, ORIGIN(ram) + LENGTH(ram)) > ram

    IGNORE_DISCARD_SECTS
}