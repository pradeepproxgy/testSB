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

if (CONFIG_FIBOCOM_SWEEP_CODE_LIB_QINGY)
set(import_lib ${out_lib_dir}/libdecoder.a)
configure_file(sweep_code/lib/libdecoder.a ${import_lib} COPYONLY)
else()
set(import_lib ${out_lib_dir}/libdecoder_hh_y.a)
configure_file(${CONFIG_FIBOCOM_SWEEP_CODE_LIB_PATH} ${import_lib} COPYONLY)
endif()

add_app_libraries(${import_lib})
add_library(xdldecoder STATIC IMPORTED GLOBAL)
set_target_properties(xdldecoder PROPERTIES IMPORTED_LOCATION ${import_lib})

#set(import_lib ${out_lib_dir}/libxgdzbar.a)
#configure_file(sweep_code/lib/libxgdzbar.a ${import_lib} COPYONLY)
#add_app_libraries(${import_lib})
#add_library(xgddecoder STATIC IMPORTED GLOBAL)
#set_target_properties(xgddecoder PROPERTIES IMPORTED_LOCATION ${import_lib})

#set(import_lib ${out_lib_dir}/libXRtk.a)
#configure_file(racoon/racoon_qianxun/lib/unisoc/libXRtk.a ${import_lib} COPYONLY)
#add_app_libraries(${import_lib})
#add_library(qianxun3 STATIC IMPORTED GLOBAL)
#set_target_properties(qianxun3 PROPERTIES IMPORTED_LOCATION ${import_lib})

if (CONFIG_SENSOR_ICM4X6XX AND CONFIG_SENSOR_MML_IPL)
set(import_lib ${out_lib_dir}/ipl_libmml_arm_a5.a)
configure_file(didi/sensor/lib/ipl_libmml_arm_a5.a ${import_lib} COPYONLY)
#message(${import_lib})
add_app_libraries(${import_lib})
add_library(icmSensor STATIC IMPORTED GLOBAL)
set_target_properties(icmSensor PROPERTIES IMPORTED_LOCATION ${import_lib})
endif()


