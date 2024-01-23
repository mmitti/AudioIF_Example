#include "UsbPs.h"
#include "UsbPsImpl.h"
#include "UsbDeviceDescriptor.hpp"
#include "UsbIntrrupt.hpp"

namespace dd = usbps::DeviceDescriptor;
#define SetBits(RegOffset, Bits) XUsbPs_WriteReg(this->BaseAddress, RegOffset, XUsbPs_ReadReg(this->BaseAddress, RegOffset) | (Bits));
#define ClrBits(RegOffset, Bits) XUsbPs_WriteReg(this->BaseAddress, RegOffset, XUsbPs_ReadReg(this->BaseAddress, RegOffset) & ~(Bits));

namespace usbps
{
#pragma region internal
    void UsbPs::initEndpointList()
    {
        // from xusbps_endpoint.c XUsbPs_EpListInit()
        auto p = (u8 *)this->PhysAligned;
        EndpointConfig *EpCfg = this->DeviceConfig.EpCfg;
        // Set up the Endpoint array.

        // Initialize the Queue Head pointer list.
        for (auto EpNum = 0; EpNum < this->DeviceConfig.NumEndpoints; ++EpNum)
        {
            // OUT Queue Head
            Endpoint[EpNum].Out.dQH = (USB_dQH *)p;
            p += XUSBPS_dQH_ALIGN;

            // IN Queue Head
            Endpoint[EpNum].In.dQH = (USB_dQH *)p;
            p += XUSBPS_dQH_ALIGN;
        }

        // 'p' now points to the first address after the Queue Head list. The Transfer Descriptors start here.

        for (auto EpNum = 0; EpNum < this->DeviceConfig.NumEndpoints; ++EpNum)
        {
            // OUT Descriptors.
            if (EndpointType::NONE != EpCfg[EpNum].Out.Type)
            {
                Endpoint[EpNum].Out.dTDs = (USB_dTD *)p;
                Endpoint[EpNum].Out.dTDCurr = (USB_dTD *)p;
                p += XUSBPS_dTD_ALIGN * EpCfg[EpNum].Out.NumBufs;
            }

            // IN Descriptors.
            if (EndpointType::NONE != EpCfg[EpNum].In.Type)
            {
                Endpoint[EpNum].In.dTDs = (USB_dTD *)p;
                Endpoint[EpNum].In.dTDHead = (USB_dTD *)p;
                Endpoint[EpNum].In.dTDTail = (USB_dTD *)p;
                p += XUSBPS_dTD_ALIGN * EpCfg[EpNum].In.NumBufs;
            }
        }

        // 'p' now points to the first address after the Transfer Descriptors.

        for (auto EpNum = 0; EpNum < this->DeviceConfig.NumEndpoints; ++EpNum)
        {

            if (EndpointType::NONE != EpCfg[EpNum].Out.Type)
            {
                if (0 == EpCfg[EpNum].Out.BufSize)
                {
                    Endpoint[EpNum].Out.dTDBufs = NULL;
                    continue;
                }

                Endpoint[EpNum].Out.dTDBufs = p;
                p += EpCfg[EpNum].Out.BufSize * EpCfg[EpNum].Out.NumBufs;
            }
        }

        // Initialize the endpoint event handlers to NULL.
        for (auto EpNum = 0; EpNum < this->DeviceConfig.NumEndpoints; ++EpNum)
        {
            Endpoint[EpNum].Out.HandlerFunc = NULL;
            Endpoint[EpNum].In.HandlerFunc = NULL;
        }
    }

