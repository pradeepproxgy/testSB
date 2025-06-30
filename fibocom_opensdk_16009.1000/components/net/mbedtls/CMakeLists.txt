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

configure_file(include/mbedtls_config.h.in ${out_inc_dir}/mbedtls_config.h)
file(GLOB headers include/mbedtls/*.h)
#file(GLOB headers2 include/psa/*.h)
install_headers(${headers} OUTPUT ${out_inc_dir}/mbedtls)
#install_headers(${headers2} OUTPUT ${out_inc_dir}/psa)

if(CONFIG_MBEDTLS_SUPPORT_SM2)
    file(GLOB headers 3rdparty/sm2/include/*.h)
    install_headers(${headers} OUTPUT ${out_inc_dir}/mbedtls)
endif()

set(target mbedtls)
include(mbedtls.cmake)