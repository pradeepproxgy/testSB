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

if(CONFIG_MODEM_LTE_LIB)
if(CONFIG_CFW_VOLTE_SUPPORT)
set(libname cfw_core_modem_lte_volte_lib)
else()
set(libname cfw_core_modem_lte_lib)
endif()
else()
set(libname cfw_core)
endif()

set(import_lib ${out_lib_dir}/lib${libname}.a)
configure_file(core/${CONFIG_SOC}/lib${libname}.a ${import_lib} COPYONLY)
add_app_libraries(${import_lib})
add_library(cfw_core STATIC IMPORTED GLOBAL)
set_target_properties(cfw_core PROPERTIES IMPORTED_LOCATION ${import_lib})

if((CONFIG_SOC_8811) OR (CONFIG_SOC_8821))
    set(import_lib ${out_lib_dir}/libmodem_core.a)
    configure_file(core/${CONFIG_SOC}/libmodem_core.a ${import_lib} COPYONLY)
    add_app_libraries(${import_lib})
    add_library(modem_core STATIC IMPORTED GLOBAL)
    set_target_properties(modem_core PROPERTIES IMPORTED_LOCATION ${import_lib})

    configure_file(${SOURCE_TOP_DIR}/components/hal/config/${CONFIG_SOC}/nbrom_for_link.ld ${nbrom_elf_ld})
    add_custom_target(nbrom_for_link DEPENDS ${nbrom_elf_ld})
endif()
