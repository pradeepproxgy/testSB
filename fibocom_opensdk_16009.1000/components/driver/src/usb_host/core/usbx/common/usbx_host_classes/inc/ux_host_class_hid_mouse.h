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
/**   HID Mouse Client Class                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    ux_host_class_hid_mouse.h                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains all the header and extern functions used by the  */
/*    USBX HID mouse class.                                               */
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

#ifndef UX_HOST_CLASS_HID_MOUSE_H
#define UX_HOST_CLASS_HID_MOUSE_H

/* Define HID Mouse Class constants.  */

#define UX_HOST_CLASS_HID_MOUSE_BUFFER_LENGTH 128
#define UX_HOST_CLASS_HID_MOUSE_USAGE_ARRAY_LENGTH 64

#define UX_HOST_CLASS_HID_MOUSE_BUTTON_1 0x00090001
#define UX_HOST_CLASS_HID_MOUSE_BUTTON_2 0x00090002
#define UX_HOST_CLASS_HID_MOUSE_BUTTON_3 0x00090003

#define UX_HOST_CLASS_HID_MOUSE_AXIS_X 0x00010030
#define UX_HOST_CLASS_HID_MOUSE_AXIS_Y 0x00010031

#define UX_HOST_CLASS_HID_MOUSE_BUTTON_1_PRESSED 0x01
#define UX_HOST_CLASS_HID_MOUSE_BUTTON_2_PRESSED 0x02
#define UX_HOST_CLASS_HID_MOUSE_BUTTON_3_PRESSED 0x04

#define UX_HOST_CLASS_HID_MOUSE_WHEEL 0x00010038

/* Define HID Mouse Class structure.  */

typedef struct UX_HOST_CLASS_HID_MOUSE_STRUCT
{

    ULONG ux_host_class_hid_mouse_state;
    UX_HOST_CLASS_HID *ux_host_class_hid_mouse_hid;
    USHORT ux_host_class_hid_mouse_id;
    SLONG ux_host_class_hid_mouse_x_position;
    SLONG ux_host_class_hid_mouse_y_position;
    ULONG ux_host_class_hid_mouse_buttons;
    SLONG ux_host_class_hid_mouse_wheel;

} UX_HOST_CLASS_HID_MOUSE;

/* Define HID Mouse Class function prototypes.  */

UINT _ux_host_class_hid_mouse_activate(UX_HOST_CLASS_HID_CLIENT_COMMAND *command);
VOID _ux_host_class_hid_mouse_callback(UX_HOST_CLASS_HID_REPORT_CALLBACK *callback);
UINT _ux_host_class_hid_mouse_deactivate(UX_HOST_CLASS_HID_CLIENT_COMMAND *command);
UINT _ux_host_class_hid_mouse_entry(UX_HOST_CLASS_HID_CLIENT_COMMAND *command);
UINT _ux_host_class_hid_mouse_buttons_get(UX_HOST_CLASS_HID_MOUSE *mouse_instance,
                                          ULONG *mouse_buttons);
UINT _ux_host_class_hid_mouse_position_get(UX_HOST_CLASS_HID_MOUSE *mouse_instance,
                                           SLONG *mouse_x_position,
                                           SLONG *mouse_y_position);
UINT _ux_host_class_hid_mouse_wheel_get(UX_HOST_CLASS_HID_MOUSE *mouse_instance,
                                        SLONG *mouse_wheel_movement);

/* Define HID Mouse Class API prototypes.  */

#define ux_host_class_hid_mouse_entry _ux_host_class_hid_mouse_entry
#define ux_host_class_hid_mouse_buttons_get _ux_host_class_hid_mouse_buttons_get
#define ux_host_class_hid_mouse_position_get _ux_host_class_hid_mouse_position_get
#define ux_host_class_hid_mouse_wheel_get _ux_host_class_hid_mouse_wheel_get

#endif