    void UsbPs::initdQH()
    {
        // from xusbps_endpoint.c XUsbPs_dQHInit()
        EndpointConfig *EpCfg = this->DeviceConfig.EpCfg;

        // Set Transfer Descriptor addresses
        // Set Maximum Packet Size
        // Disable Zero Length Termination (ZLT) for non-isochronous transfers
        // Enable Interrupt On Setup (IOS)
        for (auto EpNum = 0; EpNum < this->DeviceConfig.NumEndpoints; ++EpNum)
        {

            /* OUT Queue Heads.*/
            if (EndpointType::NONE != EpCfg[EpNum].Out.Type)
            {
                dd::WritedQH(this->Endpoint[EpNum].Out.dQH, dd::dQHType::dQHCPTR, (u32)this->Endpoint[EpNum].Out.dTDs);
                // For isochronous, ep max packet size translates to different values in queue head than other types.Also   enable ZLT for isochronous.
                if (EndpointType::ISOCHRONOUS == EpCfg[EpNum].Out.Type)
                {
                    dd::dQHSetMaxPacketLenISO(this->Endpoint[EpNum].Out.dQH, EpCfg[EpNum].Out.MaxPacketSize);
                    dd::dQHEnableZLT(this->Endpoint[EpNum].Out.dQH);
                }
                else
                {
                    dd::dQHSetMaxPacketLen(this->Endpoint[EpNum].Out.dQH, EpCfg[EpNum].Out.MaxPacketSize);
                    dd::dQHDisableZLT(this->Endpoint[EpNum].Out.dQH);
                }

                // Only control OUT needs this
                if (EndpointType::CONTROL == EpCfg[EpNum].Out.Type)
                {
                    dd::dQHSetIOS(this->Endpoint[EpNum].Out.dQH);
                }

                // Set up the overlay next dTD pointer.
                dd::WritedQH(this->Endpoint[EpNum].Out.dQH, dd::dQHType::dQHdTDNLP, (u32)this->Endpoint[EpNum].Out.dTDs);

                dd::dQHFlushCache(this->Endpoint[EpNum].Out.dQH);
            }

            // IN Queue Heads.
            if (EndpointType::NONE != EpCfg[EpNum].In.Type)
            {
                dd::WritedQH(this->Endpoint[EpNum].In.dQH, dd::dQHType::dQHCPTR, (u32)this->Endpoint[EpNum].In.dTDs);

                // Isochronous ep packet size can be larger than 1024.
                if (EndpointType::ISOCHRONOUS == EpCfg[EpNum].In.Type)
                {
                    dd::dQHSetMaxPacketLenISO(this->Endpoint[EpNum].In.dQH, EpCfg[EpNum].In.MaxPacketSize);
                    dd::dQHEnableZLT(this->Endpoint[EpNum].In.dQH);
                }
                else
                {
                    dd::dQHSetMaxPacketLen(this->Endpoint[EpNum].In.dQH, EpCfg[EpNum].In.MaxPacketSize);
                    dd::dQHDisableZLT(this->Endpoint[EpNum].In.dQH);
                }

                dd::dQHFlushCache(this->Endpoint[EpNum].In.dQH);
            }
        }
    }

