#include "descriptor.h"
#include <string.h>

/// descriptor constants
#define USB_DC_BCDUSB 0x200
#define USB_DC_IAD_CLASS 0xEF
#define USB_DC_IAD_SUB_CLASS 0x02
#define USB_DC_IAD_PROTOCOL 0x01
#define USB_DC_VENDOR_CLASS 0xFF
#define USB_DC_VENDOR_SUB_CLASS 0x00
#define USB_DC_VENDOR_PROTOCOL 0x00
#define USB_DC_ID_VENDOR 0x6666 // PROTOTIPE VENDOR ID DO NOT USE FOR PRODUCTION
#define USB_DC_ID_PRODUCT 0x0001
#define USB_DC_BCD_DEVICE 0x0100

#define USB_DC_AUDIO_CTRL_IF_CLASS 0x01
#define USB_DC_AUDIO_CTRL_SUB_CLASS 0x01
#define USB_DC_AUDIO_CTRL_IF_PROTOCOL 0x00

#define USB_DC_MIDI_STREAM_IF_CLASS 0x01
#define USB_DC_MIDI_STREAM_IF_SUB_CLASS 0x03
#define USB_DC_MIDI_STREAM_IF_PROTOCOL 0x00

#define USB_STR_MANUFACTURER 0x01
#define USB_STR_PRODUCT 0x02
#define USB_STR_SERIAL_NUMBER 0x03
#define USB_STR_DEV_NAME_MIDI 0x05
#define USB_STR_MIDI_IN0 0x07
#define USB_STR_MIDI_OUT0 0x08
static const char *StringList[] = {"",
    "mmitti",
    "mmitti Composit Device",
    "00000001",
    "",
    "mmitti MIDI",
    "",
    "midi in",
    "midi out"
};

template <class T> u32 GetDescriptor(u8 *BufPtr, u32 BufLen, T& desc){
	/* Check buffer pointer is there and buffer is big enough. */
	if (!BufPtr) return 0;
	if (BufLen < sizeof(T)) return 0;

	memcpy(BufPtr, &desc, sizeof(T));

	return sizeof(T);
}

u32 GetDeviceDescriptor(u8 *BufPtr, u32 BufLen, bool high_speed)
{
	USB_DESCRIPTOR_DEVICE deviceDescHS = {
		sizeof(USB_DESCRIPTOR_DEVICE),	/* bLength */
		USB_DEVICE_DESC,		/* bDescriptorType */
		be2les(USB_DC_BCDUSB),			/* bcdUSB 2.0 */
		USB_DC_IAD_CLASS,				/* bDeviceClass */
		USB_DC_IAD_SUB_CLASS,				/* bDeviceSubClass */
		USB_DC_IAD_PROTOCOL,				/* bDeviceProtocol */
		USB_ENDPOINT0_MAXP,		/* bMaxPackedSize0 */
		be2les(USB_DC_ID_VENDOR),			/* idVendor */
		be2les(USB_DC_ID_PRODUCT),			/* idProduct */
		be2les(USB_DC_BCD_DEVICE),			/* bcdDevice */
		USB_STR_MANUFACTURER,				/* iManufacturer */
		USB_STR_PRODUCT,				/* iProduct */
		USB_STR_SERIAL_NUMBER,				/* iSerialNumber */
		0x01				/* bNumConfigurations */
	};
    USB_DESCRIPTOR_DEVICE deviceDescFS = {
		sizeof(USB_DESCRIPTOR_DEVICE),	/* bLength */
		USB_DEVICE_DESC,		/* bDescriptorType */
		be2les(USB_DC_BCDUSB),			/* bcdUSB 2.0 */
		USB_DC_VENDOR_CLASS,				/* bDeviceClass */
		USB_DC_VENDOR_SUB_CLASS,				/* bDeviceSubClass */
		USB_DC_VENDOR_PROTOCOL,				/* bDeviceProtocol */
		USB_ENDPOINT0_MAXP,		/* bMaxPackedSize0 */
		be2les(USB_DC_ID_VENDOR),			/* idVendor */
		be2les(USB_DC_ID_PRODUCT),			/* idProduct */
		be2les(USB_DC_BCD_DEVICE),			/* bcdDevice */
		USB_STR_MANUFACTURER,				/* iManufacturer */
		USB_STR_PRODUCT,				/* iProduct */
		USB_STR_SERIAL_NUMBER,				/* iSerialNumber */
		0x01				/* bNumConfigurations */
	};
    
    if (high_speed) {
        return GetDescriptor<USB_DESCRIPTOR_DEVICE>(BufPtr, BufLen, deviceDescHS);
    }
    return GetDescriptor<USB_DESCRIPTOR_DEVICE>(BufPtr, BufLen, deviceDescFS);
}

