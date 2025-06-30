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

configure_file(include/kernel_config.h.in ${out_inc_dir}/kernel_config.h)

set(target fibo_kernel)
add_app_libraries($<TARGET_FILE:${target}>)

add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_KERNEL)
target_include_directories(${target} PUBLIC include)
target_include_directories(${target} PRIVATE freertos/include src)
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_targets(${target} PRIVATE hal atr driver fs service nanopb)
target_include_targets_if(CONFIG_TRUSTZONE_SUPPORT THEN ${target} PRIVATE trusty_core)

target_sources_if(CONFIG_SOC_8850 THEN ${target} PRIVATE
    chip/8850/osi_boot.c
)
set_if(chipdir CONFIG_SOC_8850 THEN chip chip/8850 hal/src/8850)

include(kernel.cmake)
include(core.cmake)

set(target os_lib)
include(osi_lib.cmake)
