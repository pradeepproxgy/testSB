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
if (CONFIG_SUPPORT_GNSS)

set(target gnss)
if(CONFIG_GPS_SPREAD_ORBIT_SUPPORT)
set(libname gnss)
else()
set(libname gnss_without_orbit)
endif()

set(import_lib ${out_lib_dir}/lib${libname}.a)

configure_file(${CONFIG_SOC}/lib${libname}.a ${import_lib} COPYONLY)
add_app_libraries(${import_lib})
add_library(${target} STATIC IMPORTED GLOBAL)


set_target_properties(${target} PROPERTIES
    IMPORTED_LOCATION ${import_lib}
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include 
)

set(target qxsupl)
set(import_lib ${out_lib_dir}/qxsupl.a)
configure_file(${CONFIG_SOC}/qxsupl.a ${import_lib} COPYONLY)
add_app_libraries(${import_lib})
add_library(${target} STATIC IMPORTED GLOBAL)

set_target_properties(${target} PROPERTIES
    IMPORTED_LOCATION ${import_lib}
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/inc ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/hal/include ${CMAKE_CURRENT_SOURCE_DIR}/Third-Party/demo/include
)
endif()