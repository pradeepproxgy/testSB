/**
 * Reliance JioInfocomm (RJIL) Ltd. CONFIDENTIAL
 * Copyright (c) 2020 All rights reserved.
 *
 * This source code is proprietory and owned by RJIL.
 *
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by RJIL Ltd.
 * No part of the Material may be used,copied,
 * reproduced, modified, published, uploaded,posted, transmitted,
 * distributed, or disclosed in any way without RJIL Ltd prior written permission.
 *
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by RJIL Ltd. in writing.
 */

/*! 	\file jgpio.h
	\brief This file contains the api for setting the getting the gpio related status like setting the DO, getting the DI / DO status.
*/

#ifndef _JGPIO_H_
#define _JGPIO_H_

#if defined(__cplusplus)
 extern "C" {
#endif

/* Standard libraries. */
#include <stdint.h>

/* # defines */
#define JIOT_PAL_GPIO_IO_MAX_CNT	109		/*!< Maximum number of input and output supported. */


/* Enums */
/*! \brief Indicating the GPIO information related id. */
typedef enum
{
	JIOT_PAL_GPIO_DI_CONN,			/*!< Indicate id if IO is connected. */
	JIOT_PAL_GPIO_DI_DISC,			/*!< Indicate id if IO is disconnect. */
	JIOT_PAL_GPIO_DI_HIGH,			/*!< Indicate if DI is high. */
	JIOT_PAL_GPIO_DI_LOW,			/*!< Indicate if DI is low. */
	JIOT_PAL_GPIO_UNKNOWN,			/*!< Indicate invalid IO. */
}jiot_client_PAL_Gpio_info_t;

/*! \brief Set GPIO type, used for pre-configuring GPIO type. */
typedef enum
{
	JIOT_PAL_GPIO_TYPE_DI,				/*!< Indicate GPIO as digital input. */
	JIOT_PAL_GPIO_TYPE_INT_DI,			/*!< Indicate GPIO as interrupt. */
	JIOT_PAL_GPIO_TYPE_DO,				/*!< Indicate GPIO as digital output. */
}jiot_client_PAL_Gpio_type_t;


/* Structures */


/* Callback function definition. */
typedef void (*jiot_client_PAL_Gpio_cb_t)(uint8_t status, void *data);	/*!< Callback function which is used to inform the status of the digital input / output to application when change occured.
1. uint8_t status: indicate the jiot_client_PAL_DevStatus_info_t status,
2. void * data:  indicate the data of the respective enum.
*/


/* External variables */


/* External functions */
/*! \brief It shall update information of the GPIO's supported.
 
	\param gpio	 	In parameter - Gpio number eg: Gpio1, Gpio2, Gpio3, Gpio4.
	\param type		In parameter - Type indicates whether that gpio is input(DI), output(DO) or interrupt as mentioned in jiot_client_PAL_Gpio_type_t.

	\return 		Returns 0 on success or already configure else,
				-1 if the GPIO number is invalid,
				-2 for invalid GPIO type,
				-3 if fails to read GPIO info from device,
				-4 in case of invalid GPIO type in device,
				-5 if configuration change and reboot required only in case of gpio change to interrupt or normal GPIO,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_Gpio_config(uint8_t gpio, jiot_client_PAL_Gpio_type_t type);

/*! \brief It shall return the status of the specified digital input.

	\param gpio	 	In parameter - Gpio number eg: Gpio1, Gpio2, Gpio3, Gpio4.

	\return 		Returns either 1 or 0 as per status of the digital input else,
				-1 if the GPIO number is invalid,
				-2 if fail to get GPIO value,
				-3 if the GPIO can't be get due it's type,
				-4 if the GPIO not configured,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_Gpio_getDiStatus(uint8_t gpio);

/*! \brief It shall return the status of the specified digital output.

	\param gpio	 	In parameter - Gpio number eg: Gpio1, Gpio2, Gpio3, Gpio4.

	\return 		Returns either 1 or 0 as per status of the digital output else,
				-1 if the GPIO number is invalid,
				-2 if fail to get GPIO value,
				-3 if the GPIO can't be get due it's type,
				-4 if the GPIO not configured,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_Gpio_getDoStatus(uint8_t gpio);

/*! \brief This function shall set the digital output with the specified status.

	\param gpio	 	In parameter - Gpio number eg: Gpio1, Gpio2, Gpio3, Gpio4.
	\param status		In parameter - Value to be set on DO / Status to be set.

	\return 		Returns 0 on success else,
				-1 if the GPIO number is invalid,
				-2 if fail to set GPIO value,
				-3 if the GPIO can't be set due it's type,
				-4 if the GPIO not configured,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_Gpio_setDoStatus(uint8_t gpio, int8_t status);

/*! \brief It shall register the callback for both digital input and output.
 
	\param cb	 	Registering the callback for handler.

	\return 		Returns nothing.
*/
extern void jiot_client_PAL_Gpio_registerCb(jiot_client_PAL_Gpio_cb_t cb);


#if defined(__cplusplus)
}
#endif
#endif


