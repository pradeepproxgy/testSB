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

add_subdirectory_if_exist(appstart)
add_subdirectory_if_exist(apploader)
add_subdirectory_if_exist(atr)
add_subdirectory_if_exist(ats)
add_subdirectory_if_exist(audio)
add_subdirectory_if_exist(aworker)
add_subdirectory_if_exist(bootloader)
add_subdirectory_if_exist(calclib)
add_subdirectory_if_exist(cfw)
add_subdirectory_if_exist(mal)
add_subdirectory_if_exist(diag)
add_subdirectory_if_exist(driver)
add_subdirectory_if_exist(lcdpanel)
add_subdirectory_if_exist(firmwareupdate)
add_subdirectory_if_exist(fs)
add_subdirectory_if_exist(hal)
add_subdirectory_if_exist(kernel)
add_subdirectory_if_exist(libjpeg-turbo)
add_subdirectory_if_exist(ml)
add_subdirectory_if_exist(modem)
add_subdirectory_if_exist(modem_lte)
add_subdirectory_if_exist(nanopb)
add_subdirectory_if_exist(net)
add_subdirectory_if_exist(nvm)
add_subdirectory_if_exist(rpc)
add_subdirectory_if_exist(service)
add_subdirectory_if_exist(unity)
add_subdirectory_if_exist(halo_rec)
add_subdirectory_if_exist(misc)
if(CONFIG_SOC_8910)
add_subdirectory_if_exist(connectivity)
endif()
add_subdirectory_if_exist(tts)
add_subdirectory_if_exist(littlevgl)
add_subdirectory_if_exist(micropython)
add_subdirectory_if_exist(crypto)
add_subdirectory_if_exist(lz4)
add_subdirectory_if_exist(tb)
add_subdirectory_if_exist(lpa)
if(CONFIG_SOC_8910)
add_subdirectory_if_exist(bt_app)
endif()
if(CONFIG_SOC_8910 OR CONFIG_SOC_8811)
add_subdirectory_if_exist(console)
endif()
add_subdirectory_if_exist(tfm)
add_subdirectory_if_exist(tf-m-tests)
if(CONFIG_SOC_8850)
add_subdirectory_if_exist(pam)
add_subdirectory_if_exist(gnss)
add_subdirectory_if_exist(SpreadOrbit)
endif()

if (CONFIG_8850CM_TARGET)
add_subdirectory_if_exist(simlock)
endif()

if((CONFIG_SOC_8811) OR (CONFIG_SOC_8850) OR (CONFIG_SOC_8821))
add_subdirectory_if_exist(openCPU)
endif()
add_subdirectory_if_exist(fibo_logo)