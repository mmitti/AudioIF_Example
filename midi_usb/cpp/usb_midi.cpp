#include "usb_midi.h"
#include <stdlib.h>
#include <stdio.h>
#define FAIL() { \
    printf("FAIL!!"); \
    while(true);\
}
namespace
{
    static void UsbIntrHandler(void *CallBackRef, u32 Mask)
    {
        USBMidi *inst = reinterpret_cast<USBMidi *>(CallBackRef);
        inst->intrHandler(Mask);
    }
    static void Ep0EventHandler(void *CallBackRef, u8 EpNum,
                                usbps::EndpointEvent EventType, void *Data)
    {
        USBMidi *inst = reinterpret_cast<USBMidi *>(CallBackRef);
        inst->ep0Handler(EpNum, EventType, Data);
    }
    static void EpEventHandler(void *CallBackRef, u8 EpNum,
                               usbps::EndpointEvent EventType, void *Data)
    {
		USBMidi *inst = reinterpret_cast<USBMidi *>(CallBackRef);
		inst->epHandler(EpNum, EventType, Data);
    }
} // namespace

USBMidi::USBMidi(XScuGic &intc, MIDIPeripheral &midi_ph, u32 baseAddr) : IntcInstance(intc), midi_ph(midi_ph), UsbInstance(baseAddr){};

bool USBMidi::init()
{
    constexpr u16 UsbIntrId = XPAR_XUSBPS_0_INTR;
    int Status;
    // init usb
    Status = UsbInstance.init(&IntcInstance, UsbIntrId);
    if (XST_SUCCESS != Status)
        return false;

    // initialize endpoint
    UsbInstance.DeviceConfig.EpCfg[USB_CFG_CONTROLE_EP].Out.Type = usbps::EndpointType::CONTROL;
    UsbInstance.DeviceConfig.EpCfg[USB_CFG_CONTROLE_EP].Out.NumBufs = 2;
    UsbInstance.DeviceConfig.EpCfg[USB_CFG_CONTROLE_EP].Out.BufSize = USB_ENDPOINT0_MAXP;
    UsbInstance.DeviceConfig.EpCfg[USB_CFG_CONTROLE_EP].Out.MaxPacketSize = USB_ENDPOINT0_MAXP;
    UsbInstance.DeviceConfig.EpCfg[USB_CFG_CONTROLE_EP].In.Type = usbps::EndpointType::CONTROL;
    UsbInstance.DeviceConfig.EpCfg[USB_CFG_CONTROLE_EP].In.NumBufs = 2;
    UsbInstance.DeviceConfig.EpCfg[USB_CFG_CONTROLE_EP].In.MaxPacketSize = USB_ENDPOINT0_MAXP;
    
	UsbInstance.DeviceConfig.EpCfg[USB_CFG_MIDI_STREAM_EP].Out.Type = usbps::EndpointType::BULK;
	UsbInstance.DeviceConfig.EpCfg[USB_CFG_MIDI_STREAM_EP].Out.NumBufs = 256;
	UsbInstance.DeviceConfig.EpCfg[USB_CFG_MIDI_STREAM_EP].Out.BufSize = USB_MIDI_ENDPOINT_MAXP;
	UsbInstance.DeviceConfig.EpCfg[USB_CFG_MIDI_STREAM_EP].Out.MaxPacketSize = USB_MIDI_ENDPOINT_MAXP;
	UsbInstance.DeviceConfig.EpCfg[USB_CFG_MIDI_STREAM_EP].In.Type = usbps::EndpointType::BULK;
	UsbInstance.DeviceConfig.EpCfg[USB_CFG_MIDI_STREAM_EP].In.NumBufs = 128;
	UsbInstance.DeviceConfig.EpCfg[USB_CFG_MIDI_STREAM_EP].In.MaxPacketSize = USB_MIDI_ENDPOINT_MAXP;

    UsbInstance.DeviceConfig.NumEndpoints = 4; // sets the max index of endpoint

    Status = UsbInstance.setupDevice((u8 *)&Buffer[0], MEMORY_SIZE);
    if (XST_SUCCESS != Status)
        return false;

    // register interrups
    UsbInstance.setInterrupt(UsbIntrHandler, this, XUSBPS_IXR_SR_MASK);
    UsbInstance.setEndpointHandler(0, usbps::EndpointDirection::OUT, Ep0EventHandler, this);
    UsbInstance.setEndpointHandler(3, usbps::EndpointDirection::IN_OUT, EpEventHandler, this);
    // enable interrups
    UsbInstance.enableInterrupt(XUSBPS_IXR_SR_MASK);
    // Start the USB engine
    UsbInstance.start();
    DPRINT("start");
    return true;
}

