# Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
# All rights reserved.
#
# This software is supplied "AS IS" without any warranties.
# RDA assumes no responsibility or liability for the use of the software,
# conveys no license or title under any patent, copyright, or mask work
# right to the product. RDA reserves the right to make changes in the
# software without notification.  RDA also make no representation or
# warranty that such application will be suitable for the specified use
# without further testing or modification.

set(rmarun_ld ldscripts/ramrun_8850.ld)
set(psrmarun_ld ldscripts/psramrun_8850.ld)

if(CONFIG_FIBOCOM_SUPPORT_TWO_SPL)
    set(target spl2)
    set(spl2_ld ldscripts/psramrun_8850_spl.ld)
    add_simage(${target} ${spl2_ld} 8850/spl2_start.c)
    target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_BOOT)
    target_link_whole_archive(${target} PRIVATE bootloader)
    target_link_group(${target} PRIVATE hal calclib boot_calclib_crc fupdate fupdate_security_boot sffs bdev bdev_core osi_lib fsmount driver hal_open fibo_fsmount)
    target_include_targets(${target} PRIVATE kernel fs sffs bdev fupdate fupdate_security_boot fsmount driver)
    target_link_libraries(${target} PRIVATE ${libc_file_name} ${libgcc_file_name})
    target_include_directories(${target} PUBLIC ../../fibocom/api include ../../fibocom/fibo-framework/lib/include)
    target_link_libraries(${target} PRIVATE fibo_boot_logo)
    set(spl2_img ${out_hex_dir}/spl2.img)
    set(spl2_lzma ${spl2_img}.lzma)
    add_custom_command(OUTPUT ${spl2_lzma}
        COMMAND ${cmd_lzmacomp} ${spl2_img}
        DEPENDS ${spl2_img}
    )
endif()

set(target spl)
add_simage(${target} ${rmarun_ld} 8850/spl_start.c)
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_BOOT)
target_link_whole_archive(${target} PRIVATE bootloader fibo_boot_cfg)
if(CONFIG_FIBOCOM_BASE)
    if(CONFIG_LCD_SUPPORT AND CONFIG_FIBOCOM_FW_UPDATE_LOGO_SUPPORT)
        target_sources(${target} PRIVATE ../../cust_app/src/fibo_logo_config.c)
        target_link_libraries(${target} PRIVATE fibo_boot_logo lcdpanel)
        target_include_directories(${target} PRIVATE ../fibo_logo)
    endif()
endif()
target_link_group(${target} PRIVATE hal calclib boot_calclib_crc fupdate fupdate_security_boot sffs bdev bdev_core osi_lib fsmount driver hal_open fibo_fsmount)
target_include_targets(${target} PRIVATE kernel fs sffs bdev fupdate fupdate_security_boot fsmount driver)
target_link_libraries(${target} PRIVATE ${libc_file_name} ${libgcc_file_name})
target_include_directories(${target} PUBLIC ../../fibocom/api)

if(CONFIG_FIBOCOM_SUPPORT_TWO_SPL)
    target_incbin_size(${target} ${spl2_lzma} gBoot2CompData gBoot2CompSize BALIGN 8)
endif()

if(CONFIG_TRUSTZONE_SUPPORT)
    set(rmarun_ld ldscripts/ramrun_8850.ld)
endif()
set(target boot)
add_simage(${target} ${psrmarun_ld} 8850/boot_start.c)
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_BOOT)
target_link_whole_archive(${target} PRIVATE bootloader fibo_boot_cfg)
target_link_group(${target} PRIVATE hal hal_open calclib boot_calclib_crc osi_lib)
target_include_targets(${target} PRIVATE kernel fs sffs bdev fupdate fsmount driver)
target_link_libraries(${target} PRIVATE ${libc_file_name} ${libgcc_file_name})

set(target norfdl)
add_simage(${target} ${rmarun_ld} 8850/fdl.c)
target_compile_definitions(${target}_ldscript PRIVATE
    RAMRUN_IMAGE_START=${CONFIG_NORFDL_IMAGE_START}
    RAMRUN_IMAGE_SIZE=${CONFIG_NORFDL_IMAGE_SIZE}
)
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_BOOT)
target_include_targets(${target} PRIVATE fs fsmount nvm calclib cpio_parser driver)
target_link_whole_archive(${target} PRIVATE bootloader cpio_parser bdev_core fibo_boot_cfg)
target_link_group(${target} PRIVATE hal calclib boot_calclib_crc sffs fsmount fibo_fsmount bdev osi_lib nvm lz4 fs hal_open)
target_link_libraries(${target} PRIVATE ${libc_file_name} ${libgcc_file_name} driver)
