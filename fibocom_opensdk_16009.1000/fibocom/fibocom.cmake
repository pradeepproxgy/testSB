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

set(import_lib ${out_lib_dir}/libfibocom.a)
configure_file(../target/${BUILD_TARGET}/libs/libfibocom.a ${import_lib} COPYONLY)
add_app_libraries(${import_lib})
add_library(fibocom STATIC IMPORTED GLOBAL)
set_target_properties(fibocom PROPERTIES IMPORTED_LOCATION ${import_lib} INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/api)

set(import_lib_open ${out_lib_dir}/libopencpu.a)
configure_file(../target/${BUILD_TARGET}/libs/libopencpu.a ${import_lib_open} COPYONLY)
add_app_libraries(${import_lib_open})
add_library(opencpu STATIC IMPORTED GLOBAL)
set_target_properties(opencpu PROPERTIES IMPORTED_LOCATION ${import_lib_open} INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/api)

message(STATUS "current ${CMAKE_SOURCE_DIR}") 
if(EXISTS ${CMAKE_SOURCE_DIR}/target/${BUILD_TARGET}/libs/libfibocustom.a)
set(import_lib_fibocustom ${out_lib_dir}/libfibocustom.a)
configure_file(../target/${BUILD_TARGET}/libs/libfibocustom.a ${import_lib_fibocustom} COPYONLY)
message(STATUS " use fibocustom")
add_app_libraries(${import_lib_fibocustom})
add_library(fibocustom STATIC IMPORTED GLOBAL)
set_target_properties(fibocustom PROPERTIES IMPORTED_LOCATION ${import_lib_fibocustom} INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/api)
else()
message(STATUS " no use fibocustom")
endif()

set(import_lib_fibohw ${out_lib_dir}/libfibomhw.a)
configure_file(../target/${BUILD_TARGET}/libs/libfibomhw.a ${import_lib_fibohw} COPYONLY)
message(STATUS " use fibohw")
add_app_libraries(${import_lib_fibohw})
add_library(fibomhw STATIC IMPORTED GLOBAL)
set_target_properties(fibomhw PROPERTIES IMPORTED_LOCATION ${import_lib_fibohw} INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/api)

if (CONFIG_TTS_SUPPORT)
message(STATUS " use tts")
set(import_lib_octts ${out_lib_dir}/liboctts.a)
configure_file(../target/${BUILD_TARGET}/libs/liboctts.a ${import_lib_octts} COPYONLY)
add_app_libraries(${import_lib_octts})
add_library(octts STATIC IMPORTED GLOBAL)
else()
message(STATUS " use tts no")
set(import_lib_octts ${out_lib_dir}/liboctts.a)
configure_file(../target/${BUILD_TARGET}/libs/libocttsno.a ${import_lib_octts} COPYONLY)
add_app_libraries(${import_lib_octts})
add_library(octts STATIC IMPORTED GLOBAL)
endif()