void USBMidi::release()
{
    UsbInstance.stop();
    UsbInstance.disableInterrupt(XUSBPS_IXR_ALL);
    UsbInstance.setInterrupt(NULL, NULL, 0);
    XScuGic_Disconnect(&IntcInstance, XPAR_XUSBPS_0_INTR);
}

void USBMidi::intrHandler(u32 mask)
{
    
}

void USBMidi::ep0Handler(u8 endpoint_num, usbps::EndpointEvent event_type, void *data)
{
    int Status;
    usbps::SetupData SetupData;
    switch (event_type)
    {

    case usbps::EndpointEvent::SETUP_DATA_RECEIVED:
        Status = UsbInstance.getSetupData(endpoint_num, SetupData);
        if (XST_SUCCESS != Status)
        {
            DPRINT("GET FAILED\n");
            break;
        }

        // handle setup packet
        switch (SetupData.getType())
        {
        case usbps::SetupType::STANDARD:
            stdDeviceRequest(SetupData);
            break;
        case usbps::SetupType::CLASS:
        {
            if ((SetupData.bmRequestType & 0x80) == 0x00 && SetupData.wLength > 0)
            {
                constexpr u32 EpNum = 0;
                if(!UsbInstance.primeEndpoint(EpNum, usbps::EndpointDirection::OUT, true))
                    return;
                auto buffer = receiveBuffer(EpNum, true);
                if (buffer.status == XST_SUCCESS)
                {
                    classRequest(SetupData, buffer.buffer);
                }
            }
            else
            {
                classRequest(SetupData, nullptr);
            }
        }
        break;
        default:
            DPRINT("unknown class req, stall 0 in out\n");
            stall(0);
            break;
        }

        break;

    // ignore data to EP0
    case usbps::EndpointEvent::DATA_RX:
    {
        auto buffer = receiveBuffer(endpoint_num);
        if (buffer.length != 0)
            DPRINT("EP 0 DATA RECEIVED : (%d) %x %x %x\n", buffer.length, buffer.buffer[0], buffer.buffer[1], buffer.buffer[2]);
        releaseBuffer(buffer);
        break;
    }
    default:
        break;
    }
}

void USBMidi::classRequest(usbps::SetupData &data, u8 *in_data)
{

    if (data.wLength > XUSBPS_REQ_REPLY_LEN)
    {
        DPRINT("E: requested reply length of %d is bigger than reply buffer (%d)\n", data.wLength,
               XUSBPS_REQ_REPLY_LEN);
        return;
    }
    if (in_data == nullptr)
        DPRINT("if req IN T:%d R:%d I:%d V:%d \n", data.bmRequestType, data.bRequest, data.wIndex, data.wValue);
    else
        DPRINT("if req OUT T:%d R:%d I:%d V:%d \n", data.bmRequestType, data.bRequest, data.wIndex, data.wValue);
    
    // DO Nothing

    ack(0);
}

