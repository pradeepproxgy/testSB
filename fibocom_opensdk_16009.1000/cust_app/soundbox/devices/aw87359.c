#include "aw87359.h"
#include "fibo_opencpu.h"
#include "fb_config.h"

#if AW87359_EN
#define AW87359_CHIP_ID     0x59
#define AW87359_ADDR        0x58

int AW87359_init(void)
{ 
    char ucAddr[22] = {0x70,0x01,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x01};
    char ucVal[22] = {0x80,0x00,0x00,0x0c,0x08,0x05,0x10,0x0f,0x4e,0x08,0x08,0x4a,0xbb,0x80,0x29,0x58,0xcd,0x3c,0x2f,0x07,0xdb,0x0d};
    int i = 0;
    char chipId = 0;
    int ret = 0;

    HAL_I2C_BPS_T bps;

    bps.i2c_name=1;
    bps.value=0;
    ret = i2c_open(bps);
    if(ret<0)
    {
        //i2c open fail
        return ret;
    }

    // addr  0x58   //CHIP ID 0x59
    ret=i2c_get_data((UINT32)AW87359_ADDR,(UINT32)0x00,0,(UINT8 *)&chipId,1);
    if(ret<0)
    {
        //get device id fail
        return ret;
    }
   if(AW87359_CHIP_ID!=chipId)
   {
       //chip_id error
       return -1;
   }

    fibo_taskSleep(100);
    for(i=0;i<22;i++)
    {
        i2c_send_data((UINT32)0x58,(UINT32)ucAddr[i],0,(UINT8 *)&ucVal[i],1);
        fibo_taskSleep(10);
    }

    return 0;
}

#endif