u32 GetDeviceQualifierDescriptor(u8 *BufPtr, u32 BufLen, bool high_speed)
{
	USB_DESCRIPTOR_DEVICE_QUALIFIER deviceDescHS = {
		sizeof(USB_DESCRIPTOR_DEVICE_QUALIFIER),	/* bLength */
		USB_DEVICE_QUALIFIER_DESC,		/* bDescriptorType */
		be2les(USB_DC_BCDUSB),			/* bcdUSB 2.0 */
		USB_DC_VENDOR_CLASS,				/* bDeviceClass */
		USB_DC_VENDOR_SUB_CLASS,				/* bDeviceSubClass */
		USB_DC_VENDOR_PROTOCOL,				/* bDeviceProtocol */
		USB_ENDPOINT0_MAXP,		/* bMaxPackedSize0 */
        0x01,				/* bNumConfigurations */
        0x00
	};
    USB_DESCRIPTOR_DEVICE_QUALIFIER deviceDescFS = {
		sizeof(USB_DESCRIPTOR_DEVICE_QUALIFIER),	/* bLength */
		USB_DEVICE_QUALIFIER_DESC,		/* bDescriptorType */
		be2les(USB_DC_BCDUSB),			/* bcdUSB 2.0 */
		USB_DC_IAD_CLASS,				/* bDeviceClass */
		USB_DC_IAD_SUB_CLASS,				/* bDeviceSubClass */
		USB_DC_IAD_PROTOCOL,				/* bDeviceProtocol */
		USB_ENDPOINT0_MAXP,		/* bMaxPackedSize0 */
        0x01,				/* bNumConfigurations */
        0x00
	};
    if (high_speed) {
        return GetDescriptor<USB_DESCRIPTOR_DEVICE_QUALIFIER>(BufPtr, BufLen, deviceDescHS);
    }
    return GetDescriptor<USB_DESCRIPTOR_DEVICE_QUALIFIER>(BufPtr, BufLen, deviceDescFS);
}

