/*******************************************************************************
 *
 * Copyright (c) 2014 Bosch Software Innovations GmbH, Germany.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * The Eclipse Distribution License is available at
 *    http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Bosch Software Innovations GmbH - Please refer to git log
 *
 *******************************************************************************/
/*
 * lwm2mclient.h
 *
 *  General functions of lwm2m test client.
 *
 *  Created on: 22.01.2015
 *  Author: Achim Kraus
 *  Copyright (c) 2015 Bosch Software Innovations GmbH, Germany. All rights reserved.
 */

#ifndef LWM2MCLIENT_H_
#define LWM2MCLIENT_H_

#include "liblwm2m.h"

extern int ecloud_g_reboot;

/*
 * object_device.c
 */
lwm2m_object_t * ecloud_get_object_device(void);
uint8_t ecloud_device_change(lwm2m_data_t * dataArray, lwm2m_object_t * objectP);
void ecloud_display_device_object(lwm2m_object_t * objectP);
/*
 * object_firmware.c
 */
lwm2m_object_t * ecloud_get_object_firmware(void);
void ecloud_display_firmware_object(lwm2m_object_t * objectP);
/*
 * object_location.c
 */
lwm2m_object_t * ecloud_get_object_location(void);
void ecloud_display_location_object(lwm2m_object_t * objectP);
/*
 * object_test.c
 */
#define TEST_OBJECT_ID 31024
lwm2m_object_t * ecloud_get_test_object(void);
void ecloud_display_test_object(lwm2m_object_t * objectP);
/*
 * object_server.c
 */
lwm2m_object_t * ecloud_get_server_object(int serverId, const char* binding, int lifetime, bool storing);
void ecloud_display_server_object(lwm2m_object_t * objectP);
void ecloud_copy_server_object(lwm2m_object_t * objectDest, lwm2m_object_t * objectSrc);

/*
 * object_connectivity_moni.c
 */
lwm2m_object_t * ecloud_get_object_conn_m(void);
uint8_t ecloud_connectivity_moni_change(lwm2m_data_t * dataArray, lwm2m_object_t * objectP);

/*
 * object_connectivity_stat.c
 */
extern lwm2m_object_t * ecloud_get_object_conn_s(void);
extern void ecloud_conn_s_updateTxStatistic(lwm2m_context_t * lwm2mH, uint16_t txDataByte, bool smsBased);
extern void ecloud_conn_s_updateRxStatistic(lwm2m_context_t * lwm2mH, uint16_t rxDataByte, bool smsBased);

/*
 * object_access_control.c
 */
lwm2m_object_t* acc_ctrl_create_object(void);
bool  acc_ctrl_obj_add_inst (lwm2m_object_t* accCtrlObjP, uint16_t instId,
                 uint16_t acObjectId, uint16_t acObjInstId, uint16_t acOwner);
bool  acc_ctrl_oi_add_ac_val(lwm2m_object_t* accCtrlObjP, uint16_t instId,
                 uint16_t aclResId, uint16_t acValue);
/*
 * lwm2mclient.c
 */
int ecloud_handle_value_changed(lwm2m_context_t* lwm2mH, lwm2m_uri_t* uri, int type, const char * value, size_t valueLength);
/*
 * system_api.c
 */
void init_value_change(lwm2m_context_t * lwm2m);
void system_reboot(void);

/*
 * object_security.c
 */
lwm2m_object_t * ecloud_get_security_object(int serverId, const char* serverUri, char * bsPskId, char * psk, uint16_t pskLen, bool isBootstrap);
char * ecloud_get_server_uri(lwm2m_object_t * objectP, uint16_t secObjInstID);
void ecloud_display_security_object(lwm2m_object_t * objectP);
void ecloud_copy_security_object(lwm2m_object_t * objectDest, lwm2m_object_t * objectSrc);


/*
 * object_temperature.c
 */
 lwm2m_object_t * get_temperature_object(float sensorValue, char *uints,
                                 float minMeasuredValue, float maxMeasuredValue,
                                 float minRangeValue, float maxRangeValue);

lwm2m_object_t * get_illuminance_object(float sensorValue, char *uints,
                                 float minMeasuredValue, float maxMeasuredValue,
                                 float minRangeValue, float maxRangeValue);

/* for watermeter object_watermeter.c  */
extern lwm2m_object_t * ecloud_get_object_watermeter(void);
void free_object_watermeter(lwm2m_object_t * object);
void display_watermeter_object(lwm2m_object_t * objectP);
uint8_t watermeter_change(lwm2m_data_t * dataArray, lwm2m_object_t * objectP);


int ecloud_lwm2mclient_main(int argc, char *argv[]);
extern void lwm2m_report_data(uint8_t *data, size_t length);
extern void lwm2m_report_at_command_result(int success);
#endif /* LWM2MCLIENT_H_ */
