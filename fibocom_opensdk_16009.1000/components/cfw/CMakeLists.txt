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

set(FUNC_SUPPORT_SIMLOCK ${CONFIG_FUNC_SUPPORT_SIMLOCK})
set(LTE_SUPPORT ${CONFIG_LTE_SUPPORT})
set(LTE_NBIOT_SUPPORT ${CONFIG_LTE_NBIOT_SUPPORT})
set(DEDICATED_BEARER_SUPPORT ${CONFIG_DEDICATED_BEARER_SUPPORT})
set(RPM_SUPPORT ${CONFIG_RPM_SUPPORT})
set(CSG_SUPPORT ${CONFIG_CSG_SUPPORT})
set(CFW_VOLTE_SUPPORT ${CONFIG_CFW_VOLTE_SUPPORT})
set(VOLTE_SUPPORT ${CONFIG_VOLTE_SUPPORT})
configure_file(include/cfw_config.h.in ${out_inc_dir}/cfw_config.h)

set(target cfw)

include(cfw.cmake)
include(core.cmake)

