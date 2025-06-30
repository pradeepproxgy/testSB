#Copyright(C) 2018 RDA Technologies Limited and / or its affiliates("RDA").
#All rights reserved.
#
#This software is supplied "AS IS" without any warranties.
#RDA assumes no responsibility or liability for the use of the software,
#conveys no license or title under any patent, copyright, or mask work
#right to the product.RDA reserves the right to make changes in the
#software without notification.RDA also make no representation or
#warranty that such application will be suitable for the specified use
#without further testing or modification.

set(target openCPU_SRC)

configure_file(include/openCPU_config.h.in ${out_inc_dir}/openCPU_config.h)

if(CONFIG_OPENCPU_ENABLED)
    #To generate openCPU src library
    add_library(${target} STATIC)
    if(CONFIG_SOC_8811)
        target_sources(${target} PRIVATE
            8811/openCPU_entry_v8m.S
            8811/openCPU_start.c
            8811/openCPU_secure.c
            8811/openCPU_chip.c
            8811/openCPU_platform.c
        )
        target_sources(${target} PRIVATE
             src/openCPU_uart.c
             src/openCPU_mem.c
             src/openCPU_trace.c
             src/openCPU_override.c
             src/openCPU_sffs_port.c
             src/openCPU_spi_flash.c
             src/openCPU_vfs.c
             src/openCPU_irq_nvic.c
             src/openCPU_timer.c
             src/openCPU_time.c
             src/openCPU_fifo.c
             src/openCPU_shm.c
             src/openCPU_sleep.c
             src/openCPU_gpio.c
             src/openCPU_adc.c
             src/openCPU_gpt.c
             src/openCPU_i2c.c
             src/openCPU_infra.c
             src/openCPU_iomux_8811.c
             src/openCPU_pmic_intr_1811.c
             src/console/openCPU_console.c
             src/console/openCPU_cmd_mem.c
             src/console/openCPU_cmd_plat.c
             src/console/openCPU_cmd_vfs.c
             src/console/openCPU_cmd_lp.c
             src/console/openCPU_cmd_adc.c
             src/console/openCPU_cmd_infra.c
             src/console/linenoise/linenoise.c
        )

        set(DRV_SRC ../driver/src)
        target_sources(${target} PRIVATE
             ${DRV_SRC}/drv_adc_8811.c
             ${DRV_SRC}/drv_infra.c
             ${DRV_SRC}/drv_efuse_1811.c
        )
    else()
        target_sources(${target} PRIVATE
            8850/openCPU_entry_v7a.S
            8850/openCPU_start.c
            8850/openCPU_secure.c
            8850/openCPU_chip.c
            8850/openCPU_platform.c
        )

        target_sources(${target} PRIVATE
            src/openCPU_uart.c
            src/openCPU_mem.c
            src/openCPU_trace.c
            src/openCPU_irq_gic.c
            src/openCPU_override.c
            src/openCPU_sffs_port.c
            src/openCPU_spi_flash.c
            src/openCPU_sleep.c
            src/openCPU_vfs.c
            src/openCPU_timer.c
            src/openCPU_shm.c
            src/openCPU_time.c
            src/openCPU_fifo.c
            src/openCPU_infra.c
            src/openCPU_ifc.c
            src/openCPU_gpt_8850.c
            src/openCPU_gpio_8850.c
            src/openCPU_iomux_8850.c
            src/console/openCPU_console.c
            src/console/openCPU_cmd_adc.c
            src/console/openCPU_cmd_infra.c
            src/console/openCPU_cmd_gpt.c
            src/console/linenoise/linenoise.c
            src/console/openCPU_cmd_mem.c
            src/console/openCPU_cmd_plat.c
            src/console/openCPU_cmd_vfs.c
            src/console/openCPU_cmd_lp.c
        )
        set(DRV_SRC ../driver/src)
        target_sources(${target} PRIVATE
             ${DRV_SRC}/drv_efuse_8850.c
        )
    endif()

    set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
    target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_OPENCPU)
    if(CONFIG_SOC_8811)
    target_include_directories(${target} PUBLIC include include/8811)
    else()
    target_include_directories(${target} PUBLIC include include/8850)
    endif()
    target_include_directories(${target} PRIVATE src)
    target_include_targets(${target} PRIVATE hal driver calclib osi_lib fsmount sffs fs bdev)

    #To generate openCPU library for AP
    if(CONFIG_SOC_8811)
        set(target openCPU_AP)
        add_library(${target} STATIC
             8811/openCPU_platform_ap.c
             src/openCPU_shm.c
        )
        add_app_libraries($<TARGET_FILE:${target}>)
        set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
        target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_OPENCPU)
        target_include_directories(${target} PUBLIC include include/8811)
        target_include_targets(${target} PRIVATE hal driver calclib osi_lib fs)
    else()
        set(target openCPU_AP)
        add_library(${target} STATIC
             8850/openCPU_platform_ap.c
             src/openCPU_shm.c
        )
        add_app_libraries($<TARGET_FILE:${target}>)
        set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${out_lib_dir})
        target_compile_definitions(${target} PRIVATE OSI_LOG_TAG=LOG_TAG_OPENCPU)
        target_include_directories(${target} PUBLIC include include/8850)
        target_include_targets(${target} PRIVATE hal driver calclib osi_lib fs)
    endif()

    #To generate openCPU SDK library
    set(target openCPU_SDK)

    #To generate openCPU depends library
    set(opencpu_libs_in ${out_lib_dir}/libopenCPU_Depends.a)
    set_property(GLOBAL PROPERTY depends_libraries
        $<TARGET_FILE:openCPU_SRC>
        $<TARGET_FILE:hal>
        $<TARGET_FILE:calclib>
        $<TARGET_FILE:calclib_crc>
        $<TARGET_FILE:osi_lib>
        $<TARGET_FILE:fsmount>
        $<TARGET_FILE:sffs>
        $<TARGET_FILE:lz4>
        $<TARGET_FILE:bdev>
    )
    get_property(depends_libraries GLOBAL PROPERTY depends_libraries)
    add_custom_command(OUTPUT ${opencpu_libs_in}
        COMMAND python3 ${groupgen_py} --base ${BINARY_TOP_DIR} ${opencpu_libs_in} ${depends_libraries}
        DEPENDS ${groupgen_py} ${depends_libraries}
        WORKING_DIRECTORY ${BINARY_TOP_DIR}
    )
    add_custom_target(depends_${target} DEPENDS ${opencpu_libs_in})

    #pack all library to openCPU SDK library
    set(opencpu_sdk_out ${out_lib_dir}/libopenCPU_SDK.a)
    add_custom_command(OUTPUT ${opencpu_sdk_out}
        COMMAND python3 ${libs2one_py}
            --ar ${CROSS_COMPILE}ar
            -i ${opencpu_libs_in}
            -o ${opencpu_sdk_out}
        WORKING_DIRECTORY ${out_lib_dir}
        DEPENDS ${opencpu_libs_in}
        )
    add_custom_target(build_${target} DEPENDS ${opencpu_sdk_out})
    add_library(${target} STATIC IMPORTED GLOBAL)
    set_target_properties(${target} PROPERTIES IMPORTED_LOCATION ${opencpu_sdk_out})
    if(CONFIG_SOC_8811)
    target_include_directories(${target} INTERFACE include include/8811
        $<TARGET_PROPERTY:osi_lib,INTERFACE_INCLUDE_DIRECTORIES>)
    else()
    target_include_directories(${target} INTERFACE include include/8850
        $<TARGET_PROPERTY:osi_lib,INTERFACE_INCLUDE_DIRECTORIES>)
    endif()
    add_dependencies(${target} build_${target})
endif()

relative_glob(srcs include/*.h src/*.[ch])
beautify_c_code(${target} ${srcs})

