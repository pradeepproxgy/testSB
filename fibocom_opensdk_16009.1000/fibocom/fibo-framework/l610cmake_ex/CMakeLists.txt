# cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

include(./ffw.mak)
if (${CONFIG_FIBOCOM_ALIPAY_SDK})
include(alipaysdk.cmake)
endif()

