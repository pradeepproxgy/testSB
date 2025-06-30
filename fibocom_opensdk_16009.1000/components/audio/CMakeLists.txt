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

configure_file(include/audio_config.h.in ${out_inc_dir}/audio_config.h)

set(target audio)
    if(CONFIG_AUDIO_VOICE_ENABLE)
    set(vsi_amrnb_lib ${out_lib_dir}/libvsi_amrnb.a)
    configure_file(src/8850/amrcodec/libvsi_amrnb.a ${vsi_amrnb_lib} COPYONLY)
		add_app_libraries(${vsi_amrnb_lib})
		add_library(vsi_amrnb_lib STATIC IMPORTED GLOBAL)

    set(vsi_amrwb_lib ${out_lib_dir}/libvsi_amrwb.a)
    configure_file(src/8850/amrcodec/libvsi_amrwb.a ${vsi_amrwb_lib} COPYONLY)
		add_app_libraries(${vsi_amrwb_lib})
		add_library(vsi_amrwb_lib STATIC IMPORTED GLOBAL)
    endif()
include(core.cmake)
include(audio.cmake)