    int UsbPs::initdTD()
    {
        // from xusbps_endpoint.c XUsbPs_dTDInit()
        EndpointConfig *EpCfg = this->DeviceConfig.EpCfg;

        // Walk through the list of endpoints and initialize their Transfer Descriptors.
        for (auto EpNum = 0; EpNum < this->DeviceConfig.NumEndpoints; ++EpNum)
        {
            int Td;
            int NumdTD;

            EndpointOut &Out = Endpoint[EpNum].Out;
            EndpointIn &In = Endpoint[EpNum].In;

            // OUT Descriptors
            // Set the next link pointer
            // Set the interrupt complete and the active bit
            // Attach the buffer to the dTD
            if (EndpointType::NONE != EpCfg[EpNum].Out.Type)
            {
                NumdTD = EpCfg[EpNum].Out.NumBufs;
            }
            else
            {
                NumdTD = 0;
            }

            for (Td = 0; Td < NumdTD; ++Td)
            {
                int Status;

                int NextTd = (Td + 1) % NumdTD;

                dd::dTDInvalidateCache(&Out.dTDs[Td]);

                // Set NEXT link pointer.
                dd::WritedTD(&Out.dTDs[Td], dd::dTDType::dTDNLP, (u32)&Out.dTDs[NextTd]);

                // Set the OUT descriptor ACTIVE and enable the interrupt on complete.
                dd::dTDSetActive(&Out.dTDs[Td]);
                dd::dTDSetIOC(&Out.dTDs[Td]);

                // Set up the data buffer with the descriptor.
                if (NULL == Out.dTDBufs)
                {
                    dd::dTDFlushCache(&Out.dTDs[Td]);
                    continue;
                }

                Status = dd::dTDAttachBuffer(&Out.dTDs[Td], Out.dTDBufs + (Td * EpCfg[EpNum].Out.BufSize), EpCfg[EpNum].Out.BufSize);
                if (XST_SUCCESS != Status)
                {
                    return XST_FAILURE;
                }

                dd::dTDFlushCache(&Out.dTDs[Td]);
            }

            // IN Descriptors
            // Set the next link pointer
            // Set the Terminate bit to mark it available
            if (EndpointType::NONE != EpCfg[EpNum].In.Type)
            {
                NumdTD = EpCfg[EpNum].In.NumBufs;
            }
            else
            {
                NumdTD = 0;
            }

            for (Td = 0; Td < NumdTD; ++Td)
            {
                int NextTd = (Td + 1) % NumdTD;

                dd::dTDInvalidateCache(&In.dTDs[Td]);

                // Set NEXT link pointer.
                dd::WritedTD(&In.dTDs[Td], dd::dTDType::dTDNLP, (u32)&In.dTDs[NextTd]);

                // Set the IN descriptor's TERMINATE bits.
                dd::dTDSetTerminate(&In.dTDs[Td]);

                dd::dTDFlushCache(&In.dTDs[Td]);
            }
        }

        return XST_SUCCESS;
    }

    int UsbPs::reset()
    {
        // from xusbps.c XUsbPs_Reset()
        // Write a 1 to the RESET bit. The RESET bit is cleared by HW once the RESET is complete.
        XUsbPs_WriteReg(this->BaseAddress, XUSBPS_CMD_OFFSET, XUSBPS_CMD_RST_MASK);
        auto Timeout = TIMEOUT_COUNTER;
        /* Wait for the RESET bit to be cleared by HW. */
        while ((XUsbPs_ReadReg(this->BaseAddress, XUSBPS_CMD_OFFSET) & XUSBPS_CMD_RST_MASK) && --Timeout)
        {
            /* NOP */
        }
        return (0 == Timeout) ? XST_FAILURE : XST_SUCCESS;
    }

#pragma endregion
#pragma region initialize
    UsbPs::UsbPs(u32 baseAddr)
    {
        // from xusbps_g.c, xusbps_sinit.c XUsbPs_LookupConfig()
        this->BaseAddress = baseAddr;
    }

    int UsbPs::init(XScuGic *InterruptPtr, u32 Int_Id)
    {
        // === from xusbps.c XUsbPs_CfgInitialize() ===
        /* Initialize the XUsbPs structure to default values. */
        this->CurrentAltSetting = DEFAULT_ALT_SETTING;
        this->IntHandlerFunc = NULL;
        // ===========================================
        // 割り込みハンドラー登録
        /*
        * Connect the device driver handler that will be called when an
        * interrupt for the device occurs, the handler defined above performs
        * the specific interrupt processing for the device.
        */
        auto Status = XScuGic_Connect(InterruptPtr, Int_Id, (Xil_ExceptionHandler)Interrupt::Handler, (void *)this);

        if (Status != XST_SUCCESS)
            return Status;
        /*
        * Enable the interrupt for the device.
        */
        XScuGic_Enable(InterruptPtr, Int_Id);

        return XST_SUCCESS;
    }

