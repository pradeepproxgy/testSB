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

configure_file(include/hal_revision.h.in ${out_inc_dir}/hal_revision.h)
configure_file(include/hal_config.h.in ${out_inc_dir}/hal_config.h)
configure_file(include/${CONFIG_SOC}/hal_chip_config.h.in ${out_inc_dir}/hal_chip_config.h)

set(hal_spi_flash_prop_csv ${CMAKE_CURRENT_SOURCE_DIR}/src/hal_spi_flash_prop.csv)
set(hal_spi_flash_prop_h ${CMAKE_CURRENT_BINARY_DIR}/hal_spi_flash_prop.h)
add_custom_command(OUTPUT ${hal_spi_flash_prop_h}
    COMMAND python3 ${norpropgen_py} ${hal_spi_flash_prop_csv} ${hal_spi_flash_prop_h}
    DEPENDS ${norpropgen_py} ${hal_spi_flash_prop_csv}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

if(NOT ${CONFIG_PINMUX_JSON_PATH} STREQUAL "")
    # Put the definition in respository make it easier to read codes
    set(pin_def_h ${CMAKE_CURRENT_SOURCE_DIR}/include/${CONFIG_SOC}/hal_iomux_pin.h)
    set(pinmux_json ${SOURCE_TOP_DIR}/${CONFIG_PINMUX_JSON_PATH})
    set(iomux_prop_h ${CMAKE_CURRENT_BINARY_DIR}/hal_iomux_prop.h)
    set(iomux_prop_h_in ${CMAKE_CURRENT_SOURCE_DIR}/src/hal_iomux_prop.h.in)
    set(pinmux_csv ${CMAKE_CURRENT_SOURCE_DIR}/config/${CONFIG_SOC}/pinmux_${CONFIG_SOC}.csv)
    add_custom_command(OUTPUT ${iomux_prop_h}
        COMMAND python3 ${iomuxgen_py} --chip ${CONFIG_SOC}
            ${pinmux_json} ${iomux_prop_h_in} ${iomux_prop_h} # ${pin_def_h}
        DEPENDS ${chip_iomuxgen_py} ${pinmux_json} ${pinmux_csv} ${iomux_prop_h_in}
    )
endif()

set(target hal)
include(hal.cmake)

set(target hal_open)
add_app_libraries($<TARGET_FILE:${target}>)
add_library(${target} STATIC 
    src/8850/hal_clock_init.c
    src/8850/hal_pmu.c
    src/hal_spi_flash.c
    ${hal_spi_flash_prop_h}
)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
target_include_directories(${target} PUBLIC include)
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_targets(${target} PUBLIC kernel)
target_include_targets(${target} PRIVATE calclib fs nvm driver service)