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

configure_file(include/net_config.h.in ${out_inc_dir}/net_config.h)

set(target net)
add_app_libraries($<TARGET_FILE:${target}>)

add_library(${target} STATIC)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_NET)
target_include_directories(${target} PUBLIC include)
target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(${target} PRIVATE ../../ffw/lib/include)
target_include_directories(${target} PUBLIC ../../components/firmwareupdate/include)
target_include_directories(${target} PRIVATE lwip/include)
target_include_directories(${target} PRIVATE sci_netif/include)
target_include_directories(${target} PRIVATE lwip/src/include)
target_include_directories(${target} PRIVATE lwip/src/include/lwip)
target_include_directories(${target} PRIVATE lwip/src/include/lwip/priv)
target_include_directories(${target} PRIVATE ../ats/include)
target_include_directories(${target} PRIVATE ../hal/include)
target_include_directories(${target} PRIVATE ../net/mupnp/src/mupnp/include)
target_include_directories(${target} PRIVATE ../net/mbedtls/include)
target_include_directories(${target} PRIVATE http/include)
target_include_directories(${target} PRIVATE ../json/cJSON)
target_include_directories(${target} PRIVATE coap/include)
target_include_directories(${target} PRIVATE ctiotcryplib/include)
target_include_directories(${target} PRIVATE ../fs/include)
target_include_directories(${target} PRIVATE ctwing/include)
target_include_directories(${target} PRIVATE ../pam/include)
target_include_directories(${target} PRIVATE ../../ffw/cbb/um_esim/src/crypto/sm3)

target_include_directories(${target} PRIVATE cis_sdk/src/NB_SDK2.3.0_D1.1.1/src/ciscore)
# target_include_directories(${target} PRIVATE ../../fibocom/fibo-framework/cbb/um_esim/src/crypto/sm3)
target_include_directories(${target} PRIVATE ../../fibocom/fibo-framework/include)
target_include_directories(${target} PRIVATE ../../fibocom/fibo-framework/lib/include)
target_include_directories(${target} PRIVATE ../../fibocom/fibo-framework/plateform/unisoc/include)
target_include_directories(${target} PRIVATE  json/cJSON)
target_include_directories(${target} PRIVATE  ../../components/kernel/include/sci)


target_include_targets(${target} PRIVATE kernel driver calclib lwip atr cfw json fs nvm fupdate service)
target_include_targets_if(CONFIG_NET_ALI_SDK_SUPPORT THEN ${target} PRIVATE ali_c_sdk)
target_include_targets_if(CONFIG_MBEDTLS_LIB_SUPPORT THEN ${target} PRIVATE mbedtls)
target_include_targets_if(CONFIG_TINYDTLS_LIB_SUPPORT THEN ${target} PRIVATE tinydtls)
target_include_targets_if(CONFIG_MBEDTLS_USE_CE_CRYPTO THEN ${target} PRIVATE crypto)
target_include_targets_if(CONFIG_NET_DM_CISSDK_SUPPORT THEN ${target} PRIVATE cis_sdk)
target_include_targets_if(CONFIG_NET_DM_LWM2M_SUPPORT THEN ${target} PRIVATE lwm2m)
target_include_targets_if(CONFIG_NET_DM_CUS_SUPPORT THEN ${target} PRIVATE lwm2m)
target_include_targets_if(CONFIG_NET_IDS_SUPPORT THEN ${target} PRIVATE ids)
target_include_targets_if(CONFIG_NET_SCI_NETIF_SUPPORT THEN ${target} PRIVATE sci_netif)
target_include_targets_if(CONFIG_NET_CTIOTSM_SUPPORT THEN ${target} PRIVATE ctiotsm)
target_include_targets_if(CONFIG_NET_LIBDMP_SDK_SUPPORT THEN ${target} PRIVATE libdmpsdk)
target_include_targets_if(CONFIG_CTHTTP_API_SUPPORT THEN ${target} PRIVATE ctwing)

target_sources(${target} PRIVATE

	src/netif_gprs.c
	src/lwipports.c
	src/netmain.c
	src/netutils.c
	src/cfw_sockets_api.c
	src/ppp_interface.c
	src/lwip_mqtt_api.c
	src/unicomIot.c
	src/netif_nat_wan.c
	src/netif_nat_lan_lwip.c
	src/mbedtls_sockets.c
	src/ssl_socket.c
	src/netif_psm.c
	src/lwip_data_event.c
    src/app_tcp_if_open.c
	)

if(CONFIG_NET_NONIP_SUPPORT)
target_sources(${target} PRIVATE
        src/nonip.c
        )
endif()