    int UsbPs::setupDevice(u8 *DMAMemory, u32 DMAMemorySize)
    {
        this->DeviceConfig.DMAMemPhys = (u32)DMAMemory;
        memset(DMAMemory, 0, DMAMemorySize);
        Xil_DCacheFlushRange((unsigned int)DMAMemory, DMAMemorySize);
        // === from xusbps_endpoint.c XUsbPs_ConfigureDevice() ===
        int Status;
        /* Align the buffer to a 2048 byte (XUSBPS_dQH_BASE_ALIGN) boundary.*/
        this->PhysAligned = (this->DeviceConfig.DMAMemPhys + XUSBPS_dQH_BASE_ALIGN) & ~(XUSBPS_dQH_BASE_ALIGN - 1);
        /* Initialize the endpoint pointer list data structure. */
        initEndpointList();
        /* Initialize the Queue Head structures in DMA memory. */
        initdQH();
        /* Initialize the Transfer Descriptors in DMA memory.*/
        Status = initdTD();
        if (XST_SUCCESS != Status)
            return XST_FAILURE;
        /* Changing the DEVICE mode requires a controller RESET. */
        if (XST_SUCCESS != reset())
            return XST_FAILURE;

        /* Set the Queue Head List address. */
        XUsbPs_WriteReg(this->BaseAddress, XUSBPS_EPLISTADDR_OFFSET, (u32)this->PhysAligned);

        /* Set the USB mode register to configure DEVICE mode.
        *
        * XUSBPS_MODE_SLOM_MASK note:
        *   Disable Setup Lockout. Setup Lockout is not required as we
        *   will be using the tripwire mechanism when handling setup
        *   packets.
        */
        auto ModeValue = XUSBPS_MODE_CM_DEVICE_MASK | XUSBPS_MODE_SLOM_MASK;

        XUsbPs_WriteReg(this->BaseAddress, XUSBPS_MODE_OFFSET, ModeValue);

        SetBits(XUSBPS_OTGCSR_OFFSET, XUSBPS_OTGSC_OT_MASK);

        // =======================================================

        return XST_SUCCESS;
    }
#pragma endregion
#pragma region general
    void UsbPs::setInterrupt(InterruptHandlerFunc CallBackFunc, void *CallBackRef, u32 Mask)
    {
        // from xusbps_intr.c XUsbPs_IntrSetHandler()
        this->IntHandlerFunc = CallBackFunc;
        this->IntHandlerRef = CallBackRef;
        this->IntHandlerMask = Mask;
    }

    void UsbPs::enableInterrupt(u32 Mask)
    {
        // from xusbps.h XUsbPs_IntrEnable()
        SetBits(XUSBPS_IER_OFFSET, Mask);
    }

    void UsbPs::disableInterrupt(u32 Mask)
    {
        // from xusbps.h XUsbPs_IntrDisable()
        ClrBits(XUSBPS_IER_OFFSET, Mask);
    }

    void UsbPs::start()
    {
        // from xusbps.h XUsbPs_Start()
        SetBits(XUSBPS_CMD_OFFSET, XUSBPS_CMD_RS_MASK);
    }

    void UsbPs::stop()
    {
        // from xusbps.h XUsbPs_Stop()
        ClrBits(XUSBPS_CMD_OFFSET, XUSBPS_CMD_RS_MASK);
    }

    int UsbPs::setDeviceAddress(u8 Address)
    {
        // from xusbps.c XUsbPs_SetDeviceAddress()
        if (Address > XUSBPS_DEVICEADDR_MAX)
        {
            return XST_INVALID_PARAM;
        }
        // set address register
        XUsbPs_WriteReg(this->BaseAddress, XUSBPS_DEVICEADDR_OFFSET, (Address << XUSBPS_DEVICEADDR_ADDR_SHIFT) | XUSBPS_DEVICEADDR_DEVICEAADV_MASK);
        return XST_SUCCESS;
    }
    
