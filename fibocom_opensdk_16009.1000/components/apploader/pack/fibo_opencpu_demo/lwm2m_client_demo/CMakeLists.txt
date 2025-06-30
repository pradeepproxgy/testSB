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

configure_file(include/cust_app.h.in ${out_inc_dir}/cust_app.h)
set(target appimg)

if(CONFIG_SOC_8850)
    add_definitions(-DLWM2M_BIG_ENDIAN)
    add_definitions(-DLWM2M_CLIENT_MODE)
    add_definitions(-DLWM2M_SUPPORT_TLV)
    add_definitions(-DLWM2M_SUPPORT_SENML_JSON)
    add_definitions(-DLWM2M_OLD_CONTENT_FORMAT_SUPPORT)
    add_definitions(-DLWM2M_COAP_DEFAULT_BLOCK_SIZE=1024)
    add_definitions(-DLWM2M_BOOTSTRAP)
    add_definitions(-DLWM2M_WITH_LOGS) 
    add_definitions(-DCOAP_ENABLE)
    #add_definitions(-DWITH_MBEDTLS)
    add_definitions(-DMG_ENABLE_COAP -DCS_PLATFORM=0)

    add_library(${target} STATIC
        src/app_lwm2m.c
        src/app_at_cmd.c
        src/flash_partinfo.c
        src/fibo_config.c
        src/lwm2m_client/lwm2mclient.c
        src/lwm2m_client/object_device.c
        src/lwm2m_client/object_firmware.c
        src/lwm2m_client/object_security.c
        src/lwm2m_client/object_server.c
        src/lwm2m_client/shared/connection.c
        #src/lwm2m_client/shared/mbedconnection.c
        src/lwm2m_client/shared/platform.c
        src/lwm2m_client/lwm2m_fota.c
        src/lwm2m_client/app_pdp.c)

    target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_APPSTART)
    set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
    target_include_targets(${target} PRIVATE
    atr ats kernel hal driver ml fs sffs fsmount mal service
    cfw net nvm diag audio apploader fibocom)
    target_include_directories(${target} PRIVATE ../components/tts/include)
    target_include_directories(${target} PRIVATE ../fibocom/opencpu)
    target_include_directories(${target} PRIVATE ../fibocom/api)
    target_include_directories(${target} PRIVATE ./include)
    target_include_directories(${target} PRIVATE ./src/lwm2m_client)
    target_include_directories(${target} PRIVATE ./src/lwm2m_client/shared)
    target_include_directories(${target} PRIVATE ../components/firmwareupdate/include)
    target_include_directories(${target} PRIVATE ../components/calclib/include)
    target_include_directories(${target} PUBLIC  ../components/net/lwip/src/include/lwip)
    target_include_directories(${target} PUBLIC  ../components/net/lwip/src/include)
    target_include_directories(${target} PUBLIC  ../components/net/lwip/include)
    target_include_directories(${target} PUBLIC  ../components/net/libexpat/include)
    target_include_directories(${target} PUBLIC  ../components/net/json/cJSON)
    target_include_directories(${target} PRIVATE ../fibocom/ble/inc)
    target_include_directories(${target} PRIVATE ../fibocom/fibo-framework/lib/include)
    target_include_directories(${target} PRIVATE ../fibocom/fibo-framework/plateform/unisoc/include)
    target_include_directories(${target} PRIVATE ../fibocom/fibo-framework/3rd/wakaama/coap)
    target_include_directories(${target} PRIVATE ../fibocom/fibo-framework/3rd/wakaama/core)
    target_include_directories(${target} PRIVATE ../fibocom/fibo-framework/3rd/wakaama/include)
    target_include_directories(${target} PRIVATE ../components/atr/src)
    target_include_directories(${target} PRIVATE ../components/atr/include)
if(CONFIG_FIBOCOM_XIAONIU_FEATURE)
    target_include_directories(${target} PRIVATE ../components/fibocustom/NIU/)
endif()
    target_link_libraries(${BUILD_TARGET} PRIVATE all_libs ${target} all_libs)
endif()

add_subdirectory_if_exist(didi)

if(CONFIG_APPIMG_LOAD_FLASH)
    set(target appimg_flash)
    add_appimg_flash_example(${target} src/app_image.c
                                       src/app_ver.c)
    target_include_targets(${target} PRIVATE
    atr ats kernel hal driver ml fs sffs fsmount mal service
    cfw net nvm diag audio apploader fibocom)
    target_include_directories(${target} PRIVATE ../components/tts/include)
    target_include_directories(${target} PRIVATE ../fibocom/opencpu)
    target_include_directories(${target} PRIVATE ../fibocom/api)
    target_include_directories(${target} PRIVATE ./include)
    target_include_directories(${target} PRIVATE ../components/firmwareupdate/include)
    target_include_directories(${target} PRIVATE ../components/calclib/include)
    target_include_directories(${target} PUBLIC  ../components/net/lwip/src/include/lwip)
    target_include_directories(${target} PUBLIC  ../components/net/lwip/src/include)
    target_include_directories(${target} PUBLIC  ../components/net/lwip/include)
    target_include_directories(${target} PUBLIC  ../components/net/libexpat/include)
    target_include_directories(${target} PUBLIC  ../components/net/json/cJSON)
    target_include_directories(${target} PRIVATE ../fibocom/ble/inc)
    target_include_directories(${target} PRIVATE ../fibocom/fibo-framework/lib/include)
    target_include_directories(${target} PRIVATE ../fibocom/fibo-framework/plateform/unisoc/include)
    target_include_directories(${target} PRIVATE ../components/atr/src)
    target_include_directories(${target} PRIVATE ../components/atr/include)
if(CONFIG_FIBOCOM_XIAONIU_FEATURE)
    target_include_directories(${target} PRIVATE ../components/fibocustom/NIU/)
endif()
endif()

if(CONFIG_APPIMG_LOAD_FILE)
    set(target appimg_file)
    add_appimg_file_example(${target} src/app_image.c)
    target_include_targets(${target} PRIVATE kernel)
endif()
relative_glob(srcs include/*.h src/*.c src/*.h *.c)
beautify_c_code(${target} ${srcs})