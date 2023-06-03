#pragma once
#include <usbps/UsbPs.h>
#include <xil_cache.h>
#include <memory>
#include <list>
#include <MIDIPeripheral.h>
#include "common.h"
#include "descriptor.h"
#include "ring_buffer.h"
#define MEMORY_SIZE (128 * 1024)
#define MIDI_MESSAGE_BUFFER_SIZE 12 //4096byte = 512 messages
// Request Values
#define XUSBPS_REQ_GET_STATUS		0x00
#define XUSBPS_REQ_CLEAR_FEATURE	0x01
#define XUSBPS_REQ_SET_FEATURE		0x03
#define XUSBPS_REQ_SET_ADDRESS		0x05
#define XUSBPS_REQ_GET_DESCRIPTOR	0x06
#define XUSBPS_REQ_SET_DESCRIPTOR	0x07
#define XUSBPS_REQ_GET_CONFIGURATION	0x08
#define XUSBPS_REQ_SET_CONFIGURATION	0x09
#define XUSBPS_REQ_GET_INTERFACE	0x0a
#define XUSBPS_REQ_SET_INTERFACE	0x0b
#define XUSBPS_REQ_SYNC_FRAME		0x0c

// Feature Selectors
#define XUSBPS_ENDPOINT_HALT		0x00
#define XUSBPS_DEVICE_REMOTE_WAKEUP	0x01
#define XUSBPS_TEST_MODE		0x02

//USB Device States
#define XUSBPS_DEVICE_ATTACHED		0x00
#define XUSBPS_DEVICE_POWERED		0x01
#define XUSBPS_DEVICE_DEFAULT		0x02
#define XUSBPS_DEVICE_ADDRESSED	0x03
#define XUSBPS_DEVICE_CONFIGURED	0x04
#define XUSBPS_DEVICE_SUSPENDED	0x05


// Descriptor Types
#define XUSBPS_TYPE_DEVICE_DESC		0x01
#define XUSBPS_TYPE_CONFIG_DESC		0x02
#define XUSBPS_TYPE_STRING_DESC		0x03
#define XUSBPS_TYPE_IF_CFG_DESC		0x04
#define XUSBPS_TYPE_ENDPOINT_CFG_DESC	0x05
#define XUSBPS_TYPE_DEVICE_QUALIFIER	0x06
#define XUSBPS_TYPE_OTHER_SPEED_CFG_DESC	0x07
#define XUSBPS_TYPE_REPORT_DESC		0x22

#define XUSBPS_REQ_REPLY_LEN 1024 
#define DPRINT(...) 

class USBMidi{
protected:
    XScuGic& IntcInstance; /* The instance of the IRQ Controller */
    MIDIPeripheral &midi_ph;
    u8 Buffer[MEMORY_SIZE] ALIGNMENT_CACHELINE;
    RingBuffer<MIDI_MESSAGE_BUFFER_SIZE> outbuffer ALIGNMENT_CACHELINE;
    RingBuffer<MIDI_MESSAGE_BUFFER_SIZE> inbuffer ALIGNMENT_CACHELINE;

    usbps::UsbPs UsbInstance;   /* The instance of the USB Controller */

    void stdDeviceRequest(usbps::SetupData& data);
    void classRequest(usbps::SetupData& data, u8* in_data);
public:

    USBMidi(XScuGic& intc, MIDIPeripheral &midi_ph, u16 deviceId, u32 baseAddr);
    virtual ~USBMidi(){};
    // generic usb proc
    virtual bool init();
    virtual void release();
    void ep0Handler(u8 endpoint_num, usbps::EndpointEvent event_type, void *data);
    inline void stall(u8 ep, usbps::EndpointDirection dir = usbps::EndpointDirection::IN_OUT){
        UsbInstance.stallEndpoint(ep, dir);
    }
    inline int ack(u8 ep){
        return sendBuffer(ep, NULL, 0);
    }
    void intrHandler(u32 mask);
    inline usbps::EndpointBuffer receiveBuffer(u8 endpoint, bool pooling = false) {
        return UsbInstance.receiveBuffer(endpoint, pooling);
    }
    inline void releaseBuffer(usbps::EndpointBuffer& buffer) {
        UsbInstance.releaseBuffer(buffer);
    }
    inline int sendBuffer(u8 endpoint, const u8* buffer, u32 length){
        return UsbInstance.sendBuffer(endpoint, buffer, length);
    }
    // usb midi proc
    void epHandler(u8 endpoint_num, usbps::EndpointEvent event_type, void *data);

    virtual void update();

    
};