    SpeedMode UsbPs::getDeviceSpeed() {
        return (SpeedMode) (XUsbPs_ReadReg(this->BaseAddress, XUSBPS_PORTSCR1_OFFSET) & XUSBPS_PORTSCR_PSPD_MASK);
    }
#pragma endregion
#pragma region endpoin
    void UsbPs::setEndpointHandler(const u8 endpoint, EndpointDirection direction, EndpointHandlerFunc CallBackFunc, void *CallBackRef)
    {
        // from xusbps_endpoint.c XUsbPs_EpSetHandler()
        auto &Ep = this->Endpoint[endpoint & 0xF];

        if (direction == EndpointDirection::OUT || direction == EndpointDirection::IN_OUT)
        {
            Ep.Out.HandlerFunc = CallBackFunc;
            Ep.Out.HandlerRef = CallBackRef;
        }
        if (direction == EndpointDirection::IN || direction == EndpointDirection::IN_OUT)
        {
            Ep.In.HandlerFunc = CallBackFunc;
            Ep.In.HandlerRef = CallBackRef;
        }
    }

    bool UsbPs::primeEndpoint(const u8 endpoint, EndpointDirection direction, bool wait)
    {
        // from xusbps_endpoint.c XUsbPs_EpPrime()
        u32 Mask;
        // Get the right bit mask for the endpoint direction.
        switch (direction)
        {
        case EndpointDirection::OUT:
            Mask = 0x00000001;
            break;
        case EndpointDirection::IN:
            Mask = 0x00010000;
            break;
        default:
            return false;
        }
        Mask = Mask << (endpoint & 0xF);
        // Write the endpoint prime register.
        XUsbPs_WriteReg(this->BaseAddress, XUSBPS_EPPRIME_OFFSET, Mask);
        if (!wait)
            return true;
        auto Timeout = TIMEOUT_COUNTER;
        u32 Reg;
        do
        {
            Reg = XUsbPs_ReadReg(this->BaseAddress, XUSBPS_EPPRIME_OFFSET);
        } while (((Reg & Mask) == 1) && --Timeout);
        if (!Timeout)
            return false;
        return true;
    }

    void UsbPs::enableEndpoint(const u8 endpoint, EndpointDirection direction)
    {
        // from xusbps.h XUsbPs_EpEnable()
        const auto reg = XUSBPS_EPCRn_OFFSET(endpoint & 0xF);
        SetBits(reg,
                ((direction == EndpointDirection::OUT || direction == EndpointDirection::IN_OUT) ? XUSBPS_EPCR_RXE_MASK : 0) |
                    ((direction == EndpointDirection::IN || direction == EndpointDirection::IN_OUT) ? XUSBPS_EPCR_TXE_MASK : 0));
        // ===============================
        // data toggle reset
        auto mask = XUSBPS_EPCR_TXR_MASK | XUSBPS_EPCR_RXR_MASK;
        // endpoint type
        if (direction == EndpointDirection::IN || direction == EndpointDirection::IN_OUT)
        {
            switch (this->DeviceConfig.EpCfg[endpoint & 0xF].In.Type)
            {
            case EndpointType::BULK:
                mask |= XUSBPS_EPCR_TXT_BULK_MASK;
                break;
            case EndpointType::INTERRUPT:
                mask |= XUSBPS_EPCR_TXT_INTR_MASK;
                break;
            case EndpointType::ISOCHRONOUS:
                mask |= XUSBPS_EPCR_TXT_ISO_MASK;
                break;
            default:
                break;
            }
        }
        if (direction == EndpointDirection::OUT || direction == EndpointDirection::IN_OUT)
        {
            switch (this->DeviceConfig.EpCfg[endpoint & 0xF].In.Type)
            {
            case EndpointType::BULK:
                mask |= XUSBPS_EPCR_RXT_BULK_MASK;
                break;
            case EndpointType::INTERRUPT:
                mask |= XUSBPS_EPCR_RXT_INTR_MASK;
                break;
            case EndpointType::ISOCHRONOUS:
                mask |= XUSBPS_EPCR_RXT_ISO_MASK;
                break;
            default:
                break;
            }
        }
        SetBits(reg, mask);
    }
    
