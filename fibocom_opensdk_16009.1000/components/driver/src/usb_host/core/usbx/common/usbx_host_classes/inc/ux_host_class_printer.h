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
/**   Printer Class                                                       */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    ux_host_class_printer.h                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains all the header and extern functions used by the  */
/*    USBX printer class.                                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*  09-30-2020     Chaoqiong Xiao           Modified comment(s),          */
/*                                            used UX prefix to refer to  */
/*                                            TX symbols instead of using */
/*                                            them directly,              */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/

#ifndef UX_HOST_CLASS_PRINTER_H
#define UX_HOST_CLASS_PRINTER_H

/* Define Printer Class constants.  */

#define UX_HOST_CLASS_PRINTER_CLASS_TRANSFER_TIMEOUT 300000
#define UX_HOST_CLASS_PRINTER_CLASS 7
#define UX_HOST_CLASS_PRINTER_SUBCLASS 1
#define UX_HOST_CLASS_PRINTER_PROTOCOL_BI_DIRECTIONAL 2
#define UX_HOST_CLASS_PRINTER_GET_STATUS 1
#define UX_HOST_CLASS_PRINTER_SOFT_RESET 2
#define UX_HOST_CLASS_PRINTER_STATUS_LENGTH 4
#define UX_HOST_CLASS_PRINTER_DESCRIPTOR_LENGTH 1024
#define UX_HOST_CLASS_PRINTER_GET_DEVICE_ID 0
#define UX_HOST_CLASS_PRINTER_NAME_LENGTH 64

/* Define Printer Class 1284 descriptor tag constants.  */

#define UX_HOST_CLASS_PRINTER_TAG_DESCRIPTION "DESCRIPTION:"
#define UX_HOST_CLASS_PRINTER_TAG_DES "DES:"

/* Define Printer Class string constants.  */

#define UX_HOST_CLASS_PRINTER_GENERIC_NAME "USB PRINTER"

/* Define Printer Class structure.  */

typedef struct UX_HOST_CLASS_PRINTER_STRUCT
{

    struct UX_HOST_CLASS_PRINTER_STRUCT
        *ux_host_class_printer_next_instance;
    UX_HOST_CLASS *ux_host_class_printer_class;
    UX_DEVICE *ux_host_class_printer_device;
    UX_INTERFACE *ux_host_class_printer_interface;
    UX_ENDPOINT *ux_host_class_printer_bulk_out_endpoint;
    UX_ENDPOINT *ux_host_class_printer_bulk_in_endpoint;
    UINT ux_host_class_printer_state;
    UCHAR ux_host_class_printer_name[UX_HOST_CLASS_PRINTER_NAME_LENGTH];
    UX_SEMAPHORE *ux_host_class_printer_semaphore;
} UX_HOST_CLASS_PRINTER;

/* Define Printer Class function prototypes.  */

UINT _ux_host_class_printer_activate(UX_HOST_CLASS_COMMAND *command);
UINT _ux_host_class_printer_configure(UX_HOST_CLASS_PRINTER *printer);
UINT _ux_host_class_printer_deactivate(UX_HOST_CLASS_COMMAND *command);
UINT _ux_host_class_printer_endpoints_get(UX_HOST_CLASS_PRINTER *printer);
UINT _ux_host_class_printer_entry(UX_HOST_CLASS_COMMAND *command);
UINT _ux_host_class_printer_name_get(UX_HOST_CLASS_PRINTER *printer);
UINT _ux_host_class_printer_device_id_get(UX_HOST_CLASS_PRINTER *printer, UCHAR *descriptor_buffer, ULONG length);
UINT _ux_host_class_printer_read(UX_HOST_CLASS_PRINTER *printer, UCHAR *data_pointer,
                                 ULONG requested_length, ULONG *actual_length);
UINT _ux_host_class_printer_soft_reset(UX_HOST_CLASS_PRINTER *printer);
UINT _ux_host_class_printer_status_get(UX_HOST_CLASS_PRINTER *printer, ULONG *printer_status);
UINT _ux_host_class_printer_write(UX_HOST_CLASS_PRINTER *printer, UCHAR *data_pointer,
                                  ULONG requested_length, ULONG *actual_length);

/* Define Printer Class API prototypes.  */

#define ux_host_class_printer_activate _ux_host_class_printer_activate
#define ux_host_class_printer_name_get _ux_host_class_printer_name_get
#define ux_host_class_printer_device_id_get _ux_host_class_printer_device_id_get
#define ux_host_class_printer_read _ux_host_class_printer_read
#define ux_host_class_printer_soft_reset _ux_host_class_printer_soft_reset
#define ux_host_class_printer_status_get _ux_host_class_printer_status_get
#define ux_host_class_printer_write _ux_host_class_printer_write

#endif
