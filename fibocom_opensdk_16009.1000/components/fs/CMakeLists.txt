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

configure_file(include/fs_config.h.in ${out_inc_dir}/fs_config.h)
install_headers(include/vfs.h OUTPUT ${out_inc_dir})

set(target fs)
include(fs.cmake)

add_subdirectory_if_exist(bdev)
add_subdirectory_if_exist(fatfs)
add_subdirectory_if_exist(fsmount)
add_subdirectory_if_exist(sffs)
if(CONFIG_FS_SUPPORT_NAND_SFS)
add_subdirectory_if_exist(nanddev)
add_subdirectory_if_exist(bsd)
add_subdirectory_if_exist(ftl)
add_subdirectory_if_exist(sfs)
endif()