if(CONFIG_MUPNP_LIB_SUPPORT)
target_sources(${target} PRIVATE
        src/dmphttp_cplog_send.c
        )
endif()

if(CONFIG_NET_DNS_SERVER_SUPPORT)
target_sources(${target} PRIVATE
        src/dns_server.c
        )
endif()

if(CONFIG_NET_SELF_REGISTE_SUPPORT)
target_sources(${target} PRIVATE
	self_registration/vnetIot.c
	)
endif()

if(CONFIG_NET_SELFREG_COAP_SUPPORT)
target_sources(${target} PRIVATE
	self_registration/tele_nb_coapselfreg.c
	)
endif()

if(CONFIG_NET_DM_CISSDK_SUPPORT)
target_sources(${target} PRIVATE
	self_registration/dm_module_cissdk.c
	)
endif()

if(CONFIG_NET_DM_LWM2M_SUPPORT)
target_sources(${target} PRIVATE
	self_registration/dm_module_lwm2m.c
	)
endif()

if(CONFIG_NET_DM_CUS_SUPPORT)
target_sources(${target} PRIVATE
	self_registration/dm_module_cus.c
	)
endif()

if(CONFIG_SOC_8910)
target_sources(${target} PRIVATE
	src/dhcp_reply.c
	src/nd6_reply.c
	src/ppp6_reply.c
	)
endif()

if(CONFIG_SOC_8850)
target_sources(${target} PRIVATE
	src/dhcp_reply.c
	src/nd6_reply.c
	src/ppp6_reply.c
	)
endif()
if(CONFIG_SOC_8821)
target_sources(${target} PRIVATE
	src/dhcp_reply.c
	src/nd6_reply.c
	src/ppp6_reply.c
	)
endif()
if(CONFIG_SOC_8811)
target_sources(${target} PRIVATE
	src/dhcp_reply.c
	src/nd6_reply.c
	src/ppp6_reply.c
	)
endif()

if(CONFIG_USB_ETHER_SUPPORT)
target_sources(${target} PRIVATE
	src/netdev_interface.c
	src/netdev_interface_nat_lan.c
	src/dhcp_reply.c
	src/nd6_reply.c
	src/ppp6_reply.c
	)
endif()

relative_glob(srcs include/*.h src/*.c src/*.h)
beautify_c_code(${target} ${srcs})

add_subdirectory_if_exist(lwip)
add_subdirectory_if(CONFIG_NET_SCI_NETIF_SUPPORT THEN sci_netif)
add_subdirectory_if(CONFIG_NET_LIBCOAP_SUPPORT THEN coap)
add_subdirectory_if_exist(json)
add_subdirectory_if_exist(libexpat)
add_subdirectory_if_exist(edp)
add_subdirectory_if(CONFIG_LWIP_IPSEC_SUPPORT THEN ipsec)
add_subdirectory_if(CONFIG_MBEDTLS_LIB_SUPPORT THEN mbedtls)
add_subdirectory_if(CONFIG_MUPNP_LIB_SUPPORT THEN http)
add_subdirectory_if(CONFIG_NET_VOLTE_HTTP_SUPPORT THEN http2)
add_subdirectory_if(CONFIG_MUPNP_LIB_SUPPORT THEN mupnp)
#add_subdirectory_if(CONFIG_NET_LWM2M_SUPPORT THEN liblwm2m)
add_subdirectory_if(CONFIG_FTP_MODULE_SUPPORT THEN ftp)
add_subdirectory_if(CONFIG_NET_CIS_SDK_SUPPORT THEN cis_sdk)
add_subdirectory_if(CONFIG_NET_PAHO_MQTT_SUPPORT THEN pahomqtt)
add_subdirectory_if(CONFIG_NET_ALI_SDK_SUPPORT THEN ali_c_sdk)
add_subdirectory_if(CONFIG_TINYDTLS_LIB_SUPPORT THEN tinydtls)
add_subdirectory_if(CONFIG_NET_IDS_SUPPORT THEN ids)
add_subdirectory_if(CONFIG_NET_CTIOTSM_SUPPORT THEN ctiotsm)
add_subdirectory_if(CONFIG_NET_LIBDMP_SDK_SUPPORT THEN libdmpsdk)
add_subdirectory_if(CONFIG_NET_CTIOT_AEP_SUPPORT THEN ctiot_aep)
add_subdirectory_if(CONFIG_CTHTTP_API_SUPPORT THEN ctwing)
add_subdirectory_if(CONFIG_MATRIX_SSL_API_SUPPORT THEN matrix_ssl)
add_subdirectory_if(CONFIG_NET_IMSHTTP_LIB_SUPPORT THEN http2)
