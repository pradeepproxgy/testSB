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

/*! 	\file jfile.h
	\brief This file contains the api for file related operation like write data in to file, read data into file, open the directory, file details and delete the file.
*/

#ifndef _JFILE_H_
#define _JFILE_H_

#if defined(__cplusplus)
 extern "C" {
#endif

/* Standard libraries. */
#include <stdint.h>


/* defines */

	 
/* Enums */
/*! \brief This enum indicate the file open operation status. */
typedef enum
{
	JIOT_PAL_FILE_FLAG_WRITE,		/*!< Open the file in write mode. Created file if it does not exist. */
	JIOT_PAL_FILE_FLAG_READ,		/*!< Open the file in read. */
	JIOT_PAL_FILE_FLAG_APPEND,		/*!< Open the file in append mode. Created file if it does not exist. */
	JIOT_PAL_FILE_FLAG_WRITEPLUS,		/*!< Open the file in read and write mode. Created file if it does not exist. */
	JIOT_PAL_FILE_FLAG_READPLUS,		/*!< Open the file in read and write mode. */
	JIOT_PAL_FILE_FLAG_APPENDPLUS,		/*!< Open the file in read and append mode. Created file if it does not exist. */
}jiot_client_PAL_File_flag_t;


/*! \brief This enum indicate the whence in which seek operation need to perform. */
typedef enum
{
	JIOT_PAL_FILE_WHENCE_START,		/*!< It denotes starting of the file. */
	JIOT_PAL_FILE_WHENCE_END,		/*!< It denotes end of the file. */
	JIOT_PAL_FILE_WHENCE_CUR,		/*!< It denotes file pointer's current position. */
}jiot_client_PAL_File_whence_t;


/* Structures */
/*! \brief This structure will represent the file details required by the applications. */
typedef struct
{
	uint32_t sizeofFile;			/*!< Total size of file in bytes. */
	uint32_t lastAccessTime;		/*!< Last access time of file in epoch. */
	uint32_t lastModificationTime;		/*!< Last time the file was modified in epoch. */
	uint32_t lastStatusChangeTime;		/*!< Last time the status of the file that is inode is changed. */
	uint32_t blockSizeIO;			/*!< This represents block size of the IO in this file system. */
	uint32_t noOfBlocksAllocated;		/*!< Number of blocks allocated to this file in 512 block size. */
}jiot_client_PAL_File_details_t;


/* Callback function definition. */


/* External variables */


/* External functions */
/*! \brief This function is used to create the directory.
	
  	\param directoryName	In parameter - Name of directory.
	
	\return 		Returns, 0 on successfully created the directory else,
 				-1 if invalid directory name,
				-2 fail to create directory,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_File_createDirectory(unsigned char *directoryName);

/*! \brief This function is uesd to get details of the directory. This function can also be used to simply check file status by passing second parameter as NULL.
	
  	\param directoryName	In parameter - This parameter indicate the directory name / path required to fetch the details.
	\param details		Out parameter - This will return details of the directory.
	
	\return 		Returns, 0 on success else,
 				-1 if invalid directory name,
				-2 if directory not present,
				-3 if directory details corrupt,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_File_getDirectoryDetails(unsigned char *directoryName, jiot_client_PAL_File_details_t *details);

/*! \brief This function is used to get details of the file like file size, time of creation etc. This function can also be used to simply check file status by passing second parameter as NULL.
	
  	\param fileName		In parameter - This parameter stats the file name required to fetch the details.
	\param details		Out parameter - This will return details of the file.
	
	\return 		Returns, 0 on success else,
 				-1 if invalid file name,
				-2 if file not present,
				-3 if file details corrupt,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_File_getFileDetails(unsigned char *fileName, jiot_client_PAL_File_details_t *details);

/*! \brief This function is used to delete the directory.
	
  	\param directoryName	In parameter - Name of the directory which is to be deleted.
	
	\return 		Returns, 0 on success else,
 				-1 if invalid directory name,
				-2 if directory not present,
				-3 if fail to remove,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_File_removeDirectory(unsigned char *directoryName);

/*! \brief This function is used to delete the file.
	
  	\param fileName		In parameter - Name of the file which is to be deleted.
	
	\return 		Returns, 0 on success else,
 				-1 if invalid file name,
				-2 if file not present,
				-3 if fail to remove,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_File_removeFile(unsigned char *fileName);

/*! \brief This function is used to open the file.
	
  	\param fileName		In parameter - Name of the file which is to be open.
	\param mode		In parameter - Indicate the mode in which file is to be open mentioned in jiot_client_PAL_File_flag_t.
	\param fd		Out parameter - Need to store the void pointer which indicate file discriptor.
	
	\return 		Returns, 0 on success else,
				-1 if invalid file name,
				-2 if invalid mode,
				-3 if fail to open,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_File_open(unsigned char *fileName, jiot_client_PAL_File_flag_t mode, void **fd);

/*! \brief This function will read a particular offset and no of bytes from the specified file. 
	
  	\param fd 		In parameter - This parameter specifies the file discriptor from which data to read.
	\param offset	 	In parameter - This parameter specified the offset from which the data has to be read.
	\param noOfBytes 	In parameter - This parameter specifies the number of bytes to be read from the specific offset.
	\param readBuffer	Out parameter - buffer in which the stored data will be read.
	
	\return 		Returns, total number of bytes which is to be read else,
				-1 if invalid file discriptor,
				-2 if invalid number of bytes to be read(noOfBytes),
				-3 if invalid read buffer,
				-4 if file details corrupt,
				-5 if file size less then number of bytes to be read,
				-6 if fail to open file,
				-7 if fail to jump on specific offset,
				-8 if fail to read,
				-128 if API not supported.
*/
extern int32_t jiot_client_PAL_File_read(void *fd, uint32_t offset, uint32_t noOfBytes, unsigned char *readBuffer);

/*! \brief This function writes the content in to the file.
	
  	\param fd 		In parameter - This parameter specifies the file discriptor to which data to write.
	\param offset	 	In parameter - This parameter specified the offset to which the data has to be written.
	\param noOfBytes 	In parameter - This parameter specifies the number of bytes to be written from the specific offset.
	\param writeBuffer	In parameter - This parameter is an in parameter buffer from which the data will be written.
	
	\return 		Returns, total number of bytes to be writen else,
				-1 if invalid file discriptor,
				-2 if invalid number of bytes to be write(noOfBytes),
				-3 if invalid write buffer,
				-4 if file details corrupt,
				-5 if file size less then number of bytes to be write,
				-6 if fail to open file,
				-7 if fail to jump on specific offset,
				-8 if fail to write,
				-128 if API not supported.
*/
extern int32_t jiot_client_PAL_File_write(void *fd, uint32_t offset, uint32_t noOfBytes, unsigned char *writeBuffer);

/*! \brief This function is used for reposition a stream.
	
  	\param fd 		In parameter - This parameter specifies the file discriptor to which data is to be seek.
	\param offset		In parameter - Indicate the offset by which fd has to be moved.
	\param whence		In parameter - Indicate the mode mentioned in jiot_client_PAL_File_whence_t for reposition a file discriptor.
	
	\return 		Returns, 0 on success,
				-1 if invalid file discriptor,
				-2 if fail to jump on specific offset,
				-3 if seek fails,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_File_seek(void *fd, uint32_t offset, jiot_client_PAL_File_whence_t whence);

/*! \brief Function obtains the current value of the file position indicator.
	
  	\param fd 		In parameter - This parameter specifies the file discriptor.
	
	\return 		Returns, the current offset,
				-1 if invalid file discriptor,
				-2 if tell fails,
				-128 if API not supported.
*/
extern int32_t jiot_client_PAL_File_tell(void *fd);

/*! \brief This function is used to close the file.
	
  	\param fd 		In parameter - This parameter specifies the file discriptor.
	
	\return 		Returns, 0 on success,
				-1 if invalid file discriptor,
				-2 if close fail,
				-128 if API not supported.
*/
extern int8_t jiot_client_PAL_File_close(void *fd);


#if defined(__cplusplus)
}
#endif
#endif

