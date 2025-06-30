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

set(LTE_PHY_V4 ${CONFIG_LTE_PHY_V4})
configure_file(include/modem_lte_config.h.in ${out_inc_dir}/modem_lte_config.h)

set(target modem_lte)

if(CONFIG_MODEM_LTE_LIB)
    file(GLOB nvm_headers ../../prebuilts/modem/8850/${CONFIG_MODEM_LTE_LIB_VARINTS}/ap_cp_syn/nvm_*)
    foreach(files ${nvm_headers})
        configure_file(${files} ${out_inc_dir} COPYONLY)
    endforeach()
    configure_file(../../prebuilts/modem/8850/${CONFIG_MODEM_LTE_LIB_VARINTS}/ap_cp_syn/ap_cp_share_symb.txt ${out_inc_dir} COPYONLY)
    set(modem_lte_lib ${out_lib_dir}/modem_lte_lib.a)
    configure_file(../../prebuilts/modem/8850/${CONFIG_MODEM_LTE_LIB_VARINTS}/ap_cp_syn/ps_ap_module.a ${modem_lte_lib} COPYONLY)
    add_app_libraries(${modem_lte_lib})
    add_library(modem_lte_lib STATIC IMPORTED GLOBAL)

include(modem_lte.cmake)


endif()