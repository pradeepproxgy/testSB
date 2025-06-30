####################=========================copy  nv=========================######################
echo $BUILD_TARGET
LOCATION="prebuilts/modem/8850"

nvmdir=`cat target/$BUILD_TARGET/target.config|grep CONFIG_NVM_VARIANTS|awk -F= '{print $2}'`
nvmname=`echo ${nvmdir:1:-1}`
echo "nvm name is ${nvmname}"


cp -rf target/$BUILD_TARGET/nv/* $LOCATION/${nvmname}/

echo $LINENO

