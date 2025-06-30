#include "osi_api.h"
#include "osi_log.h"
#include "hal_config.h"
#include "osi_sysnv.h"
#include "fibo_opencpu.h"
#include "fibocom.h"
#include "drv_hid.h"
#include "usb/usb_ch9.h"
#include "drv_usb.h"

#ifdef CONFIG_FIBOCOM_HID_KEYBOARD_SUPPORT
struct hid_class_descriptor
{
    uint8_t bDescriptorType;
    uint16_t wDescriptorLength;
} __attribute__((packed));

struct hid_descriptor
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdHID;
    uint8_t bCountryCode;
    uint8_t bNumDescriptors;

    struct hid_class_descriptor desc[1];
} __attribute__((packed));

/*-------------------------------------------------------------------------*/
/*                           keyboard Interface descriptors                            */

const unsigned char keyboard_bReport_desc[] =
    {
        0x05,0x01,//UsagePage(USB_HID_GENERIC_DESKTOP),
        0x09,0x06,//Usage(USB_HID_KEYBOARD),
        0xa1,0x01,//Collection(USB_HID_APPLICATION),

        0x05,0x07,//UsagePage(0x07),
        0x19,0xe0, //USAGE_MINIMUM(keyboard leftcontrol)
        0x29,0xe7, //USAGE_MaxIMUM(keyboard right GUI)
        0x15,0x00,//LogicalMinimum(0),
        0x25,0x01,//LogicalMaximum(0x01),
        0x75,0x01,//ReportSize(0x01),
        0x95,0x08,//ReportCount(0x08), 
        0x81,0x02,//Input(USB_HID_INPUT_DATA | USB_HID_INPUT_VARIABLE | USB_HID_INPUT_ABS),

        0x95,0x01,//ReportCount(0x01),
        0x75,0x08,//ReportSize(0x08),
        0x81,0x03,//Input(USB_HID_INPUT_CONSTANT | USB_HID_INPUT_VARIABLE | USB_HID_INPUT_ABS),

        0x95,0x05,//ReportCount(0x05),
        0x75,0x01,//ReportSize(0x01),
        0x05,0x08,//UsagePage(0x08), //leds
        0x19,0x01, //USAGE_MINIMUM(NUM lock)
        0x29,0x05, //USAGE_MAXIMUM (kana)
        0x91,0x02,//Output(USB_HID_OUTPUT_DATA | USB_HID_OUTPUT_VARIABLE | USB_HID_OUTPUT_ABS),
        0x95,0x01,//ReportCount(0x01),
        0x75,0x03,//ReportSize(0x03),
        0x91,0x03,//Output(USB_HID_OUTPUT_CONSTANT | USB_HID_OUTPUT_VARIABLE | USB_HID_OUTPUT_ABS),
        
        0x95,0x06,//ReportCount(0x06),
        0x75,0x08,//ReportSize(0x08),
        0x15,0x00,//LogicalMinimum(0),
        0x25,0xff,//LogicalMaximum(0xff),
        0x05,0x07,//UsagePage(0x07), //keyboard
        0x19,0x00, //USAGE_MINIMUM(reserved)
        0x29,0x65, //USAGE_MAXIMUM (keyboard application)
        0x81,0x00,//Input(USB_HID_INPUT_DATA | USB_HID_INPUT_ARRAY | USB_HID_INPUT_ABS),
        
        0xc0};//EndCollection};


usb_interface_descriptor_t keyboard_interface_desc = {
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = UDESC_INTERFACE,
    /*.bInterfaceNumber	= 0x01,*/
    .bAlternateSetting = 0,
    .bNumEndpoints = 1,
    .bInterfaceClass = UDCLASS_HID,
    .bInterfaceSubClass	= UISUBCLASS_BOOT,
    .bInterfaceProtocol	= UIPROTO_BOOT_KEYBOARD,
    .iInterface		= 0,
};

struct hid_descriptor keyboard_hidg_desc = {
    .bLength = sizeof(keyboard_hidg_desc),
    .bDescriptorType = 0x21,//HID_DT_HID,
    .bcdHID = 0x0111,
    .bCountryCode = 0x21,
    .bNumDescriptors = 0x1,
    .desc[0].bDescriptorType	= 0x22,
    .desc[0].wDescriptorLength	= HID_KEYBOARD_REPORT_DESC_SIZE,
};

usb_endpoint_descriptor_t keyboard_hidg_hs_in_ep_desc = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = UDESC_ENDPOINT,
    .bEndpointAddress = 0x81,//UE_DIR_IN,
    .bmAttributes = UE_INTERRUPT,
    .wMaxPacketSize = 0x0008,
    .bInterval = 4,
};

