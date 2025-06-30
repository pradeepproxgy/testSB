
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>

#include "osi_api.h"
#include "osi_log.h"

#include "at_command.h"
#include "at_response.h"
#include "at_parse.h"
#include "fibo_opencpu.h"

#define BUFFER_MAX_LEN 40

typedef struct{
int testcus;
char buffer[20];
}testcus_t;

testcus_t g_testcus;
char g_firmware_ver[BUFFER_MAX_LEN] = "1.00.00.00.001";

void atCmdHandleTESTCUS(atCommand_t *cmd)
{
   char buf[BUFFER_MAX_LEN+1] = {0};
   bool paramok = true;
   switch (cmd->type)
   {
       case AT_CMD_SET: //at+testcus=1,"test"
           g_testcus.testcus = atParamInt(cmd->params[0], &paramok);
           if (!paramok)
               RETURN_CME_ERR(cmd->engine, ERR_AT_CME_PARAM_INVALID);

           const char *test_str = atParamDefStr(cmd->params[1], "", &paramok);
               if (!paramok)
                   RETURN_CME_ERR(cmd->engine, ERR_AT_CME_PARAM_INVALID);
               if(strlen(test_str) >= BUFFER_MAX_LEN )
               RETURN_CME_ERR(cmd->engine, ERR_AT_CME_PARAM_INVALID);
               memcpy(g_testcus.buffer,test_str,strlen(test_str));
               RETURN_OK(cmd->engine);
       break;

       case AT_CMD_READ: //at+testcus?      
           sprintf(buf, "+TESTCUS:%d,%s",g_testcus.testcus,g_testcus.buffer);
           atCmdRespInfoText(cmd->engine, buf);
           atCmdRespOK(cmd->engine);
       break;

       case AT_CMD_EXE: //at+testcus
           sprintf(buf, "+TESTCUS:%d,%s",g_testcus.testcus,g_testcus.buffer);
           atCmdRespInfoText(cmd->engine, buf);
           atCmdRespOK(cmd->engine);
       break;

       case AT_CMD_TEST: //at+testcus=?
           sprintf(buf, "+TESTCUS:1-5,\"test\"");
           atCmdRespInfoText(cmd->engine, buf);
           atCmdRespOK(cmd->engine);
       break;

       default:
           RETURN_CME_ERR(cmd->engine, ERR_AT_CME_PARAM_INVALID);
       break;

   }

}


void atCmdHandleAPPIMGVER(atCommand_t *cmd)
{
   char buf[48] = {0};
   bool paramok = true;
   int type = 0;
   switch (cmd->type)
   {
       case AT_CMD_EXE: //at+appimgver
           sprintf(buf, "%s",g_firmware_ver);
           atCmdRespInfoText(cmd->engine, buf);
           atCmdRespOK(cmd->engine);
       break;

       case AT_CMD_SET: //at+testcus=x (x:1 or 2)

           type = atParamInt(cmd->params[0], &paramok);
           if (!paramok)
               RETURN_CME_ERR(cmd->engine, ERR_AT_CME_PARAM_INVALID);

           if(type == 1)// get firmare version
           {
               sprintf(buf, "%s",g_firmware_ver);
               atCmdRespInfoText(cmd->engine, buf);
           }
           else if(type == 2)//get app image version
           {
               char app_version[48];
               memset(app_version,0,48);
               fibo_app_version_get(app_version, 16);
               sprintf(buf, "%s",app_version);
               atCmdRespInfoText(cmd->engine, buf);
           }
           else
           {
               RETURN_CME_ERR(cmd->engine, ERR_AT_CME_PARAM_INVALID);
           }
           RETURN_OK(cmd->engine);
       break;

       default:
           RETURN_CME_ERR(cmd->engine, ERR_AT_CME_PARAM_INVALID);
       break;

   }

}
char chipid[32];
char * fibo_get_chip_unique_id(uint32_t *len)
{
   memset(chipid,0,sizeof(chipid));
   fibo_get_chip_id((UINT8 *)chipid);
   fibo_textTrace("chipid %s", chipid);
   *len = 16;
   return (char *)chipid;
}

void atCmdHandleGTCUSTDATAVER(atCommand_t *cmd)
{
   char buf[256] = {0};
   switch (cmd->type)
   {
       case AT_CMD_EXE: //at+GTCUSTDATAVER
           fibo_app_version_get(buf,255);
           atCmdRespInfoText(cmd->engine, buf);
           atCmdRespOK(cmd->engine);
       break;

       default:
           RETURN_CME_ERR(cmd->engine, ERR_AT_CME_PARAM_INVALID);
       break;
   }
}

extern void fibo_set_security_flag(void);
extern bool fibo_get_security_flag(void);
void atCmdHandleSECURE(atCommand_t * cmd)
{
    char result_buf[64] ={0};
    switch (cmd->type)
    {
        case AT_CMD_EXE:
            atCmdRespOK(cmd->engine);
            fibo_set_security_flag();
            break;
        case AT_CMD_READ:
        {
            bool en=fibo_get_security_flag();
            sprintf(result_buf, "+SECBOOTEN: %d", en);
            atCmdRespInfoText(cmd->engine, result_buf);
            atCmdRespOK(cmd->engine);
        }
        break;
        default:
            atCmdRespCmeError(cmd->engine, ERR_AT_CME_OPERATION_NOT_ALLOWED);
            break;
    }
}