    int UsbPs::getSetupData(const u8 endpoint, SetupData &data)
    {
        // from xusbps_endpoint.c XUsbPs_EpGetSetupData()
        u32 Data[2];
        auto &Ep = this->Endpoint[endpoint & 0xF].Out;
        // Get the data from the Queue Heads Setup buffer
        do
        {
            // Arm tripwire.It tell us if a new setup packet arrived.
            {
                // from xusbps.h XUsbPs_SetSetupTripwire()
                SetBits(XUSBPS_CMD_OFFSET, XUSBPS_CMD_SUTW_MASK);
            }
            dd::dQHInvalidateCache(Ep.dQH);
            Data[0] = dd::ReaddQH(Ep.dQH, dd::dQHType::dQHSUB0);
            Data[1] = dd::ReaddQH(Ep.dQH, dd::dQHType::dQHSUB1);
        } while (
            // from xusbpbs.h XUsbPs_SetupTripwireIsSet()
            (XUsbPs_ReadReg(this->BaseAddress, XUSBPS_CMD_OFFSET) & XUSBPS_CMD_SUTW_MASK) == 0);
        // Clear the pending endpoint setup stat bit.
        XUsbPs_WriteReg(this->BaseAddress, XUSBPS_EPSTAT_OFFSET, 1 << (endpoint & 0xF));
        // Clear the Tripwire bit and continue.
        {
            // from xusbps.h XUsbPs_ClrSetupTripwire()
            ClrBits(XUSBPS_CMD_OFFSET, XUSBPS_CMD_SUTW_MASK)
        }
        // read data
        auto p = (u8 *)Data;
        data.bmRequestType = p[0];
        data.bRequest = p[1];
        data.wValue = (p[3] << 8) | p[2];
        data.wIndex = (p[5] << 8) | p[4];
        data.wLength = (p[7] << 8) | p[6];

        // check setup bit has cleared.
        auto Timeout = TIMEOUT_COUNTER;
        while ((XUsbPs_ReadReg(this->BaseAddress, XUSBPS_EPSTAT_OFFSET) & (1 << (endpoint & 0xF))) && --Timeout)
        {
            // NOP
        }

        if (0 == Timeout)
        {
            return XST_FAILURE;
        }
        return XST_SUCCESS;
    }

    int UsbPs::receiveBuffer(const u8 endpoint, u8 **BufferPtr, u32 *BufferLenPtr, u32 *Handle)
    {
        // Locate the next available buffer in the ring.
        auto &Ep = this->Endpoint[endpoint & 0xF].Out;
        dd::dTDInvalidateCache(Ep.dTDCurr);
        if (dd::dTDIsActive(Ep.dTDCurr))
        {
            return XST_USB_NO_BUF;
        }
        auto &EpSetup = this->DeviceConfig.EpCfg[endpoint & 0xF].Out;
        // Use the buffer pointer stored in the "user data" field
        *BufferPtr = (u8 *)dd::ReaddTD(Ep.dTDCurr, dd::dTDType::dTDUSERDATA);

        u32 length = EpSetup.BufSize - dd::dTDGetTransferLen(Ep.dTDCurr);

        if (length > 0)
            *BufferLenPtr = length;
        else
            *BufferLenPtr = 0;

        *Handle = (u32)Ep.dTDCurr;
        // Reset the descriptor's BufferPointer0 and Transfer Length fields to their original value.
        dd::WritedTD(Ep.dTDCurr, dd::dTDType::dTDBPTR0, (u32)*BufferPtr);
        dd::dTDSetTransferLen(Ep.dTDCurr, EpSetup.BufSize);
        dd::dTDFlushCache(Ep.dTDCurr);
        return XST_SUCCESS;
    }

