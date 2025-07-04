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
/**   Host Stack                                                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    ux_host_stack.h                                     PORTABLE C      */
/*                                                           6.1.4        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains all the header and extern functions used by the  */
/*    USBX Host Stack component.                                          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*  09-30-2020     Chaoqiong Xiao           Modified comment(s),          */
/*                                            added uninitialize APIs,    */
/*                                            optimized based on compile  */
/*                                            definitions,                */
/*                                            resulting in version 6.1    */
/*  02-02-2021     Chaoqiong Xiao           Modified comment(s),          */
/*                                            added configuration activate*/
/*                                            and deactivate support,     */
/*                                            added host device string    */
/*                                            descriptor get support,     */
/*                                            updated internal function,  */
/*                                            resulting in version 6.1.4  */
/*                                                                        */
/**************************************************************************/

#ifndef UX_HOST_STACK_H
#define UX_HOST_STACK_H

/* Define Host Stack component function prototypes.  */

#if UX_MAX_DEVICES > 1
VOID _ux_host_stack_bandwidth_release(UX_HCD *hcd, UX_ENDPOINT *endpoint);
VOID _ux_host_stack_bandwidth_claim(UX_HCD *hcd, UX_ENDPOINT *endpoint);
UINT _ux_host_stack_bandwidth_check(UX_HCD *hcd, UX_ENDPOINT *endpoint);
#else
#define _ux_host_stack_bandwidth_release(a, b)
#define _ux_host_stack_bandwidth_claim(a, b)
#define _ux_host_stack_bandwidth_check(a, b) (UX_SUCCESS)
#endif

UINT _ux_host_stack_application_call(ULONG state, UX_HOST_CLASS *class, VOID *para);
UINT _ux_host_stack_application_register(UCHAR *class_name, UINT (*app_start)(void *), UINT (*app_stop)(void *));
UX_HOST_CLASS *_ux_host_stack_class_call(UX_HOST_CLASS_COMMAND *class_command);
UINT _ux_host_stack_class_device_scan(UX_DEVICE *device);
UINT _ux_host_stack_class_get(UCHAR *class_name, UX_HOST_CLASS **class);
UINT _ux_host_stack_class_instance_destroy(UX_HOST_CLASS *class, VOID *class_instance);
UINT _ux_host_stack_class_instance_create(UX_HOST_CLASS *class, VOID *class_instance);
UINT _ux_host_stack_class_instance_get(UX_HOST_CLASS *class, UINT class_index, VOID **class_instance);
UINT _ux_host_stack_class_instance_verify(UCHAR *class_name, VOID *class_instance);
UINT _ux_host_stack_class_interface_scan(UX_DEVICE *device);
UINT _ux_host_stack_class_register(UCHAR *class_name,
                                   UINT (*class_entry_function)(struct UX_HOST_CLASS_COMMAND_STRUCT *));
UINT _ux_host_stack_class_unregister(UINT (*class_entry_function)(struct UX_HOST_CLASS_COMMAND_STRUCT *));
UINT _ux_host_stack_configuration_descriptor_parse(UX_DEVICE *device, UX_CONFIGURATION *configuration, UINT configuration_index);
UINT _ux_host_stack_configuration_enumerate(UX_DEVICE *device);
UINT _ux_host_stack_configuration_instance_create(UX_CONFIGURATION *configuration);
VOID _ux_host_stack_configuration_instance_delete(UX_CONFIGURATION *configuration);
UINT _ux_host_stack_configuration_interface_get(UX_CONFIGURATION *configuration,
                                                UINT interface_index, UINT alternate_setting_index,
                                                UX_INTERFACE **interface);
UINT _ux_host_stack_configuration_interface_scan(UX_CONFIGURATION *configuration);
UINT _ux_host_stack_configuration_set(UX_CONFIGURATION *configuration);
VOID _ux_host_stack_delay_ms(ULONG time);
UINT _ux_host_stack_device_address_set(UX_DEVICE *device);
UINT _ux_host_stack_device_configuration_activate(UX_CONFIGURATION *configuration);
UINT _ux_host_stack_device_configuration_deactivate(UX_DEVICE *device);
UINT _ux_host_stack_device_configuration_get(UX_DEVICE *device, UINT configuration_index,
                                             UX_CONFIGURATION **configuration);
