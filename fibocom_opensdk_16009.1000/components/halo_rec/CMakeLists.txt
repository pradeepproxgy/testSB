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

configure_file(include/halo_rec_config.h.in ${out_inc_dir}/halo_rec_config.h)
configure_file(include/Slp.h ${out_inc_dir}/Slp.h)

set(target halo_rec)

if(CONFIG_HALO_SUPPORT)
    add_library(${target} STATIC IMPORTED GLOBAL)
    set_target_properties(${target} PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libhalo_rec.a
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    add_app_libraries($<TARGET_PROPERTY:${target},IMPORTED_LOCATION>)
elseif(CONFIG_HALO_V1_SUPPORT)
    add_library(${target} STATIC IMPORTED GLOBAL)
    set_target_properties(${target} PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/libhalo_rec_v1.a
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
)
add_app_libraries($<TARGET_PROPERTY:${target},IMPORTED_LOCATION>)
else()
    add_library(${target} INTERFACE)
    set_target_properties(${target} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include
)
endif()