    void UsbPs::releaseBuffer(u32 Handle)
    {
        // from xusbps_endpoint.c XUsbPs_EpBufferRelease()
        auto dTDPtr = (USB_dTD *)Handle;
        // Activate the descriptor and clear the Terminate bit
        dd::dTDInvalidateCache(dTDPtr);
        dd::dTDClrTerminate(dTDPtr);
        dd::dTDSetActive(dTDPtr);
        dd::dTDSetIOC(dTDPtr);
        dd::dTDFlushCache(dTDPtr);
    }

    void UsbPs::stallEndpoint(const u8 endpoint, EndpointDirection direction)
    {
        // from xusbps.h XUsbPs_EpStall()
        SetBits(XUSBPS_EPCRn_OFFSET(endpoint & 0xF),
                ((direction == EndpointDirection::OUT || direction == EndpointDirection::IN_OUT) ? XUSBPS_EPCR_RXS_MASK : 0) |
                    ((direction == EndpointDirection::IN || direction == EndpointDirection::IN_OUT) ? XUSBPS_EPCR_TXS_MASK : 0));
    }

    void UsbPs::clearStallEndpoint(const u8 endpoint, EndpointDirection direction)
    {
        ClrBits(XUSBPS_EPCRn_OFFSET(endpoint & 0xF),
                ((direction == EndpointDirection::OUT || direction == EndpointDirection::IN_OUT) ? XUSBPS_EPCR_RXS_MASK : 0) |
                    ((direction == EndpointDirection::IN || direction == EndpointDirection::IN_OUT) ? XUSBPS_EPCR_TXS_MASK : 0));
    }
    bool UsbPs::isStallEndpoint(const u8 endpoint, EndpointDirection direction)
    {
        auto reg = XUsbPs_ReadReg(this->BaseAddress, XUSBPS_EPCRn_OFFSET(endpoint & 0xF));
        switch (direction)
        {
        case EndpointDirection::IN:
            return reg & XUSBPS_EPCR_TXS_MASK;
        case EndpointDirection::OUT:
            return reg & XUSBPS_EPCR_RXS_MASK;
            break;
        default:
            break;
        }
        return false;
    }