UINT _ux_host_stack_device_configuration_select(UX_CONFIGURATION *configuration);
UINT _ux_host_stack_device_configuration_reset(UX_DEVICE *device);
UINT _ux_host_stack_device_descriptor_read(UX_DEVICE *device);
UINT _ux_host_stack_string_descriptor_read(UX_DEVICE *device, unsigned char index, void *buf, int *size);
UINT _ux_host_stack_control_msg(UX_DEVICE *device, u8 request,
                                u8 request_type, u16 value, u16 index, void *buf, int *size);
UINT _ux_host_stack_device_get(ULONG device_index, UX_DEVICE **device);
UINT _ux_host_stack_device_string_get(UX_DEVICE *device, UCHAR *descriptor_buffer, ULONG length, ULONG language_id, ULONG string_index);
UINT _ux_host_stack_device_remove(UX_HCD *hcd, UX_DEVICE *parent, UINT port_index);
UINT _ux_host_stack_device_resources_free(UX_DEVICE *device);
UINT _ux_host_stack_endpoint_instance_create(UX_ENDPOINT *endpoint);
VOID _ux_host_stack_endpoint_instance_delete(UX_ENDPOINT *endpoint);
UINT _ux_host_stack_endpoint_reset(UX_ENDPOINT *endpoint);
UINT _ux_host_stack_endpoint_transfer_abort(UX_ENDPOINT *endpoint);
VOID _ux_host_stack_enum_thread_entry(ULONG input);
UINT _ux_host_stack_hcd_register(UCHAR *hcd_name,
                                 UINT (*hcd_init_function)(struct UX_HCD_STRUCT *), ULONG hcd_param1, ULONG hcd_param2);
UINT _ux_host_stack_hcd_unregister(UCHAR *hcd_name, ULONG hcd_param1, ULONG hcd_param2);
VOID _ux_host_stack_hcd_thread_entry(ULONG input);
UINT _ux_host_stack_hcd_transfer_request(UX_TRANSFER *transfer_request);
UINT _ux_host_stack_initialize(UINT (*ux_system_host_change_function)(ULONG, UX_HOST_CLASS *, VOID *));
UINT _ux_host_stack_uninitialize(VOID);
UINT _ux_host_stack_interface_endpoint_get(UX_INTERFACE *interface, UINT endpoint_index, UX_ENDPOINT **endpoint);
UINT _ux_host_stack_interface_instance_create(UX_INTERFACE *interface);
VOID _ux_host_stack_interface_instance_delete(UX_INTERFACE *interface);
UINT _ux_host_stack_interface_set(UX_INTERFACE *interface);
UINT _ux_host_stack_interface_setting_select(UX_INTERFACE *interface);
UINT _ux_host_stack_interfaces_scan(UX_CONFIGURATION *configuration, UCHAR *descriptor);
VOID _ux_host_stack_new_configuration_create(UX_DEVICE *device, UX_CONFIGURATION *configuration);
UX_DEVICE *_ux_host_stack_new_device_get(VOID);
UINT _ux_host_stack_new_device_create(UX_HCD *hcd, UX_DEVICE *device_owner,
                                      UINT port_index, UINT device_speed,
                                      UINT port_max_power,
                                      UX_DEVICE **created_device);
UINT _ux_host_stack_new_endpoint_create(UX_INTERFACE *interface, UCHAR *interface_endpoint);
UINT _ux_host_stack_new_interface_create(UX_CONFIGURATION *configuration, UCHAR *descriptor, ULONG length);
VOID _ux_host_stack_rh_change_process(VOID);
UINT _ux_host_stack_rh_device_extraction(UX_HCD *hcd, UINT port_index);
UINT _ux_host_stack_rh_device_insertion(UX_HCD *hcd, UINT port_index);
UINT _ux_host_stack_transfer_request(UX_TRANSFER *transfer_request);
UINT _ux_host_stack_transfer_request_abort(UX_TRANSFER *transfer_request);
UINT _ux_host_stack_role_swap(UX_DEVICE *device);

#if defined(UX_OTG_SUPPORT)
VOID _ux_host_stack_hnp_polling_thread_entry(ULONG id);
#endif

#endif
