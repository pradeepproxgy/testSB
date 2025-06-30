#!/bin/bash

TARGET=$1
TARGET_OUT=out/${TARGET}_debug
source tools/launch.sh $TARGET
tools/deliverpack.py load --keyword TARGET_OUT=$TARGET_OUT --inclusive 8850 prebuilts -- components/apploader/pack/core_filelist.xml list.json
tools/deliverpack.py pack --output core_sdk.tar.gz list.json