u32 GetConfigDescriptor(u8 *BufPtr, u32 BufLen, bool high_speed)
{
    if (!high_speed) {
        USB_DESCRIPTOR_CONFIG conf = {
            sizeof(USB_DESCRIPTOR_CONFIG),	/* bLength */
            USB_CONFIG_DESC,		/* bDescriptorType */
            be2les(sizeof(USB_CONFIG_NOP)),	/* wTotalLength */
            0,				/* bNumInterfaces */
            0x00,				/* bConfigurationValue */
            0x00,				/* iConfiguration-->String ID*/
            0xc0,				/* bmAttribute Power Bus+Self */
            0x00                /* bMaxPower */
        };
        USB_DESCRIPTOR_INTERFACE vendor_iface = {
            sizeof(USB_DESCRIPTOR_INTERFACE),	/* bLength */
            USB_INTERFACE_CFG_DESC,	/* bDescriptorType */
            0x00,				/* bInterfaceNumber */
            0x00,				/* bAlternateSetting */
            0x00,				/* bNumEndPoints */
            USB_DC_VENDOR_CLASS,				/* bInterfaceClass */ 
            USB_DC_VENDOR_SUB_CLASS,				/* bInterfaceSubClass *///ACM
            USB_DC_VENDOR_PROTOCOL,				/* bInterfaceProtocol */
            0x00 /*iInterface -> String ID*/
        };
        USB_CONFIG_NOP ret = {
            conf,
            vendor_iface
        };
        return GetDescriptor<USB_CONFIG_NOP>(BufPtr, BufLen, ret);
    }
    
    
	USB_DESCRIPTOR_CONFIG conf = {
		sizeof(USB_DESCRIPTOR_CONFIG),	/* bLength */
        USB_CONFIG_DESC,		/* bDescriptorType */
        be2les(sizeof(USB_CONFIG)),	/* wTotalLength */
        USB_INTERFACE_MIDI_NUM,				/* bNumInterfaces */
        0x01,				/* bConfigurationValue */
        USB_STR_DEV_NAME_MIDI,				/* iConfiguration-->String ID*/
        0xc0,				/* bmAttribute Power Bus+Self */
        0x00                /* bMaxPower */
    };
	USB_DESCRIPTOR_IAD iad={
		sizeof(USB_DESCRIPTOR_IAD),
		USB_INTERFACE_ASSOCIATION,
		USB_INTERFACE_AUDIO_CTRL, //start if num
		USB_INTERFACE_MIDI_NUM, // interface num
		0x00,
		0x00,
		0x00,
		USB_STR_DEV_NAME_MIDI
	};
    
    // MIDI

    USB_DESCRIPTOR_INTERFACE audio_ctrl_iface = {
        sizeof(USB_DESCRIPTOR_INTERFACE),	/* bLength */
        USB_INTERFACE_CFG_DESC,	/* bDescriptorType */
        USB_INTERFACE_AUDIO_CTRL,				/* bInterfaceNumber */
        0x00,				/* bAlternateSetting */
        0x00,				/* bNumEndPoints */ //割り込みをサポートする場合ので1
        USB_DC_AUDIO_CTRL_IF_CLASS,				/* bInterfaceClass */ 
        USB_DC_AUDIO_CTRL_SUB_CLASS,				/* bInterfaceSubClass *///ACM
        USB_DC_AUDIO_CTRL_IF_PROTOCOL,				/* bInterfaceProtocol */
        USB_STR_DEV_NAME_MIDI /*iInterface -> String ID*/
    };
    
    USB_DESCRIPTOR_AUDIO_CS audio_cs = {
        sizeof(USB_DESCRIPTOR_AUDIO_CS),/* u8 bLength;*/
        USB_INTERFACE_CS_DESC,/* u8 bDescriptorType;*/
        0x01,/* u8 bDescriptorSubType; HEADER*/
        be2les(0x0100),/* u16 bcdADC;*/
        be2les(sizeof(USB_DESCRIPTOR_AUDIO_CS)),/* u16 wTotalLength;*/
        1,/* u8 bInCollection;*/
        USB_INTERFACE_MIDISTREAM,/* u8 baInferface;*/
    };

    USB_DESCRIPTOR_INTERFACE midi_iface = {
        sizeof(USB_DESCRIPTOR_INTERFACE),	/* bLength */
        USB_INTERFACE_CFG_DESC,	/* bDescriptorType */
        USB_INTERFACE_MIDISTREAM,				/* bInterfaceNumber */
        0x00,				/* bAlternateSetting */
        0x02,				/* bNumEndPoints */
        USB_DC_MIDI_STREAM_IF_CLASS,				/* bInterfaceClass */ 
        USB_DC_MIDI_STREAM_IF_SUB_CLASS,				/* bInterfaceSubClass *///ACM
        USB_DC_MIDI_STREAM_IF_PROTOCOL,				/* bInterfaceProtocol */
        USB_STR_DEV_NAME_MIDI/*iInterface -> String ID*/
    };
    
    USB_DESCRIPTOR_MIDI_HEAD_CS midi_cs_head = {
        sizeof(USB_DESCRIPTOR_MIDI_HEAD_CS),
        USB_INTERFACE_CS_DESC,
        0x01, //descriptor subtype HEADER
        be2les(0x0100),//MIDI Stream release ver 0x100=1.0
        be2les(sizeof(USB_DESCRIPTOR_MIDI_CS))
    };
    
    // MIDI OUT(ENBIN->EXT OUT)
    USB_DESCRIPTOR_MIDI_IN_JACK_CS out0_emb_in_jack = {
        sizeof(USB_DESCRIPTOR_MIDI_IN_JACK_CS),
        USB_INTERFACE_CS_DESC,
        0x02,//descriptor subtype MIDI_IN_JACK
        MIDI_JACK_EMBEDDED,
        MIDI_JACK_EMB_MIDI_IN,//jack id
        USB_STR_MIDI_OUT0// string
    };
    
    USB_DESCRIPTOR_MIDI_OUT_JACK_CS out0_ext_out_jack={
        sizeof(USB_DESCRIPTOR_MIDI_OUT_JACK_CS),
        USB_INTERFACE_CS_DESC,
        MIDI_OUT_JACK,//descriptor subtype
        MIDI_JACK_EXTERNAL,
        MIDI_JACK_EXT_MIDI_OUT,//jack id
        0x01,//input pin num
        MIDI_JACK_EMB_MIDI_IN,//src jack id
        0x01,//src jack pin
        USB_STR_MIDI_OUT0//string
    };
    
    // MIDI IN(EXT IN->EMB OUT)
    USB_DESCRIPTOR_MIDI_IN_JACK_CS in0_ext_in_jack = {
        sizeof(USB_DESCRIPTOR_MIDI_IN_JACK_CS),
        USB_INTERFACE_CS_DESC,
        0x02,//descriptor subtype MIDI_IN_JACK
        MIDI_JACK_EXTERNAL,
        MIDI_JACK_EXT_MIDI_IN,//jack id
        USB_STR_MIDI_IN0// string
    };
    
    USB_DESCRIPTOR_MIDI_OUT_JACK_CS in0_emb_out_jack={
        sizeof(USB_DESCRIPTOR_MIDI_OUT_JACK_CS),
        USB_INTERFACE_CS_DESC,
        MIDI_OUT_JACK,//descriptor subtype
        MIDI_JACK_EMBEDDED,
        MIDI_JACK_EMB_MIDI_OUT,//jack id
        0x01,//input pin num
        MIDI_JACK_EXT_MIDI_IN,//src jack id
        0x01,//src jack pin
        USB_STR_MIDI_IN0//string
    };
    
    USB_DESCRIPTOR_ENDPOINT midi_ep0={
        sizeof(USB_DESCRIPTOR_ENDPOINT),	/* bLength */
        USB_ENDPOINT_CFG_DESC,		/* bDescriptorType */
        USB_MIDI_STREAM_BULKOUT_EP,		/* bEndpointAddress */
        0x02,				/* bmAttribute  */
        be2les(USB_MIDI_ENDPOINT_MAXP),			/* wMaxPacketSize */
        0 /*ms*/
    };
    
    USB_DESCRIPTOR_MIDI_OUT_EP_CS midi_ep0_cs = {
        sizeof(USB_DESCRIPTOR_MIDI_OUT_EP_CS),
        USB_ENDPOINT_CS_DESC,
        0x01,//MS_GENERAL
        0x01,//num jack
        MIDI_JACK_EMB_MIDI_IN//emb midi in
    };
    
    USB_DESCRIPTOR_ENDPOINT midi_ep1={
        sizeof(USB_DESCRIPTOR_ENDPOINT),	/* bLength */
        USB_ENDPOINT_CFG_DESC,		/* bDescriptorType */
        USB_MIDI_STREAM_BULKIN_EP,		/* bEndpointAddress */
        0x02,				/* bmAttribute  */
        be2les(USB_MIDI_ENDPOINT_MAXP),			/* wMaxPacketSize */
        0 /*ms*/
    };
    
    USB_DESCRIPTOR_MIDI_IN_EP_CS midi_ep1_cs = {
        sizeof(USB_DESCRIPTOR_MIDI_IN_EP_CS),
        USB_ENDPOINT_CS_DESC,
        0x01,//MS_GENERAL
        0x01,//num jack
        MIDI_JACK_EMB_MIDI_OUT//emb midi out
    };

    USB_CONFIG ret = {
        conf, 
		iad,
        audio_ctrl_iface,
        audio_cs,
        USB_MIDI_INTERFACE{
            midi_iface,
			USB_DESCRIPTOR_MIDI_CS{
				midi_cs_head,
				out0_emb_in_jack,
				out0_ext_out_jack,
				in0_ext_in_jack,
				in0_emb_out_jack
            },
            midi_ep0,
            midi_ep0_cs,
            midi_ep1,
            midi_ep1_cs
        }
    };
    
    return GetDescriptor<USB_CONFIG>(BufPtr, BufLen, ret);
}

