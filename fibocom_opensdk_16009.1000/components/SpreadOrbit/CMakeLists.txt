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

if(CONFIG_SUPPORT_GNSS)
if(CONFIG_GPS_SPREAD_ORBIT_SUPPORT)
configure_file(inc/SpreadOrbit_config.h.in ${out_inc_dir}/SpreadOrbit_config.h)
include(core.cmake)
endif()
endif()