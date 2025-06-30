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

configure_file(api/fibocom.h.in ${out_inc_dir}/fibocom.h)
configure_file(api/didi_app.h.in ${out_inc_dir}/didi_app.h)
set(target fibocom)
include(fibocom.cmake)
if(CONFIG_FIBOCOM_SWEEP_CODE)
    include(core.cmake)
    #target_link_libraries(${target} PRIVATE xdldecoder)
endif()
add_subdirectory_if_exist(fibocustom)