typedef enum
{
   KEYBOARD_LED_NUMLOCK =1UL << (0), //Num lock
   KEYBOARD_LED_CAPSLOCK =1UL << (1), //Caps lock
   KEYBOARD_LED_SCROLLLOCK =1UL << (2), //Scroll lock
   KEYBOARD_LED_COMPOSE =1UL << (3), //Composition Mode
   KEYBOARD_LED_KANA =1UL << (4), //Kana mode
}hid_keyboard_led_t;

#endif
#ifdef CONFIG_FIBOCOM_CUSTOM_BULK_DEVICE_SUPPORT

typedef struct usb_string
{
    uint8_t id;
    uint8_t ref_count;
    const char *s;
} usbString_t;

/*-------------------------------------------------------------------------*/
/*                      vendo bulk Interface descriptors                            */
usbString_t generic_vendor_str = {
    .s = "vendor interface",
};

const usb_interface_descriptor_t generic_vendor_intf_desc = {
    .bLength = USB_DT_INTERFACE_SIZE,
    .bDescriptorType = UDESC_INTERFACE,
    /* .bInterfaceNumber = DYNAMIC, */
    .bNumEndpoints = 2,
    .bInterfaceClass = UDCLASS_VENDOR,
    .bInterfaceSubClass = 0xff,
    .bInterfaceProtocol = 0,
    .iInterface = 0, // DYNAMIC
};

const usb_endpoint_descriptor_t hs_vendor_data_in = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = UDESC_ENDPOINT,
    .bEndpointAddress = 0x84,//UE_DIR_IN,
    .bmAttributes = UE_BULK,
    .wMaxPacketSize = 512,//cpu_to_le16(512),
    .bInterval = 0,
};

const usb_endpoint_descriptor_t hs_vendor_data_out = {
    .bLength = USB_DT_ENDPOINT_SIZE,
    .bDescriptorType = UDESC_ENDPOINT,
    .bEndpointAddress = 0x5,//UE_DIR_OUT,
    .bmAttributes = UE_BULK,
    .wMaxPacketSize = 512,//cpu_to_le16(512),
    .bInterval = 0,
};
//winUSB
const usb_winusb_osdesc_str_t winusb_os_string ={
  .bLength =0x12,
  .bDescriptorType=0x03,
  .unicode_string ={'M','S','F','T','1','0','0'},
  .vendor=USB_WINUSB_VENDOR_CODE,
  .padding =0x00,
};

const uint8_t bosfeature_desc[USB_LEN_OS_FEATURE_DESC]=
{
0x28,0,0,0, //length
0,1,    //bcd version 1.0
4,0, //windex: externed compat ID descritor
1,    //no of function
0,0,0,0,0,0,0,  //reserver 7 bytes
1, //interface no
0, //reserved
'W','I','N','U','S','B',0,0, //first ID
0,0,0,0,0,0,0,0,  //second ID
0,0,0,0,0,0 //reserved 6 bytes
};

const uint8_t bosproperty_desc[USB_LEN_OS_PROPERTY_DESC]={
0x8E,0,0,0, //length 246 bytes
0x00,0x01,  //BCD VERSION 1.0
0x05,0x00,  //extended property descriptor index
0x01,0x00,  //number of section
//property section
0x84,0x00,0x00,0x00, //size of proterty section
0x1,0,0,0,  //property data type
0x28,0,  //property name length
'D',0,'e',0,'v',0,'i',0,'c',0,'e',0,
'I',0,'n',0,'t',0,'e',0,'r',0,'f',0,'a',0,'c',0,'e',0,
'G',0,'U',0,'I',0,'D',0,0,0,
/*{B024DBBF-BA7A-DF62-57D3-51C1BF9F3A9A}*/
0x4E,0,0,0, //property data length
'{',0,'B',0,'0',0,'2',0,'4',0,'D',0,'B',0,'B',0,'F',0,'-',0,
'B',0,'A',0,'7',0,'A',0,'-',0,
'D',0,'F',0,'6',0,'2',0,'-',0,
'5',0,'7',0,'D',0,'3',0,'-',0,
'5',0,'1',0,'C',0,'1',0,'B',0,'F',0,'9',0,'F',0,'3',0,'A',0,'9',0,'A',0,
'}',0,
0,0
};
#endif


void usb_state_change_notice(uint8_t state, void *cb_ctx)
{
   OSI_LOGI(0,"usb_state_change_notice %d",state);
}

usb_state_change_t usr_usb_state_change={
   .new_sta = 0,
   .callback = NULL,
   .cb_ctx = NULL,
};