void USBMidi::stdDeviceRequest(usbps::SetupData &data)
{
    int Status;
    bool Error = false;

    u8 Response ALIGNMENT_CACHELINE;
    int ReplyLen;
    static u8 Reply[XUSBPS_REQ_REPLY_LEN] ALIGNMENT_CACHELINE;

    if (data.wLength > XUSBPS_REQ_REPLY_LEN)
        return;
    DPRINT("std dev req R:%d I:%d V:%d \n", data.bRequest, data.wIndex, data.wValue);

    // handle device request
    switch (data.bRequest)
    {
    case XUSBPS_REQ_GET_STATUS:
    {
        switch (data.getTarget())
        {
        case usbps::SetupTarget::DEVICE:
            Reply[0] = 0x01;
            Reply[1] = 0x00; /* Self powered */
            break;
        case usbps::SetupTarget::INTERFACE:
            Reply[0] = 0;
            Reply[1] = 0;
            break;
        case usbps::SetupTarget::ENDPOINT:
        {
            int EpNum = data.wIndex;
            if (EpNum & 0x80)
            { /* In EP */
                if (UsbInstance.isStallEndpoint(EpNum, usbps::EndpointDirection::IN))
                    *((u16 *)&Reply[0]) = 0x0100;
                else
                    *((u16 *)&Reply[0]) = 0x0000;
            }
            else
            { /* Out EP */
                if (UsbInstance.isStallEndpoint(EpNum, usbps::EndpointDirection::OUT))
                    *((u16 *)&Reply[0]) = 0x0100;
                else
                    *((u16 *)&Reply[0]) = 0x0000;
            }
            break;
        }
        default:;
            DPRINT("unknown request for status %x\n", data.bmRequestType);
        }
        sendBuffer(0, Reply, data.wLength);
        break;
    }

    case XUSBPS_REQ_SET_ADDRESS:
        UsbInstance.setDeviceAddress(data.wValue);
        DPRINT("Set address %d\n", data.wValue);
        ack(0);
        break;

    case XUSBPS_REQ_GET_INTERFACE:
        DPRINT("Get interface %d/%d/%d\n", data.wIndex, data.wLength, UsbInstance.getCurrentAltSetting());
        {
            int alt = -1;
            if (data.wIndex == USB_INTERFACE_MIDISTREAM) alt = 0;
            if (alt == -1)
            {
                Error = true;
                break;
            }
            Response = (u8)alt; 
            sendBuffer(0, &Response, 1);
        }
        break;

    case XUSBPS_REQ_GET_DESCRIPTOR:
        // reply descriptors
        switch ((data.wValue >> 8) & 0xff)
        {
        case XUSBPS_TYPE_DEVICE_DESC:
            ReplyLen = GetDeviceDescriptor(Reply, XUSBPS_REQ_REPLY_LEN, UsbInstance.getDeviceSpeed() == usbps::SpeedMode::HIGH_SPEED);
            ReplyLen = ReplyLen > data.wLength ? data.wLength : ReplyLen;
            Status = sendBuffer(0, Reply, ReplyLen);
            if (XST_SUCCESS != Status)
                FAIL();
            break;

        case XUSBPS_TYPE_DEVICE_QUALIFIER:
            ReplyLen = GetDeviceQualifierDescriptor(Reply, XUSBPS_REQ_REPLY_LEN, UsbInstance.getDeviceSpeed() == usbps::SpeedMode::HIGH_SPEED);
            ReplyLen = ReplyLen > data.wLength ? data.wLength : ReplyLen;
            Status = sendBuffer(0, Reply, ReplyLen);
            if (XST_SUCCESS != Status)
                FAIL();
            break;

        case XUSBPS_TYPE_CONFIG_DESC:
            ReplyLen = GetConfigDescriptor(Reply, XUSBPS_REQ_REPLY_LEN, UsbInstance.getDeviceSpeed() == usbps::SpeedMode::HIGH_SPEED);
            DPRINT("get config %d/%d\n", ReplyLen, data.wLength);
            ReplyLen = ReplyLen > data.wLength ? data.wLength : ReplyLen;
            Status = sendBuffer(0, Reply, ReplyLen);
            if (XST_SUCCESS != Status)
                FAIL();
            break;
        
        case XUSBPS_TYPE_OTHER_SPEED_CFG_DESC:
            ReplyLen = GetOtherSpeedConfigDescriptor(Reply, XUSBPS_REQ_REPLY_LEN, UsbInstance.getDeviceSpeed() == usbps::SpeedMode::HIGH_SPEED);
            DPRINT("get config %d/%d\n", ReplyLen, data.wLength);
            ReplyLen = ReplyLen > data.wLength ? data.wLength : ReplyLen;
            Status = sendBuffer(0, Reply, ReplyLen);
            if (XST_SUCCESS != Status)
                FAIL();
            break;

        case XUSBPS_TYPE_STRING_DESC:
            ReplyLen = GetStringDescriptor(Reply, XUSBPS_REQ_REPLY_LEN,
                                           data.wValue & 0xFF);
            ReplyLen = ReplyLen > data.wLength ? data.wLength : ReplyLen;
            Status = sendBuffer(0, Reply, ReplyLen);
            if (XST_SUCCESS != Status)
                FAIL();
            break;
            
        default:
            DPRINT("Get desc unknown %x/%d\n", (data.wValue >> 8) & 0xff, data.wLength);
            Error = true;
            break;
        }
        break;

    case XUSBPS_REQ_SET_CONFIGURATION:
        // activate configuration (only support config=1)
        if ((data.wValue & 0xff) != 1)
        {
            Error = true;
            break;
        }
        // enable endppioint
        UsbInstance.enableEndpoint(USB_CFG_MIDI_STREAM_EP, usbps::EndpointDirection::IN_OUT);
        UsbInstance.primeEndpoint(USB_CFG_MIDI_STREAM_EP, usbps::EndpointDirection::OUT);
        ack(0);
        break;

    case XUSBPS_REQ_GET_CONFIGURATION:
        Response = (u8)UsbInstance.getCurrentAltSetting();
        sendBuffer(0, &Response, 1);
        break;

    case XUSBPS_REQ_CLEAR_FEATURE:
        switch (data.getTarget())
        {
        case usbps::SetupTarget::ENDPOINT:
            if (data.wValue == XUSBPS_ENDPOINT_HALT)
            {
                int EpNum = data.wIndex;
                if (EpNum & 0x80)
                { /* In ep */
                    UsbInstance.clearStallEndpoint(EpNum, usbps::EndpointDirection::IN);
                }
                else
                { /* Out ep */
                    UsbInstance.clearStallEndpoint(EpNum, usbps::EndpointDirection::OUT);
                }
            }
            ack(0);
            break;
        default:
            Error = true;
            break;
        }
        break;

    case XUSBPS_REQ_SET_FEATURE:
        switch (data.getTarget())
        {
        case usbps::SetupTarget::ENDPOINT:
            if (data.wValue == XUSBPS_ENDPOINT_HALT)
            {
                int EpNum = data.wIndex;
                if (EpNum & 0x80)
                { /* In ep */
                    UsbInstance.stallEndpoint(EpNum, usbps::EndpointDirection::IN);
                }
                else
                { /* Out ep */
                    UsbInstance.stallEndpoint(EpNum, usbps::EndpointDirection::OUT);
                }
            }
            ack(0);
            break;
        default:
            Error = true;
            break;
        }
        break;

    case XUSBPS_REQ_SET_INTERFACE:
        // set interface (only support 0 interface)
        DPRINT("set interface %d/%d\n", data.wValue, data.wIndex);
        {
            if (data.wIndex == USB_INTERFACE_MIDISTREAM && data.wValue == 0) ack(0);
            else Error = true;
        }
        break;
    default:
        Error = true;
        break;
    }

    /* Set the send stall bit if there was an error */
    if (Error)
    {
        DPRINT("std dev req %d/%d error, stall 0 in out\n",
               data.bRequest, (data.wValue >> 8) & 0xff);
        stall(0);
    }
}

