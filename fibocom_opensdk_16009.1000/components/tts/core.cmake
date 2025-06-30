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

if(CONFIG_TTS_ENGLISH_SUPPORT)
set(import_lib ${out_lib_dir}/aisound50_EN.a)
configure_file(aisound/lib/aisound50_EN.a ${import_lib} COPYONLY)
add_app_libraries(${import_lib})
add_library(aisound50_EN STATIC IMPORTED GLOBAL)
set_target_properties(aisound50_EN PROPERTIES IMPORTED_LOCATION ${import_lib})
else()
set(import_lib ${out_lib_dir}/aisound50.a)
configure_file(aisound/lib/aisound50.a ${import_lib} COPYONLY)
add_app_libraries(${import_lib})
add_library(aisound50 STATIC IMPORTED GLOBAL)
set_target_properties(aisound50 PROPERTIES IMPORTED_LOCATION ${import_lib})
endif()

