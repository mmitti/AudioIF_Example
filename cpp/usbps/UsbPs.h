#pragma once
#include "UsbData.hpp"
#include <xscugic.h>
namespace usbps
{
    class UsbPs;
    namespace Interrupt {
        void Handler(void*);
        void IntrHandleReset(UsbPs*, u32);
        void IntrHandleTX(UsbPs*, u32);
        void IntrHandleRX(UsbPs*, u32);
    }
    
    
    enum SpeedMode : uint32_t {
        FULL_SPEED = 0x00000000,
        LOW_SPEED = 0x04000000,
        HIGH_SPEED = 0x08000000,
        NOT_CONNECTED = 0x0C000000
    };
    // from xusbps.h
    class UsbPs
    {
        friend void Interrupt::Handler(void*);
        friend void Interrupt::IntrHandleReset(UsbPs*, u32);
        friend void Interrupt::IntrHandleTX(UsbPs*, u32);
        friend void Interrupt::IntrHandleRX(UsbPs*, u32);
    private:
        // === XUsbPs_Config ===
        // Unique ID of controller.
        u16 DeviceID;
        // Core register base address.
        u32 BaseAddress;
        // =====================

        // Current alternative setting of interface
        int CurrentAltSetting;

        // === XUsbPs_DeviceConfig ===
        // List of endpoint metadata structures. (for INTERNAL)
        ::usbps::Endpoint Endpoint[XUSBPS_MAX_ENDPOINTS];
        // 64 byte aligned base address of the DMA memory block. (for INTERNAL)
        u32 PhysAligned;
        // ===========================

        // Handler function for the controller.
        InterruptHandlerFunc IntHandlerFunc;
        // User data reference for the handler.
        void *IntHandlerRef;
        u32 IntHandlerMask;
        void initEndpointList();
        void initdQH();
        int initdTD();
        int reset();
    public:
        ::usbps::DeviceConfig DeviceConfig;
        // Initializer
        UsbPs(u16 deviceId, u32 baseAddr);
        int init(XScuGic *InterruptPtr, u32 Int_Id);
        int setupDevice(u8* DMAMemory, u32 DMAMemorySize);
        
        // General
        void setInterrupt(InterruptHandlerFunc CallBackFunc, void *CallBackRef, u32 Mask);
        void enableInterrupt(u32 Mask);
        void disableInterrupt(u32 Mask);
        void start();
        void stop();
        int setDeviceAddress(u8 Address);
        int getCurrentAltSetting() {return CurrentAltSetting;}
        SpeedMode getDeviceSpeed();

        // Endpoint
        void setEndpointHandler(const u8 endpoint, EndpointDirection direction, EndpointHandlerFunc CallBackFunc, void *CallBackRef);
        bool primeEndpoint(const u8 endpoint, EndpointDirection direction, bool wait = false);
        void enableEndpoint(const u8 endpoint, EndpointDirection direction);
        int getSetupData(const u8 endpoint, SetupData& data);
        int receiveBuffer(const u8 endpoint, u8** BufferPtr, u32* BufferLenPtr, u32 *Handle);
        void releaseBuffer(u32 Handle);
        void stallEndpoint(const u8 endpoint, EndpointDirection direction);
        void clearStallEndpoint(const u8 endpoint, EndpointDirection direction);
        bool isStallEndpoint(const u8 endpoint, EndpointDirection direction);
        int sendBuffer(const u8 endpoint, const u8 *BufferPtr, u32 BufferLen);
        // Utility
        EndpointBuffer receiveBuffer(const u8 endpoint, bool pooling = false);
        void releaseBuffer(EndpointBuffer& buffer);
    };
} // namespace usbps
