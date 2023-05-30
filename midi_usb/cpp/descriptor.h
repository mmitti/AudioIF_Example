#pragma once
#include<xil_types.h>
#define be2le(val)	(u32)(val)
#define be2les(x)	(u16) (x)
#define htonl(val)	((((u32)(val) & 0x000000FF)<<24) |	\
			 (((u32)(val) & 0x0000FF00)<<8)  |	\
			 (((u32)(val) & 0x00FF0000)>>8)  |	\
			 (((u32)(val) & 0xFF000000)>>24))

#define htons(x)	(u16) ((((u16)(x))<<8) | (((u16)(x))>>8))

#define USB_ENDPOINT0_MAXP		0x40
// エンドポイントの数
#define USB_ENDPOINT_MIDI_NUM			1
// エンドポイント番号
#define USB_CFG_CONTROLE_EP				0
#define USB_CFG_MIDI_STREAM_EP			3
// エンドポイント
#define USB_ENDPOINT_DIR_IN 			0x80
#define USB_ENDPOINT_DIR_OUT 			0x00
#define USB_MIDI_STREAM_BULKIN_EP			(USB_CFG_MIDI_STREAM_EP | USB_ENDPOINT_DIR_IN)
#define USB_MIDI_STREAM_BULKOUT_EP			(USB_CFG_MIDI_STREAM_EP | USB_ENDPOINT_DIR_OUT)

// インターフェースの数
#define USB_INTERFACE_MIDI_NUM 2
// インターフェース番号
#define USB_INTERFACE_AUDIO_CTRL 0x02
#define USB_INTERFACE_MIDISTREAM 0x03

#define USB_DEVICE_DESC			0x01
#define USB_CONFIG_DESC			0x02
#define USB_STRING_DESC			0x03
#define USB_INTERFACE_CFG_DESC		0x04
#define USB_DEVICE_QUALIFIER_DESC	0x06
#define USB_OTHER_SPEED_CFG_DESC	0x07
#define USB_INTERFACE_ASSOCIATION 0x0B
#define USB_ENDPOINT_CFG_DESC		0x05
#define USB_INTERFACE_CS_DESC		0x24
#define USB_ENDPOINT_CS_DESC		0x25
#define USB_ENDPOINT_MAXP 64
#define USB_MIDI_ENDPOINT_MAXP 16

#define MIDI_JACK_EMBEDDED 0x01
#define MIDI_JACK_EXTERNAL 0x02

#define MIDI_IN_JACK 0x02
#define MIDI_OUT_JACK 0x03

#define MIDI_JACK_EMB_MIDI_IN 0x01
#define MIDI_JACK_EXT_MIDI_OUT 0x02
#define MIDI_JACK_EXT_MIDI_IN 0x03
#define MIDI_JACK_EMB_MIDI_OUT 0x04

typedef struct __USB_DESCRIPTOR_DEVICE{
	u8  bLength;
	u8  bDescriptorType;
	u16 bcdUSB;
	u8  bDeviceClass;
	u8  bDeviceSubClass;
	u8  bDeviceProtocol;
	u8  bMaxPacketSize0;
	u16 idVendor;
	u16 idProduct;
	u16 bcdDevice;
	u8  iManufacturer;
	u8  iProduct;
	u8  iSerialNumber;
	u8  bNumConfigurations;
}__attribute__((__packed__)) USB_DESCRIPTOR_DEVICE;

typedef struct __USB_DESCRIPTOR_DEVICE_QUALIFIER{
	u8  bLength;
	u8  bDescriptorType;
	u16 bcdUSB;
	u8  bDeviceClass;
	u8  bDeviceSubClass;
	u8  bDeviceProtocol;
	u8  bMaxPacketSize0;
	u8  bNumConfigurations;
	u8  bReserved;
}__attribute__((__packed__)) USB_DESCRIPTOR_DEVICE_QUALIFIER;

typedef struct __USB_DESCRIPTOR_CONFIG{
	u8  bLength;
	u8  bDescriptorType;
	u16 wTotalLength;
	u8  bNumInterfaces;
	u8  bConfigurationValue;
	u8  iConfiguration;
	u8  bmAttributes;
	u8  bMaxPower;
}__attribute__((__packed__)) USB_DESCRIPTOR_CONFIG;

typedef struct __USB_DESCRIPTOR_IAD{
	u8  bLength;
	u8  bDescriptorType;
	u8  bFirstInterface;
	u8  bInterfaceCount;
	u8  bFuncionClass;
	u8  bFuncionSubClass;
	u8  bFuncionProtocol;
	u8  iFunction;
}__attribute__((__packed__)) USB_DESCRIPTOR_IAD;

typedef struct __USB_DESCRIPTOR_INTERFACE{
	u8  bLength;
	u8  bDescriptorType;
	u8  bInterfaceNumber;
	u8  bAlternateSetting;
	u8  bNumEndPoints;
	u8  bInterfaceClass;
	u8  bInterfaceSubClass;
	u8  bInterfaceProtocol;
	u8  iInterface;
}__attribute__((__packed__)) USB_DESCRIPTOR_INTERFACE;

