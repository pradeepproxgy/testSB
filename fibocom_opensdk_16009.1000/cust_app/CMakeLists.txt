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
    add_library(${target} STATIC
        soundbox/soundbox.c
	src/app_codes.c
        src/app_at_cmd.c
        src/flash_partinfo.c
        src/fibo_config.c
        src/fibo_logo_config.c
        src/fibo_vendor_config.c
	soundbox/service/sequeue.c
        soundbox/service/http_service.c
        soundbox/service/event_signal.c
        soundbox/service/oc_audio_demo.c
        soundbox/service/oc_mqtt_demo.c
        soundbox/service/ota_service.c
        soundbox/service/log_service.c
        soundbox/service/network_service.c
        soundbox/service/oc_earphone_custom.c
	soundbox/service/oc_adc_demo.c
        # Eng Directory
        soundbox/eng/ali_iot.c

        # Eng/OTA Subdirectory
        soundbox/eng/ota/ota_lib.c
        soundbox/eng/ota/ota_coap.c
        soundbox/eng/ota/ota_mqtt.c
        soundbox/eng/ota/iotx_ota.c
        soundbox/eng/ota/ota_fetch.c

        # Eng/Wrappers Subdirectory
        soundbox/eng/wrappers/wrappers.c
        soundbox/eng/wrappers/external_libs/cJSON.c
        soundbox/eng/wrappers/external_libs/mbedtls/library/aes.c

        # Eng/MQTT Subdirectory
        soundbox/eng/mqtt/MQTTSubscribeClient.c
        soundbox/eng/mqtt/MQTTConnectClient.c
        soundbox/eng/mqtt/MQTTDeserializePublish.c
        soundbox/eng/mqtt/MQTTSerializePublish.c
        soundbox/eng/mqtt/MQTTUnsubscribeClient.c
        soundbox/eng/mqtt/MQTTPacket.c
        soundbox/eng/mqtt/mqtt_api.c
        soundbox/eng/mqtt/iotx_mqtt_client.c

        # Eng/Infra Subdirectory
        soundbox/eng/infra/infra_compat.c
        soundbox/eng/infra/infra_log.c
        soundbox/eng/infra/infra_httpc.c
        soundbox/eng/infra/infra_net.c
        soundbox/eng/infra/infra_report.c
        soundbox/eng/infra/infra_string.c
        soundbox/eng/infra/infra_json_parser.c
        soundbox/eng/infra/infra_timer.c
	soundbox/eng/infra/infra_md5.c
        soundbox/eng/infra/infra_sha1.c
        soundbox/eng/infra/infra_sha256.c
        soundbox/eng/infra/infra_defs.c
        soundbox/eng/infra/infra_aes.c
        soundbox/eng/infra/infra_cjson.c

        # Eng/HTTP Subdirectory
        soundbox/eng/http/iotx_http_api.c

        # Eng/Dev_Sign Subdirectory
        soundbox/eng/dev_sign/dev_sign_mqtt.c

        # Eng/Dev_Model Subdirectory
        soundbox/eng/dev_model/dm_client_adapter.c
        soundbox/eng/dev_model/dm_message_cache.c
        soundbox/eng/dev_model/dm_log_report.c
        soundbox/eng/dev_model/dm_opt.c
        soundbox/eng/dev_model/dm_ota.c
        soundbox/eng/dev_model/dm_fota.c
        soundbox/eng/dev_model/dm_msg_process.c
        soundbox/eng/dev_model/dm_server_adapter.c
        soundbox/eng/dev_model/dm_manager.c
        soundbox/eng/dev_model/dm_server.c
        soundbox/eng/dev_model/dm_shadow.c
        soundbox/eng/dev_model/dm_api.c
        soundbox/eng/dev_model/dm_utils.c
        soundbox/eng/dev_model/impl_solo.c
        soundbox/eng/dev_model/dm_cota.c
        soundbox/eng/dev_model/dm_ipc.c
        soundbox/eng/dev_model/impl_gateway.c
        soundbox/eng/dev_model/impl_ntp.c
        soundbox/eng/dev_model/iotx_cm.c

        # Devices Directory
	#  soundbox/devices/oc_codec_demo.c
        soundbox/devices/aw87359.c
	#soundbox/devices/oc_codec_es8311_demo.c

        # Platform Directory
        soundbox/platform/local_flash.c
        soundbox/platform/fb_tcp_lwip.c

        # User Directory
	   soundbox/user/key.c
	# soundbox/user/led.c
        soundbox/user/fb_uart.c)
    target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_APPSTART)
    set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
    target_include_targets(${target} PRIVATE
    atr ats kernel hal driver ml fs sffs fsmount mal service
    cfw net nvm diag audio apploader fibocom)
    target_include_directories(${target} PRIVATE ../components/tts/include)
    target_include_directories(${target} PRIVATE ../fibocom/opencpu)
    target_include_directories(${target} PRIVATE ../fibocom/api)
    target_include_directories(${target} PRIVATE ./include)
    target_include_directories(${target} PRIVATE ./soundbox/user)
    target_include_directories(${target} PRIVATE ./soundbox/devices)
    target_include_directories(${target} PRIVATE ./soundbox/service)
    target_include_directories(${target} PRIVATE ./soundbox/platform)
    target_include_directories(${target} PRIVATE ./soundbox/eng)
    target_include_directories(${target} PRIVATE ./soundbox/eng/dev_model)
    target_include_directories(${target} PRIVATE ./soundbox/eng/dev_sign)
    target_include_directories(${target} PRIVATE ./soundbox/eng/http)
    target_include_directories(${target} PRIVATE ./soundbox/eng/infra)
    target_include_directories(${target} PRIVATE ./soundbox/eng/mqtt)
    target_include_directories(${target} PRIVATE ./soundbox/eng/ota)
    target_include_directories(${target} PRIVATE ./soundbox/eng/wrappers)
    target_include_directories(${target} PRIVATE ./soundbox/eng/wrappers/external_libs)
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
    target_include_directories(${target} PRIVATE ../components/fibo_logo)
    target_include_directories(${target} PRIVATE ../components/driver/include)
    target_include_directories(${target} PRIVATE ../components/driver/include/usb)

