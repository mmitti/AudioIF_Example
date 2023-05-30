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

USBMidi::USBMidi(XScuGic &intc, MIDIPeripheral &midi_ph, u16 deviceId, u32 baseAddr) : IntcInstance(intc), midi_ph(midi_ph), UsbInstance(deviceId, baseAddr){};

bool USBMidi::init()
{
    constexpr u16 UsbIntrId = XPAR_XUSBPS_0_INTR;
    int Status;
    // init usb
    Status = UsbInstance.init(&IntcInstance, UsbIntrId);
    if (XST_SUCCESS != Status)
        return false;

    // Endpoint の設定
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

    UsbInstance.DeviceConfig.NumEndpoints = 4;

    Status = UsbInstance.setupDevice((u8 *)&Buffer[0], MEMORY_SIZE);
    if (XST_SUCCESS != Status)
        return false;

    // 割り込み登録
    UsbInstance.setInterrupt(UsbIntrHandler, this, XUSBPS_IXR_SR_MASK);
    UsbInstance.setEndpointHandler(0, usbps::EndpointDirection::OUT, Ep0EventHandler, this);
    UsbInstance.setEndpointHandler(3, usbps::EndpointDirection::IN_OUT, EpEventHandler, this);
    // 割り込み有効
    UsbInstance.enableInterrupt(XUSBPS_IXR_SR_MASK); // XUSBPS_IXR_UR_MASK | XUSBPS_IXR_UI_MASK
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
    
    /* Disconnect and disable the interrupt for the USB controller. */
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

        // Setupパケットの処理をする
        // DPRINT("Handle setup packet\n");
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
                /* Get the Setup DATA, don't wait for the interrupt */
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
            /* Stall on Endpoint 0 */
            DPRINT("unknown class req, stall 0 in out\n");
            stall(0);
            break;
        }

        break;

    // Controlエンドポイントに来たデータは破棄する
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
        // index: i/f
        DPRINT("Get interface %d/%d/%d\n", data.wIndex, data.wLength, UsbInstance.getCurrentAltSetting());
        {
            int alt = -1;
            if (data.wIndex == USB_INTERFACE_MIDISTREAM) alt = 0;
            if (alt == -1)
            {
                Error = true;
                break;
            }
            Response = (u8)alt; // interface alt id
            /* Ack the host */
            sendBuffer(0, &Response, 1);
        }
        break;

    case XUSBPS_REQ_GET_DESCRIPTOR:
        /* Get descriptor type. */
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
        // Configrationは1のみ受付
        if ((data.wValue & 0xff) != 1)
        {
            Error = true;
            break;
        }
        // Endpoint 有効化
        UsbInstance.enableEndpoint(3, usbps::EndpointDirection::IN_OUT);
        /* Prime the OUT endpoint. */
        UsbInstance.primeEndpoint(3, usbps::EndpointDirection::OUT);
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
        // index : i/f value:alt id
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
    // todo send midi data
}

void USBMidi::epHandler(u8 endpoint_num, usbps::EndpointEvent event_type, void* data) {
    switch (event_type)
    {
    case usbps::EndpointEvent::DATA_RX:{
        auto buffer = receiveBuffer(endpoint_num);
        if (buffer.length > 0) {
            // TODO receive midi data
			// usbMidiEventHandler(buffer.buffer, buffer.length);
		}
		releaseBuffer(buffer);
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

