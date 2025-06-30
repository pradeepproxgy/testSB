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

cmake_minimum_required(VERSION 3.13)

set(SOURCE_TOP_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BINARY_TOP_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(CMAKE_C_COMPILER_FORCED   1)
set(CMAKE_CXX_COMPILER_FORCED 1)
include(cmake/extension.cmake)

# Use a global property to collect all application libraries
define_property(GLOBAL PROPERTY app_libraries
    BRIEF_DOCS "app libraries"
    FULL_DOCS "app libraries"
)

set(out_hex_dir ${CMAKE_CURRENT_BINARY_DIR}/hex)
set(out_hex_example_dir ${CMAKE_CURRENT_BINARY_DIR}/hex/examples)
set(out_hex_unittest_dir ${CMAKE_CURRENT_BINARY_DIR}/hex/unittests)
set(out_lib_dir ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(out_rpc_dir ${CMAKE_CURRENT_BINARY_DIR}/rpcgen)
set(out_inc_dir ${CMAKE_CURRENT_BINARY_DIR}/include)
set(tools_dir ${CMAKE_CURRENT_SOURCE_DIR}/tools)

set(build_target_dir ${CMAKE_CURRENT_SOURCE_DIR}/target/${BUILD_TARGET})
set(elf2symld_py ${tools_dir}/elf2symld.py)
set(elf2incld_py ${tools_dir}/elf2incld.py)
set(pacgen_py ${tools_dir}/pacgen.py)
set(signature_py ${tools_dir}/signature/signature.py)
set(iomuxgen_py ${tools_dir}/iomuxgen.py)
set(keypadgen_py ${tools_dir}/keypadgen.py)
set(partinfogen_py ${tools_dir}/partinfogen.py)
set(modemgen_py ${tools_dir}/modemgen.py)
set(romtablegen_py ${tools_dir}/romtablegen.py)
set(symdefgen_py ${tools_dir}/symdefgen.py)
set(groupgen_py ${tools_dir}/groupgen.py)
set(norpropgen_py ${tools_dir}/norpropgen.py)
set(libs2one_py ${tools_dir}/libs2one.py)
set(codesize_py ${tools_dir}/codesize.py)
set(corestubgen dtools expgen)
set(cmd_mkuimage dtools mkuimage)
set(cmd_mksimage dtools mksimage)
set(cmd_mkappimg dtools mkappimg)
set(cmd_elf2lod dtools elf2lod)
set(cmd_elf2bin dtools elf2bin)
set(cmd_elf2romhex dtools elf2romhex)
set(cmd_lzmare2 dtools lzmare2)
set(cmd_ssign dtools ssign)
set(cmd_ssignchain dtools ssignchain)
set(cmd_ssigndebug dtools ssigndebug)
set(cmd_ssigndevel dtools ssigndevel)
set(cmd_sencrypt dtools sencrypt)
set(cmd_lzmacomp lzma -f -k -9)
set(cmd_re2c re2c)
set(cmd_insertsimlockey dtools simlockinskey)
set(partinfo_bin ${out_hex_dir}/partinfo.bin)
set(core_stub_o ${out_lib_dir}/core_stub.o)
set(dummy_c_file ${SOURCE_TOP_DIR}/cmake/dummy.c)
set(dummy_cxx_file ${SOURCE_TOP_DIR}/cmake/dummy.cpp)

if(WIN32)
    set(shell_cmd busybox sh)
else()
    set(shell_cmd /bin/sh)
endif()

set(AUTO_GENERATION_NOTICE "Auto generated. Don't edit it manually!")

find_package(Git)
if(GIT_FOUND)
    execute_process(COMMAND "${GIT_EXECUTABLE}" describe --abbrev=8 --always --dirty
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE BUILD_GIT_REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

# BUILD_REVISION: A string to identify the build. When not specified, which
#       is common in development, it will be "DEVEL". CI will define it in
#       command line, such as "8915_W19.32.2"
if(NOT BUILD_REVISION)
    set(BUILD_REVISION $ENV{BUILD_REVISION})
    if(NOT BUILD_REVISION)
        set(BUILD_REVISION DEVEL)
    endif()
endif()

# BUILD_AUTO_REVISION: A string to identify the source codes revision.
#       When it is built under git working directory, it will be "g1234abcd"
#       or "g1234abcd-dirty" if the working directory is dirty. When not in
#       git working directory, it will be "unknown". And it can be overrided
#       in command line.
if(NOT BUILD_AUTO_REVISION)
    if(BUILD_GIT_REVISION)
        set(BUILD_AUTO_REVISION "g${BUILD_GIT_REVISION}")
    endif()
endif()

if(NOT BUILD_AUTO_REVISION)
    set(BUILD_AUTO_REVISION "unknown")
endif()

if(NOT BUILD_TIMESTAMP)
    string(TIMESTAMP COMPILE_TIME %Y%m%d_%H%M%S)
    set(BUILD_TIMESTAMP ${COMPILE_TIME})
    message(STATUS build_time:${BUILD_TIMESTAMP})
endif()

# BUILD_REVISION_NUMBER: A number to identify the source codes revision.
#       When it is built under git working directory, and the working
#       directory is clean, it will be "0x1234abcd". Otherwise, it is "0".
#       And it can be overrided in command line.
if(NOT BUILD_REVISION_NUMBER)
    if(BUILD_GIT_REVISION)
        set(BUILD_REVISION_NUMBER "0x${BUILD_GIT_REVISION}")
    endif()
    if(BUILD_REVISION_NUMBER MATCHES "-dirty")
        set(BUILD_REVISION_NUMBER "0")
    endif()
endif()

if(NOT BUILD_REVISION_NUMBER)
    set(BUILD_REVISION_NUMBER "0")
endif()

# BUILD_TARGET
if(NOT BUILD_TARGET)
    set(BUILD_TARGET $ENV{BUILD_TARGET})
endif()

# BUILD_RELEASE_TYPE
if (NOT BUILD_RELEASE_TYPE)
    set (BUILD_RELEASE_TYPE $ENV{BUILD_RELEASE_TYPE})
endif()

if("${BUILD_RELEASE_TYPE}" STREQUAL "release")
    set(CONFIG_WDT_ENABLE ON)
    set(CONFIG_AP_A5_CLK_AUTO_GATE ON)
endif()

#add by fibocom for enable wdt
if("${BUILD_RELEASE_TYPE}" STREQUAL "debug")
    set(CONFIG_WDT_ENABLE ON)
endif()

if(NOT BUILD_TARGET)
    message(FATAL_ERROR "BUILD_TARGET is empty")
endif()

# Process and include target config
set(CONFIG_BUILD_TARGET ${BUILD_TARGET})
set(CONFIG_BUILD_IDENTIFY "${BUILD_TARGET}-${BUILD_RELEASE_TYPE}-${BUILD_REVISION}-${BUILD_AUTO_REVISION}-${BUILD_TIMESTAMP}")
set(TARGET_CONFIG ${SOURCE_TOP_DIR}/target/${BUILD_TARGET}/target.config)
set(TARGET_CMAKE ${BINARY_TOP_DIR}/target.cmake)
execute_process(
    COMMAND python3 ${tools_dir}/cmakeconfig.py ${TARGET_CONFIG} ${TARGET_CMAKE}
    WORKING_DIRECTORY ${SOURCE_TOP_DIR}
)

include(${TARGET_CMAKE})

# Process and include partition config
execute_process(
    COMMAND python3 ${partinfogen_py} ${CONFIG_PARTINFO_JSON_PATH} ${partinfo_bin} ${BINARY_TOP_DIR}/partinfo.cmake
    WORKING_DIRECTORY ${SOURCE_TOP_DIR}
)
include(${BINARY_TOP_DIR}/partinfo.cmake)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${partinfogen_py} ${CONFIG_PARTINFO_JSON_PATH})

# Include chip config
include(components/chip/chip.cmake)

# Process and include Kconfig dependency
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${TARGET_CONFIG})
set(KCONFIG_DEP ${BINARY_TOP_DIR}/kconfig_dep.cmake)
execute_process(
    COMMAND python3 ${tools_dir}/cmakedep.py ${KCONFIG_DEP}
    WORKING_DIRECTORY ${SOURCE_TOP_DIR}
)
include(${KCONFIG_DEP})

# Include toolchain config
include(cmake/toolchain-gcc.cmake)
set(BUILD_SHARED_LIBS OFF)

project(${BUILD_TARGET} C CXX ASM)
include_directories(${out_inc_dir})
include_directories(${SOURCE_TOP_DIR}/components/newlib/include)
include_directories(${SOURCE_TOP_DIR}/components/net/mbedtls)
include_directories(AFTER SYSTEM ${BINARY_TOP_DIR}/ffw_include)
add_custom_target(beautify)
add_custom_target(unittests)
add_custom_target(examples ALL)
add_custom_target(rpcgen)
add_appimg_delete()

if(CONFIG_APP_COMPRESSED_SUPPORT)
file(REMOVE ${out_inc_dir}/ap_compress_cfg.h)
file(REMOVE ${out_hex_dir}/ap_compress.bin)
file(REMOVE ${out_hex_dir}/ap_compress.img)
file(REMOVE ${out_hex_dir}/${nvmvariant}_ap.cpio)
include(components/appstart/include/compress.libs)
endif()

add_subdirectory_if_exist(components)
add_subdirectory_if_exist(fibocom/fibo-framework/l610cmake_ex)
add_subdirectory_if_exist(fibocom)
add_subdirectory_if_exist(fibocom/fibomhw)
add_subdirectory_if_exist(fibocom/cust)
add_subdirectory_if_exist(cust_app)
add_subdirectory_if_exist(cust_app/src)
add_subdirectory_if_exist(cust_app/src/service)
add_subdirectory_if_exist(cust_app/src/user)
add_subdirectory_if_exist(unittest)
pacvariant_gen()

# "all_libs" is a target to link all app libraries
set(target all_libs)
set(all_libs_out ${out_lib_dir}/all_libs.a)
get_property(app_libraries GLOBAL PROPERTY app_libraries)
list(APPEND app_libraries ${libc_file_name} ${libm_file_name} ${libgcc_file_name})
add_custom_command(OUTPUT ${all_libs_out}
    COMMAND python3 ${groupgen_py} --base ${BINARY_TOP_DIR} ${all_libs_out} ${app_libraries}
    DEPENDS ${groupgen_py} ${app_libraries}
    WORKING_DIRECTORY ${BINARY_TOP_DIR}
)
add_custom_target(build_${target} DEPENDS ${all_libs_out})
add_library(${target} STATIC IMPORTED GLOBAL)
set_target_properties(${target} PROPERTIES IMPORTED_LOCATION ${all_libs_out})
add_dependencies(${target} cbbsdk)
add_dependencies(${target} build_${target})

if ("${BUILD_TARGET}" STREQUAL "8850BM_C1_cat1bis")
    set(CONFIG_BUILD_CODESIZE_ENABLE ON)
endif()

if(CONFIG_BUILD_CODESIZE_ENABLE)
    set(img_map_file ${out_hex_dir}/${BUILD_TARGET}.map)
    set(img_size_file ${out_hex_dir}/outlib.csv)
    add_custom_command(OUTPUT ${img_size_file}
        COMMAND python3 ${codesize_py} --map ${img_map_file}
        DEPENDS ${codesize_py} ${img_map_file}
    )

    add_custom_target(img_size_gen ALL DEPENDS ${img_size_file})
endif()
