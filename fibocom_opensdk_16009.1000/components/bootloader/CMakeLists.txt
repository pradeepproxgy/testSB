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

configure_file(include/boot_config.h.in ${out_inc_dir}/boot_config.h)
install_headers(include/image_header.h OUTPUT ${out_inc_dir})

if(CONFIG_SOC_8850)
    set(target fibo_boot_cfg)
    set (LIB_SOURCE_FILE
        fibo_boot_config/src/fibo_boot_cfg.c
    )

    add_library(${target} STATIC ${LIB_SOURCE_FILE})
    set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
    target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_BOOT)
    target_include_directories(${target} PUBLIC include fibo_boot_config/inc)
    target_link_libraries(${target} PRIVATE calclib boot_calclib_crc)
    target_include_targets(${target} PRIVATE hal fs sffs bdev driver fsmount 
        nvm cpio_parser hal_open)
endif()

include(bootloader.cmake)

include(img.cmake)
