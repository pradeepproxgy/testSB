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

/*! 	\file jcrypto.h
	\brief This file contains api for encryption and decryption of the data.
*/

#ifndef _JCRYPTO_H_
#define _JCRYPTO_H_

#if defined(__cplusplus)
 extern "C" {
#endif

/* Standard libraries. */
#include <stdint.h>


/* # defines */


/* Enums */


/* Structures */
/*! \brief It stores the parameters required for encryption and decryption. */
typedef struct
{
	unsigned char key[20];		/*!< Key used for cryptography. Used for both encryption and decryption. Used in ECB and GCM cryptography. */
	uint8_t length;			/*!< Length of the key used for cryptography. */
	unsigned char iv[20];		/*!< Initialization vector used for encryption and decryption. Used in GCM cryptography. */
	uint8_t ivLen; 			/*!< Length of initialization vector. */
	unsigned char addData[40];	/*!< Additional data to be passed. Used in GCM cryptography. */
	uint8_t addDataLen; 		/*!< Length of additional data. */
	unsigned char authTag[20];	/*!< Authentication tag. Used in GCM cryptography. */
	uint8_t authTagLen; 		/*!< Length of authentication tag. */
}jiot_client_PAL_Crypto_param_t;


/* Callback function definition. */


/* External variables */


/* External functions */
/*! \brief This function shall encrypt the plain bytes as per the input parameters using AES 128-bit ECB (Electronic Codebook) algorithm.

	\param in		In parameter - The plain bytes to be encrypted.
	\param inLen		In parameter - Number of plain bytes to be encrypted.
	\param param 		In parameter - Parameters required for encryption. Refer jiot_client_PAL_Crypto_param_t structure for more details. 
	\param out	 	Out parameter - cipher bytes as an output of encryption. 
	
	\return 		Return number of encrypted bytes copied in the out parameter out,
				-1 in case of invalid plain bytes i.e. invalid reference to No of bytes mentioned in inLen is 0, 
				-2 in case of invalid reference to copy cipher bytes i.e. invalid reference to out parameter out,
				-3 in case of invalid encryption key length,
				-4 In case of failed context initialization,
				-5 In case of failed initialization of the encryption operation,
				-6 In case of failed to provide the message to be encrypted and obtain the output,
				-7 In case of failed to finalize the encryption,
				-20 in case of any error in encryption,
				-128 if API not supported.
*/
extern int16_t jiot_client_PAL_Crypto_encryptAes128Ecb(unsigned char *in, uint16_t inLen, jiot_client_PAL_Crypto_param_t *param, unsigned char *out);

/*! \brief This function shall decrypt the cipher bytes as per the input parameters using AES 128-bit ECB (Electronic Codebook) algorithm.

	\param in		In parameter - The cipher bytes to be decrypted. 
	\param inLen		In parameter - Number of cipher bytes to be decrypted. 
	\param param 		In parameter - Parameters required for decryption. Refer jiot_client_PAL_Crypto_param_t structure for more details.
	\param out	 	Out parameter - Plain bytes as an output of decryption. 

	\return 		Return number of decrypted bytes copied in the out parameter out, 
				-1 In case of invalid cipher bytes i.e. invalid reference to No of bytes mentioned in inLen is 0,
				-2 In case of invalid reference to copy plain bytes i.e. invalid reference to out parameter out,
				-3 In case of invalid decryption key length,
				-4 In case of failed context initialization,
				-5 In case of failed initilization of the decryption operation,
				-6 In case of failed to provide the message to be decrypted and obtain the output,
				-7 In case of failed to finalize the decryption,
				-20 in case of any error in decryption, 
				-128 if API not supported.
*/
extern int16_t jiot_client_PAL_Crypto_decryptAes128Ecb(unsigned char *in, uint16_t inLen, jiot_client_PAL_Crypto_param_t *param, unsigned char *out);

/*! \brief This function shall encrypt the plain bytes as per the input parameters using AES 128-bit GCM (Galois Counter Mode) algorithm.

	\param in		In parameter - The plain bytes to be encrypted.
	\param inLen		In parameter - Number of plain bytes to be encrypted.
	\param param 		In parameter - Parameters required for encryption. Refer jiot_client_PAL_Crypto_param_t structure for more details.
	\param out	 	Out parameter - Cipher bytes as an output of encryption. 
	
	\return			Return number of encrypted bytes copied in the out parameter out,
				-1 in case of invalid reference to copy cipher bytes i.e. invalid reference to out parameter out,
				-2 in case of invalid encryption key length,
				-3 In case of failed context initialization,
				-4 In case of failed initialization of the encryption operation,
				-5 In case of failed to initialize the IV length,
				-6 In case of failed initialization of the encryption operation,
			       	-7 In case of failed to provide any addtional data,
				-8 In case of failed to provide the message to be encrypted and obtain the output,
				-9 In case of failed to finalize the encryption,
				-10 in case of Invalid expected tag length,
				-11 In case of failed to get a tag,
				-20 in case of any error in encryption.
				-128 if API not supported.
*/
extern int16_t jiot_client_PAL_Crypto_encryptAes128Gcm(unsigned char *in, uint16_t inLen, jiot_client_PAL_Crypto_param_t *param, unsigned char *out); 

/*! \brief This function shall decrypt the cipher bytes as per the input parameters using AES 128-bit GCM (Galois Counter Mode) algorithm.

	\param in		In parameter - The cipher bytes to be decrypted. 
	\param inLen		In parameter - Number of cipher bytes to be decrypted. 
	\param param 		In parameter - Parameters required for decryption. Refer jiot_client_PAL_Crypto_param_t structure for more details.	
	\param out	 	Out parameter - Plain bytes as an output of decryption.  
	
	\return 		Return number of decrypted bytes copied in the out parameter out,
       				-1 in case of invalid reference to copy plain bytes i.e. invalid reference to out parameter out,
				-2 in case of invalid decryption key length,
				-3 In case of failed context initilization,
				-4 In case of failed initilization of the decryption operation,
				-5 In case of failed to initialise the IV length,
				-6 In case of failed initilization of the decryption operation,
				-7 in case of invalid additional data length,
			       	-8 In case of failed to provide any addtional data,
				-9 in case of invalid length of authentication tag,
				-10 In case of failed to provide the message to be decrypted and obtain the output,
				-11 In case of failed to set the expected tag value,
				-12 In case of failed to provide the message to be decrypted and obtain the output if additional data is not provided,
				-13 In case of failed to finalize the decryption,
				-20 in case of any error in decryption else,
				-128 if API not supported.
*/
extern int16_t jiot_client_PAL_Crypto_decryptAes128Gcm(unsigned char *in, uint16_t inLen, jiot_client_PAL_Crypto_param_t *param, unsigned char *out); 

#if defined(__cplusplus)
}
#endif
#endif


