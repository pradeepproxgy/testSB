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
/**   HID Remote Control Class                                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    ux_host_class_hid_remote_control.h                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains all the header and extern functions used by the  */
/*    USBX HID remote control class.                                      */
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

#ifndef UX_HOST_CLASS_HID_REMOTE_CONTROL_H
#define UX_HOST_CLASS_HID_REMOTE_CONTROL_H

/* Define HID Remote Control Class constants.  */

#define UX_HOST_CLASS_HID_REMOTE_CONTROL_BUFFER_LENGTH 128
#define UX_HOST_CLASS_HID_REMOTE_CONTROL_USAGE_ARRAY_LENGTH 64

/* Each item in usage array takes 4 bytes. Check memory bytes calculation overflow here.  */
#if UX_OVERFLOW_CHECK_MULC_ULONG(UX_HOST_CLASS_HID_REMOTE_CONTROL_USAGE_ARRAY_LENGTH, 4)
#error UX_HOST_CLASS_HID_REMOTE_CONTROL_USAGE_ARRAY_LENGTH is too large for memory allocation, please check
#endif

/* Define HID Remote Control Class structure.  */

typedef struct UX_HOST_CLASS_HID_REMOTE_CONTROL_STRUCT
{

    ULONG ux_host_class_hid_remote_control_state;
    UX_HOST_CLASS_HID *ux_host_class_hid_remote_control_hid;
    ULONG *ux_host_class_hid_remote_control_usage_array;
    ULONG *ux_host_class_hid_remote_control_usage_array_head;
    ULONG *ux_host_class_hid_remote_control_usage_array_tail;
} UX_HOST_CLASS_HID_REMOTE_CONTROL;

/* Define HID Remote Control Class function prototypes.  */

VOID _ux_host_class_hid_remote_control_callback(UX_HOST_CLASS_HID_REPORT_CALLBACK *callback);
UINT _ux_host_class_hid_remote_control_activate(UX_HOST_CLASS_HID_CLIENT_COMMAND *command);
UINT _ux_host_class_hid_remote_control_deactivate(UX_HOST_CLASS_HID_CLIENT_COMMAND *command);
UINT _ux_host_class_hid_remote_control_entry(UX_HOST_CLASS_HID_CLIENT_COMMAND *command);
UINT _ux_host_class_hid_remote_control_usage_get(UX_HOST_CLASS_HID_REMOTE_CONTROL *remote_control_instance, ULONG *usage, ULONG *value);

/* Define HID Keyboard Class API prototypes.  */

#define ux_host_class_hid_remote_control_entry _ux_host_class_hid_remote_control_entry
#define ux_host_class_hid_remote_control_usage_get _ux_host_class_hid_remote_control_usage_get

#endif