u32 GetOtherSpeedConfigDescriptor(u8 *BufPtr, u32 BufLen, bool high_speed) {
    auto ret = GetConfigDescriptor(BufPtr, BufLen, !high_speed);
    BufPtr[1] = USB_OTHER_SPEED_CFG_DESC;
    return ret;
}


u32 GetStringDescriptor(u8 *BufPtr, u32 BufLen, u8 Index)
{
	unsigned int i;

	const char *String;
	u32 StringLen;
	u32 DescLen;
	u8 TmpBuf[128];

	USB_DESCRIPTOR_STRING *StringDesc;

	if (!BufPtr) {
		return 0;
	}

	if (Index >= sizeof(StringList) / sizeof(char *)) {
		return 0;
	}

	String = StringList[Index];
	StringLen = strlen(String);

	StringDesc = (USB_DESCRIPTOR_STRING *) TmpBuf;

	if (0 == Index) {
		StringDesc->bLength = 4;
		StringDesc->bDescriptorType = USB_STRING_DESC;
		StringDesc->wLANGID[0] = be2les(0x0409);
	}
	else {
		StringDesc->bLength = StringLen * 2 + 2;
		StringDesc->bDescriptorType = USB_STRING_DESC;

		for (i = 0; i < StringLen; i++) {
			StringDesc->wLANGID[i] = be2les((u16) String[i]);
		}
	}
	DescLen = StringDesc->bLength;

	if (DescLen > BufLen) {
		return 0;
	}

	memcpy(BufPtr, StringDesc, DescLen);

	return DescLen;
}
