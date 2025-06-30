#!/bin/bash
# Copyright (C) 2019 RDA Technologies Limited and/or its affiliates("RDA").
# All rights reserved.
#
# This software is supplied "AS IS" without any warranties.
# RDA assumes no responsibility or liability for the use of the software,
# conveys no license or title under any patent, copyright, or mask work
# right to the product. RDA reserves the right to make changes in the
# software without notification.  RDA also make no representation or
# warranty that such application will be suitable for the specified use
# without further testing or modification.

set -e

EM_SS="8850EM_cat1bis_ab"
CM_SS="8850CM_cat1bis"
BM_SS="8850XM_cat1bis_ab"
BM_DS="8850XM_cat1bis_ds_ab"
DG_DS="8850XG_cat1bis_gnss_ds_ab"
DG_SS="8850XG_cat1bis_gnss_ab"
DG_SS_VOLTE="8850XG_cat1bis_volte_gnss_ab"
DG_DS_VOLTE="8850XG_cat1bis_volte_gnss_ds_ab"
DEBUG="_debug"

build_target() {
        source tools/launch.sh $1
        cout
        cmake ../.. -G Ninja
        ninja
        croot
        cd tools/linux
        echo $TARGET
        if [[ $TARGET == "MC665-CN-CM" ]];then
            echo $TARGET
            dtools pacmerge --id APPIMG,FLASH  ../../out/$TARGET$DEBUG/hex/$TARGET"-"$CM_SS"-debug".pac ../../out/$TARGET"_debug"/hex/examples/appimg_flash.pac  ../../output.pac
        fi
        cd ../../
}

for TARGET in $(
        cd target
        /bin/ls
); do
        if test -f target/$TARGET/target.config; then
                case $TARGET in
                8910DM_License)
                        echo "$TARGET in known not work"
                        ;;
                *)
                        build_target $TARGET
                        ;;
                esac
        fi
done
echo "build success"