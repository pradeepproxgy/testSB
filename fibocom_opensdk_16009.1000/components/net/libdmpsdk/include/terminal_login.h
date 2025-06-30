/*******************************************************************************
* Company		  :  unicom-IOT
* Author		  :  lwd
* Version		  :  V1.0
* Date			  :  2021-05-27
* Description	  :  terminal_login.h
********************************************************************************/

/********************************* Include File ********************************/
#ifndef _TERMINAL_LOGIN_H_
#define _TERMINAL_LOGIN_H_

/********************************* Macro Definition ****************************/

#define REGIST_TIMEOUT 500

/********************************* Type Definition *****************************/


/********************************* Variables ***********************************/

/********************************* Function ************************************/





int UNI_MQTT_TerminalRegSub(void *handle);

int UNI_MQTT_TerminalRegPub(void *handle);
int UNI_TerminalRegist(void *handle);



#endif
