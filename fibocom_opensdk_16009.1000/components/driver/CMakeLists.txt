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

configure_file(include/drv_config.h.in ${out_inc_dir}/drv_config.h)

if(NOT ${CONFIG_KEYPAD_CSV_PATH} STREQUAL "")
    set(keypadcfg_h ${CMAKE_CURRENT_BINARY_DIR}/drv_keypad_def.h)
    set(csv_keypad ${SOURCE_TOP_DIR}/${CONFIG_KEYPAD_CSV_PATH})
    add_custom_command(OUTPUT ${keypadcfg_h}
        COMMAND python3 ${keypadgen_py} ${csv_keypad} ${keypadcfg_h}
        DEPENDS ${keypadgen_py} ${csv_keypad}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
file(GLOB_RECURSE INCLUDE_FILE_LIST "include/*")
install_headers(${INCLUDE_FILE_LIST} OUTPUT ${out_inc_dir})
include(driver.cmake)
if(CONFIG_USB_HOST_SUPPORT)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src/usb_host)
endif()
include(core.cmake)

set(target driver_open)
add_app_libraries($<TARGET_FILE:${target}>)

add_library(${target} STATIC ${dummy_c_file})
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_DRIVER)
target_compile_options(${target} PRIVATE "-Wnull-dereference")
target_compile_options(${target} PRIVATE "-Wa,-mimplicit-it=thumb")
target_include_directories(${target} PUBLIC include)
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR} src)

target_include_directories(${target} PRIVATE ../../fibocom/api)
target_include_directories(${target} PRIVATE ../cfw/include)
target_include_directories(${target} PRIVATE ../atr/include) 
target_include_targets(${target} PRIVATE hal kernel fs fsmount nanopb diag ml)
target_include_targets(${target} PRIVATE nvm net calclib apploader audio)
if(CONFIG_LCD_SUPPORT)
    target_include_targets(${target} PRIVATE lcdpanel)
endif()

target_sources_if(CONFIG_SOC_8850 THEN ${target} PRIVATE
    src/spi_flash/drv_spi_flash.c
)

target_sources_if(CONFIG_LCD_SUPPORT THEN ${target} PRIVATE
    src/lcd_v2/drv_lcd_v2.c
)

target_sources_if(CONFIG_LCD_SUPPORT THEN ${target} PRIVATE
    src/drv_lcd_cfg.c
    src/drv_lcd.c
    src/lcd/drv_lcd_st7789v2.c
    src/lcd/drv_lcd_gc9307.c
)
target_sources_if(CONFIG_LCD_ST7567A_SUPPORT THEN ${target} PRIVATE
    src/lcd/drv_lcd_st7567a.c
)
target_sources_if(CONFIG_FIBOCOM_SUPPORT_LCD_ST7789p3 THEN ${target} PRIVATE
    src/lcd/drv_lcd_st7789p3.c
)

target_sources_if(CONFIG_CAMERA_SUPPORT THEN ${target} PRIVATE
    src/drv_camera.c
    src/image_sensor/camera_gc032a.c
    src/image_sensor/camera_gc0310.c
    src/image_sensor/image_sensor.c
    src/image_sensor/camera_bf30a2.c
    src/image_sensor/camera_gc2145.c
    src/image_sensor/camera_gc0406.c
    src/image_sensor/camera_bf3905.c
    src/image_sensor/camera_bf20a6.c
)

relative_glob(srcs include/*.h src/*.[ch] src/*.cpp src/uart/*.[ch]
    src/usb/*.[ch] src/image_sensor/*.[ch] core/*.[ch] src/lcd/*.[ch]
    src/usb_8850/*.[ch] src/lcd_v2/*.[ch] src/debugport/*.[ch] src/mipi_lcd/*.[ch] src/spi_flash/*.[ch]
)
#beautify_c_code(${target} ${srcs})