    int UsbPs::sendBuffer(const u8 endpoint, const u8 *BufferPtr, u32 BufferLen)
    {
        // xusbps_endpoint.c XUsbPs_EpBufferSend()
        //                   XUsbPs_EpQueueRequest()
        // u8 ReqZero = 0;
        u32 Token;
        u32 PipeEmpty = 1;
        constexpr u32 Mask = 0x00010000;
        const u32 BitMask = Mask << (endpoint & 0xF);
        u32 RegValue;
        u32 Temp;
        u32 exit = 1;
        auto &Ep = this->Endpoint[endpoint & 0xF].In;

        Xil_DCacheFlushRange((unsigned int)BufferPtr, BufferLen);

        if (Ep.dTDTail != Ep.dTDHead)
            PipeEmpty = 0;
        dd::dTDInvalidateCache(Ep.dTDHead);
        // Tell the caller if we do not have any descriptors available.
        if (dd::dTDIsActive(Ep.dTDHead))
            return XST_USB_NO_DESC_AVAILABLE;
        // Remember the current head.
        auto DescPtr = Ep.dTDHead;
        do
        {
            // Tell the caller if we do not have any descriptors available.
            if (dd::dTDIsActive(Ep.dTDHead))
                return XST_USB_NO_DESC_AVAILABLE;
            u32 Length = (BufferLen > XUSBPS_dTD_BUF_MAX_SIZE) ? XUSBPS_dTD_BUF_MAX_SIZE : BufferLen;
            // Attach the provided buffer to the current descriptor.
            if (dd::dTDAttachBuffer(Ep.dTDHead, BufferPtr, Length) != XST_SUCCESS)
            {
                return XST_FAILURE;
            }

            BufferLen -= Length;
            BufferPtr += Length;

            dd::dTDSetActive(Ep.dTDHead);

            if (BufferLen == 0) // && (ReqZero == FALSE)
            {
                dd::dTDSetIOC(Ep.dTDHead);
                exit = 0;
            }
            dd::dTDClrTerminate(Ep.dTDHead);
            dd::dTDFlushCache(Ep.dTDHead);
            // Advance the head descriptor pointer to the next descriptor.
            Ep.dTDHead = dd::dTDGetNLP(Ep.dTDHead);
            // Terminate the next descriptor and flush the cache.
            dd::dTDInvalidateCache(Ep.dTDHead);
            // if (ReqZero && BufferLen == 0) ReqZero = FALSE;
        } while (BufferLen || exit);

        dd::dTDSetTerminate(Ep.dTDHead);
        dd::dTDFlushCache(Ep.dTDHead);

        if (!PipeEmpty)
        {
            // Read the endpoint prime register.
            RegValue = XUsbPs_ReadReg(this->BaseAddress, XUSBPS_EPPRIME_OFFSET);
            if (RegValue & BitMask)
                return XST_SUCCESS;
            do
            {
                RegValue = XUsbPs_ReadReg(this->BaseAddress, XUSBPS_CMD_OFFSET);
                XUsbPs_WriteReg(this->BaseAddress, XUSBPS_CMD_OFFSET, RegValue | XUSBPS_CMD_ATDTW_MASK);
                Temp = XUsbPs_ReadReg(this->BaseAddress, XUSBPS_EPRDY_OFFSET) & BitMask;
            } while (!(XUsbPs_ReadReg(this->BaseAddress, XUSBPS_CMD_OFFSET) & XUSBPS_CMD_ATDTW_MASK));
            RegValue = XUsbPs_ReadReg(this->BaseAddress, XUSBPS_CMD_OFFSET);
            XUsbPs_WriteReg(this->BaseAddress, XUSBPS_CMD_OFFSET, RegValue & ~XUSBPS_CMD_ATDTW_MASK);
            if (Temp)
                return XST_SUCCESS;
        }
        // Check, if the DMA engine is still running.
        dd::dQHInvalidateCache(Ep.dQH);
        // Add the dTD to the dQH 
        dd::WritedQH(Ep.dQH, dd::dQHType::dQHdTDNLP, (u32)DescPtr);
        Token = dd::ReaddQH(Ep.dQH, dd::dQHType::dQHdTDTOKEN);
        Token &= ~(dd::dTDTOKEN_ACTIVE_MASK | dd::dTDTOKEN_HALT_MASK);
        dd::WritedQH(Ep.dQH, dd::dQHType::dQHdTDTOKEN, Token);

        dd::dQHFlushCache(Ep.dQH);
        primeEndpoint(endpoint, EndpointDirection::IN);
        return XST_SUCCESS;
    }
#pragma endregion
#pragma region utility
    EndpointBuffer UsbPs::receiveBuffer(const u8 endpoint, bool pooling)
    {
        EndpointBuffer buffer;
        buffer.handle = 0;
        u32 Timeout = TIMEOUT_COUNTER;
        do
        {
            buffer.status = receiveBuffer(endpoint, &buffer.buffer, &buffer.length, &buffer.handle);
        } while (pooling && (buffer.status != XST_SUCCESS) && --Timeout);
        if (buffer.status != XST_SUCCESS)
        {
            buffer.length = 0;
            return buffer;
        }

        if (buffer.length > 0)
        {
            auto invalidate_length = buffer.length;
            if (buffer.length % 32)
                invalidate_length = (buffer.length / 32) * 32 + 32;
            Xil_DCacheInvalidateRange((unsigned int)buffer.buffer, invalidate_length);
        }
        return buffer;
    }

    void UsbPs::releaseBuffer(EndpointBuffer &buffer)
    {
        if (buffer.status == XST_SUCCESS)
            this->releaseBuffer(buffer.handle);
    }
#pragma endregion
} // namespace usbps