typedef struct __USB_DESCRIPTOR_ENDPOINT{
	u8  bLength;
	u8  bDescriptorType;
	u8  bEndpointAddress;
	u8  bmAttributes;
	u16 wMaxPacketSize;
	u8  bInterval;
}__attribute__((__packed__)) USB_DESCRIPTOR_ENDPOINT;

typedef struct __USB_DESCRIPTOR_STRING{
	u8  bLength;
	u8  bDescriptorType;
	u16 wLANGID[1];
}__attribute__((__packed__)) USB_DESCRIPTOR_STRING;

typedef struct __USB_DESCRIPTOR_AUDIO_CS{
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u16 bcdADC;
	u16 wTotalLength;
	u8 bInCollection;
	u8 baInferface;
}__attribute__((__packed__)) USB_DESCRIPTOR_AUDIO_CS;

typedef struct __USB_DESCRIPTOR_MIDI_HEAD_CS{
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u16 bcdADC;
	u16 wTotalLength;
}__attribute__((__packed__)) USB_DESCRIPTOR_MIDI_HEAD_CS;


typedef struct __USB_DESCRIPTOR_MIDI_IN_JACK_CS{
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bJackType;
	u8 	bJackID;
	u8 iJack;
}__attribute__((__packed__)) USB_DESCRIPTOR_MIDI_IN_JACK_CS;

typedef struct __USB_DESCRIPTOR_MIDI_OUT_JACK_CS{
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bJackType;
	u8 	bJackID;
	u8 	bNrInputPins;
	u8 baSourceID0;
	u8 baSourcePin0;
	u8 iJack;
}__attribute__((__packed__)) USB_DESCRIPTOR_MIDI_OUT_JACK_CS;

typedef struct __USB_DESCRIPTOR_MIDI_IN_EP_CS{
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bNumEmbMIDIJack;
	u8 	baAssocJackID0;
}__attribute__((__packed__)) USB_DESCRIPTOR_MIDI_IN_EP_CS;

typedef struct __USB_DESCRIPTOR_MIDI_OUT_EP_CS{
	u8 bLength;
	u8 bDescriptorType;
	u8 bDescriptorSubType;
	u8 bNumEmbMIDIJack;
	u8 	baAssocJackID0;
}__attribute__((__packed__)) USB_DESCRIPTOR_MIDI_OUT_EP_CS;

typedef struct __USB_DESCRIPTOR_MIDI_CS{
	USB_DESCRIPTOR_MIDI_HEAD_CS head;
	USB_DESCRIPTOR_MIDI_IN_JACK_CS out0_midi_emb_in;
	USB_DESCRIPTOR_MIDI_OUT_JACK_CS out0_midi_ext_out;
	USB_DESCRIPTOR_MIDI_IN_JACK_CS in0_midi_ext_in;
	USB_DESCRIPTOR_MIDI_OUT_JACK_CS in0_midi_emb_out;
}__attribute__((__packed__)) USB_DESCRIPTOR_MIDI_CS;

typedef struct __USB_MIDI_INTERFACE{
	USB_DESCRIPTOR_INTERFACE midi_iface;
	USB_DESCRIPTOR_MIDI_CS midi_cs;
	USB_DESCRIPTOR_ENDPOINT ep_out;
	USB_DESCRIPTOR_MIDI_OUT_EP_CS ep_out_cs;
	USB_DESCRIPTOR_ENDPOINT ep_in;
	USB_DESCRIPTOR_MIDI_IN_EP_CS ep_in_cs;
}__attribute__((__packed__)) USB_MIDI_INTERFACE;

typedef struct __USB_CONFIG_NOP{
	USB_DESCRIPTOR_CONFIG stdCfg;
	USB_DESCRIPTOR_INTERFACE vendor_if;
}__attribute__((__packed__)) USB_CONFIG_NOP;

typedef struct __USB_CONFIG{
	USB_DESCRIPTOR_CONFIG stdCfg;
	USB_DESCRIPTOR_IAD iad;
	USB_DESCRIPTOR_INTERFACE audio_ctrl_iface;
	USB_DESCRIPTOR_AUDIO_CS audio_ctrl_cs;
	USB_MIDI_INTERFACE midi;
}__attribute__((__packed__)) USB_CONFIG;


u32 GetDeviceDescriptor(u8 *BufPtr, u32 BufLen, bool high_speed);
u32 GetDeviceQualifierDescriptor(u8 *BufPtr, u32 BufLen, bool high_speed);
u32 GetConfigDescriptor(u8 *BufPtr, u32 BufLen, bool high_speed);
u32 GetOtherSpeedConfigDescriptor(u8 *BufPtr, u32 BufLen, bool high_speed);
u32 GetStringDescriptor(u8 *BufPtr, u32 BufLen, u8 Index);
