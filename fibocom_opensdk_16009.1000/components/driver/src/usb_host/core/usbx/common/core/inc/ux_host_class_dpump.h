/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** USBX Component                                                        */
/**                                                                       */
/**   Host Data Pump Class                                                */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    ux_host_class_dpump.h                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains all the header and extern functions used by the  */
/*    USBX demo data pump class.                                          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*  09-30-2020     Chaoqiong Xiao           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/

#ifndef UX_HOST_CLASS_DPUMP_H
#define UX_HOST_CLASS_DPUMP_H

/* Define Data Pump Class constants.  */

#define UX_HOST_CLASS_DPUMP_CLASS_TRANSFER_TIMEOUT 300000
#define UX_HOST_CLASS_DPUMP_CLASS 0x99
#define UX_HOST_CLASS_DPUMP_SUBCLASS 0x99
#define UX_HOST_CLASS_DPUMP_PROTOCOL 0x99

/* Define Data Pump Class packet equivalences.  */
#define UX_HOST_CLASS_DPUMP_PACKET_SIZE 128

/* Define Data Pump Class Ioctl functions.  */
#define UX_HOST_CLASS_DPUMP_SELECT_ALTERNATE_SETTING 1

/* Define Data Pump Class string constants.  */

#define UX_HOST_CLASS_DPUMP_GENERIC_NAME "USB DPUMP"

/* Define Printer Class function prototypes.  */

UINT _ux_host_class_dpump_activate(UX_HOST_CLASS_COMMAND *command);
UINT _ux_host_class_dpump_configure(UX_HOST_CLASS_DPUMP *dpump);
UINT _ux_host_class_dpump_deactivate(UX_HOST_CLASS_COMMAND *command);
UINT _ux_host_class_dpump_endpoints_get(UX_HOST_CLASS_DPUMP *dpump);
UINT _ux_host_class_dpump_entry(UX_HOST_CLASS_COMMAND *command);
UINT _ux_host_class_dpump_read(UX_HOST_CLASS_DPUMP *dpump, UCHAR *data_pointer,
                               ULONG requested_length, ULONG *actual_length);
UINT _ux_host_class_dpump_write(UX_HOST_CLASS_DPUMP *dpump, UCHAR *data_pointer,
                                ULONG requested_length, ULONG *actual_length);
UINT _ux_host_class_dpump_ioctl(UX_HOST_CLASS_DPUMP *dpump, ULONG ioctl_function,
                                VOID *parameter);

#define ux_host_class_dpump_entry _ux_host_class_dpump_entry
#define ux_host_class_dpump_read _ux_host_class_dpump_read
#define ux_host_class_dpump_write _ux_host_class_dpump_write
#define ux_host_class_dpump_ioctl _ux_host_class_dpump_ioctl

#endif
