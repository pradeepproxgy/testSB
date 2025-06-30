#
#set core.cmake for alipay libiotext libiotsdk lib
#

set(import_lib ${out_lib_dir}/libiotext.a)
configure_file (../3rd/alipay/ant_iot_sdk/lib/UIS8850/libiotext.a ${import_lib} COPYONLY)
add_app_libraries(${import_lib})
add_library (iotext STATIC IMPORTED GLOBAL)
set_target_properties(iotext PROPERTIES IMPORTED_LOCATION ${import_lib})

set(import_lib ${out_lib_dir}/libiotsdk.a)
configure_file (../3rd/alipay/ant_iot_sdk/lib/UIS8850/libiotsdk.a ${import_lib} COPYONLY)
add_app_libraries(${import_lib})
add_library (iotsdk STATIC IMPORTED GLOBAL)
set_target_properties(iotsdk PROPERTIES IMPORTED_LOCATION ${import_lib})