void USBMidi::update()
{
    // receive midi message from PL and send
    while(this->midi_ph.arrivedMidiMessage(1)){ 
		auto d = this->midi_ph.readMidiMessage(1);
		if(MIDIPeripheral::checkValidMidiMessage(d)){
			u8 head = MIDIPeripheral::getMidiMessageHead(d);

            if ((head & 0xF0) == 0x80 || (head & 0xF0) == 0x90) {
                // note on or note off
                u8 cable_num = 0;
                // cable num, channel message type
                u8 buf[4];
                buf[0] = cable_num << 4 | head >> 4;
                // raw midi data
                buf[1] = head;
                buf[2] = MIDIPeripheral::getMidiMessageData1(d);
                buf[3] = MIDIPeripheral::getMidiMessageData2(d);
                auto status = this->sendBuffer(USB_CFG_MIDI_STREAM_EP, buf, 4);
                if (status != XST_SUCCESS) DPRINT("W: TX FAILED %d\n", status);
            }
		}
	}
    // ignore sysex
	while(this->midi_ph.arrivedSysEx((1))){
		this->midi_ph.readSysEx(1);
	}
    
    // send midi message to PL
    while(outbuffer.getLength() >= 4){
        u8 tmp[4];
		if (outbuffer.readBuffer(tmp, 4) != 4){
			DPRINT("error read size\n");
			continue;
		}
		u8 head = tmp[0];
		u8* data = tmp + 1;
		u8 cable_num = (head&0xF0)>>4;
		u8 midi_command = (head&0x0F);
		if (cable_num != 0) continue;

		switch(midi_command){
            case 0x08: // NOTE OFF
            case 0x09: // NOTE ON
                this->midi_ph.sendMidiMessage(1, data[0], data[1], data[2]);
                break;
            default:
                // ignore other message
                break;
		}
	}

}

void USBMidi::epHandler(u8 endpoint_num, usbps::EndpointEvent event_type, void* data) {
    switch (event_type)
    {
    case usbps::EndpointEvent::DATA_RX:{
        auto buffer = this->receiveBuffer(endpoint_num);
        if (buffer.length > 0) {
            // euqueue ring buffer
            this->outbuffer.writeBuffer(buffer.buffer, buffer.length);
		}
		this->releaseBuffer(buffer);
		if(buffer.status != XST_SUCCESS) DPRINT("W: RX FAILED %d\n", buffer.status);
        break;
	}
    case usbps::EndpointEvent::DATA_TX:
    	break;
    default:
    	DPRINT("W: Unhandled event type %d received on %d\n", event_type, endpoint_num);
        break;
    }
}

