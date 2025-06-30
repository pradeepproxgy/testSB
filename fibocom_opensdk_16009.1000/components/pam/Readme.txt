1.if used gnss module, the cmakelist should is write

configure_file(include/pam_config.h.in ${out_inc_dir}/pam_config.h)

set(target pam)
add_app_libraries($<TARGET_FILE:${target}>)

add_library(${target} STATIC
    src/pam_gnss_api.c
    src/pam_api.c
    src/pam_worker.c
)
set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_PAM)
target_include_directories(${target} PUBLIC include)
target_include_targets(${target} PRIVATE kernel cfw atr ats net mal gnss)

target_include_directories(${target} PRIVATE
	../kernel/include/
	../diag/include/
	../diag/src/
	../driver/include/
	../fs/include/
	../gnss/include/
	../gnss/include/inf/
	../gnss/src/adpt/inc/
	../gnss/src/green_eye2/ge2_libgps_mp/gps_lib/inc/
)

relative_glob(srcs include/*.h src/*.c src/*.h)
beautify_c_code(${target} ${srcs})

2.if used gnss ,the pam_worker.c line 348 change NULL,write gnss task