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

configure_file(include/atr_config.h.in ${out_inc_dir}/atr_config.h)

set(target at_cmd_table_gperf)
set(gperf_in src/at_cmd_table.gperf)
set(gperf_h ${CMAKE_CURRENT_BINARY_DIR}/at_cmd_table.h)
cpp_only(${target} ${gperf_in})

add_custom_command(OUTPUT ${gperf_h}
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/src/gperf2h.py
        $<TARGET_OBJECTS:at_cmd_table_gperf> ${gperf_h}
    DEPENDS $<TARGET_OBJECTS:at_cmd_table_gperf>
        ${CMAKE_CURRENT_SOURCE_DIR}/src/gperf2h.py
)

nanopbgen(src/at_setting.proto)

set(target fibo_atr)
add_app_libraries($<TARGET_FILE:${target}>)

add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_ATE)
target_include_directories(${target} PUBLIC include)
target_include_directories(${target} PRIVATE ../../fibocom/api)
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${target} PRIVATE ../audio/include)
target_include_directories(${target} PRIVATE ../nvm/include)
target_include_directories(${target} PRIVATE ../hal/include)
target_include_targets(${target} PRIVATE tb service hal)
if(CONFIG_SOC_8910 OR CONFIG_SOC_8850)
target_include_targets(${target} PRIVATE lpa)
endif()
if(CONFIG_PAM_LTE_GNSS_WIFISCAN_SUPPORT)
target_include_targets(${target} PRIVATE pam)
endif()
target_link_libraries(${target} PRIVATE kernel driver calclib ats cfw net nanopb fs service
diag fibocom fibocustom)

target_sources(${target} PRIVATE
    ${gperf_h}
    src/at_parse.c
    src/at_setting_codec.c
    ${CMAKE_CURRENT_BINARY_DIR}/at_setting.pb.c
)
set(target atr)
include(core.cmake)
include(atr.cmake)
