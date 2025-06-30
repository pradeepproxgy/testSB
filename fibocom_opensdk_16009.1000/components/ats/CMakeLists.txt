# copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
# All rights reserved.
#
# This software is supplied "AS IS" without any warranties.
# RDA assumes no responsibility or liability for the use of the software,
# conveys no license or title under any patent, copyright, or mask work
# right to the product. RDA reserves the right to make changes in the
# software without notification.  RDA also make no representation or
# warranty that such application will be suitable for the specified use
# without further testing or modification.

set(AT_MANUFACTURER_ID "UNISOC")
set(AT_MODEL_ID "${BUILD_TARGET}-${BUILD_RELEASE_TYPE}")
set(AT_REVISION_ID "${BUILD_REVISION}-${BUILD_AUTO_REVISION}")
set(AT_TIMESTAMP_ID "${BUILD_TIMESTAMP}")

configure_file(include/ats_config.h.in ${out_inc_dir}/ats_config.h)

set(at_apn_table_h ${CMAKE_CURRENT_BINARY_DIR}/at_apn_table.h)
set_if(apn_csv CONFIG_AT_GLOBAL_APN_SUPPORT
    THEN apn/global_apn.csv
    OTHERWISE apn/local_apn.csv
)
add_custom_command(OUTPUT ${at_apn_table_h}
    COMMAND python3 apn/csv_parse.py ${apn_csv} ${at_apn_table_h}
    DEPENDS apn/csv_parse.py ${apn_csv}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

set(target fibo_ats)
add_app_libraries($<TARGET_FILE:${target}>)

add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
target_include_directories(${target} PUBLIC include)
target_include_directories(${target} PUBLIC ../../ffw/lib/include)
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_targets(${target} PRIVATE
    kernel driver ml hal calclib atr cfw fs fsmount bdev
    aworker net lwip mal diag nvm fupdate audio service tts
)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_ATE)

target_include_directories(${target} PRIVATE ../atr/src)
target_include_directories(${target} PRIVATE ../audio/include)
target_include_directories(${target} PRIVATE ../driver/include)
target_include_directories(${target} PRIVATE ../fibocom/include)
target_include_directories(${target} PRIVATE ../net/lwip/src/include)

target_sources(${target} PRIVATE
    ${at_apn_table_h}
    misc/at_cmd_sysnv.c
)

set(target ats)
include(ats.cmake)