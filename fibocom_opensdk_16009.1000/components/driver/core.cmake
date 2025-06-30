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

if(CONFIG_IPC_PLAT_8850CM_V2)
set(libname driver_core_V2)
else()
set(libname driver_core)
endif()

set(import_lib ${out_lib_dir}/lib${libname}.a)
configure_file(core/${CONFIG_SOC}/lib${libname}.a ${import_lib} COPYONLY)
add_app_libraries(${import_lib})
add_library(${libname} STATIC IMPORTED GLOBAL)
set_target_properties(${libname} PROPERTIES IMPORTED_LOCATION ${import_lib})
