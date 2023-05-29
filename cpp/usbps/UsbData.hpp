#pragma once
// USB psを利用するアプリケーション用。共通で利用するデータ構造
#include <xil_types.h>
#include <xusbps_hw.h>
namespace usbps
{
    // from xusbps.h
    
    typedef void (*InterruptHandlerFunc)(void *CallBackRef, u32 IrqMask);
    typedef u8  USB_dQH[XUSBPS_dQH_ALIGN];
    typedef u8  USB_dTD[XUSBPS_dTD_ALIGN];

    enum class EndpointType : u32
    {
        // Endpoint is not used.
        NONE = 0,
        // Endpoint for Control Transfers.
        CONTROL = 1,
        // Endpoint for isochronous data.
        ISOCHRONOUS = 2,
        // Endpoint for BULK Transfers.
        BULK = 3,
        // Endpoint for interrupt Transfers.
        INTERRUPT = 4,
    };
    enum class EndpointDirection {
        IN = 1,
        OUT = 2,
        IN_OUT = 3,
    };
    enum class EndpointEvent {
        // Setup data has been received on the endpoint.
        SETUP_DATA_RECEIVED = 0x01,
        // Data frame has been received on the endpoint. 
        DATA_RX = 0x02,
        // Data frame has been sent on the endpoint.
        DATA_TX = 0x03,
    };
    
    struct EndpointBuffer{
        u8* buffer;
        u32 length;
        int status;
        u32 handle;
    };

    
    typedef void (*EndpointHandlerFunc)(void *CallBackRef, u8 EpNum, EndpointEvent EventType, void *Data);


    struct EndpointSetup
    {
        EndpointType Type;
        // Number of buffers to be handled by this endpoint.
        u32 NumBufs;
        // Buffer size. Only relevant for OUT (receive) Endpoints.
        u32 BufSize;
        // Maximum packet size for this endpoint. This number will define the maximum number of bytes sent on the wire per transaction. Range: 0..1024
        u16 MaxPacketSize;
    };

    struct EndpointConfig
    {
        EndpointSetup Out;
        EndpointSetup In;
    };
    
    struct EndpointOut {
        // Pointer to the Queue Head structure of the endpoint.
        USB_dQH  *dQH;
        // Pointer to the first dTD of the dTD list for this endpoint.
        USB_dTD  *dTDs;
        // Buffer to the currently processed descriptor.
        USB_dTD  *dTDCurr;
        // Pointer to the first buffer of the buffer list for this endpoint.
        u8  *dTDBufs;
        // Handler function for this endpoint.
        EndpointHandlerFunc HandlerFunc;
        // User data reference for the handler.
        void *HandlerRef;
    };

    struct EndpointIn {
        // Pointer to the Queue Head structure of the endpoint.
        USB_dQH  *dQH;
        // List of pointers to the Transfer Descriptors of the endpoint.
        USB_dTD  *dTDs;
        // Buffer to the next available descriptor in the list.
        USB_dTD  *dTDHead;
        // Buffer to the last unsent descriptor in the list.
        USB_dTD  *dTDTail;
        // Handler function for this endpoint.
        EndpointHandlerFunc HandlerFunc;
        // User data reference for the handler.
        void *HandlerRef;
    };

    struct Endpoint
    {
        EndpointOut Out;
        EndpointIn  In;
    };

    struct DeviceConfig
    {
        // Number of Endpoints for the controller. This number depends on the runtime configuration of driver. The driver may configure fewer endpoints than are available in the core.
        u8 NumEndpoints;
        // List of endpoint configurations.
        EndpointConfig EpCfg[XUSBPS_MAX_ENDPOINTS];
        // Physical base address of DMAable memory allocated for the driver.
        u32 DMAMemPhys;
    };


    enum class SetupDirection : u8{
        IN = 0x80,
        OUT = 0x00,
    };
    enum class SetupTarget : u8{
        DEVICE = 0x0,
        INTERFACE = 0x1,
        ENDPOINT = 0x2
    };
    enum class SetupType : u8 {
        STANDARD =  0x00,
        CLASS =  0x20,
        VENDOR =  0x40,
    };
    
    constexpr u8 STATUS_DIR_MASK = 0x80;
    constexpr u8 STATUS_TYPE_MASK = 0x60;
    constexpr u8 STATUS_TARGET_MASK = 0x03;
    struct SetupData
    {
        // bmRequestType in setup data
        u8 bmRequestType;
        // bRequest in setup data
        u8 bRequest;
        // wValue in setup data
        u16 wValue;
        // wIndex in setup data
        u16 wIndex;
        // wLength in setup data
        u16 wLength;
        
        inline SetupDirection getDir() const {
            return static_cast<SetupDirection>(bmRequestType & STATUS_DIR_MASK);
        }
        inline SetupTarget getTarget() const {
            return static_cast<SetupTarget>(bmRequestType & STATUS_TARGET_MASK);
        }inline SetupType getType() const {
            return static_cast<SetupType>(bmRequestType & STATUS_TYPE_MASK);
        }
    };
} // namespace usbps