if(CONFIG_FIBOCOM_XIAONIU_FEATURE)
    target_include_directories(${target} PRIVATE ../components/fibocustom/NIU/)
endif()
    target_link_libraries(${BUILD_TARGET} PRIVATE all_libs ${target} all_libs)
    if(CONFIG_FIBOCOM_POWERON_LOGO_SUPPORT)
        target_link_libraries(${target} PRIVATE fibo_logo)
    endif()
endif()

add_subdirectory_if_exist(didi)

if(CONFIG_APPIMG_LOAD_FLASH)
    if(CONFIG_APPIMG_APP_TEST)
        set(target testapi)
        add_appimg_flash_example(${target} src/app_ver.c)
        target_link_libraries(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../target/${BUILD_TARGET}/libs/libtestcase.a ${libc_file_name} ${libm_file_name} ${libgcc_file_name})
    else()
        set(target soundbox_mc661)
        add_appimg_flash_example(${target} soundbox/soundbox.c src/app_ver.c)
    endif()


    target_include_targets(${target} PRIVATE
    atr ats kernel hal driver ml fs sffs fsmount mal service
    cfw net nvm diag audio apploader fibocom)
    target_include_directories(${target} PRIVATE ../components/tts/include)
    target_include_directories(${target} PRIVATE ../fibocom/opencpu)
    target_include_directories(${target} PRIVATE ../fibocom/api)
    target_include_directories(${target} PRIVATE ./include)
    target_include_directories(${target} PRIVATE ./soundbox/user)
    target_include_directories(${target} PRIVATE ./soundbox/devices)
    target_include_directories(${target} PRIVATE ./soundbox/service)
    target_include_directories(${target} PRIVATE ./soundbox/platform)
    target_include_directories(${target} PRIVATE ./soundbox/eng)
    target_include_directories(${target} PRIVATE ./soundbox/eng/dev_model)
    target_include_directories(${target} PRIVATE ./soundbox/eng/dev_sign)
    target_include_directories(${target} PRIVATE ./soundbox/eng/http)
    target_include_directories(${target} PRIVATE ./soundbox/eng/infra)
    target_include_directories(${target} PRIVATE ./soundbox/eng/mqtt)
    target_include_directories(${target} PRIVATE ./soundbox/eng/ota)
    target_include_directories(${target} PRIVATE ./soundbox/eng/wrappers)
    target_include_directories(${target} PRIVATE ./soundbox/eng/wrappers/external_libs)
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
    add_appimg_file_example(${target} soundbox/soundbox.c)
    target_include_targets(${target} PRIVATE kernel)
endif()
relative_glob(srcs include/*.h src/*.c src/*.h *.c)
beautify_c_code(${target} ${srcs})
