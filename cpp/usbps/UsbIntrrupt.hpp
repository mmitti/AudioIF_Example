#pragma once
#include "UsbPs.h"
#include "UsbDeviceDescriptor.hpp"
#include "UsbPsImpl.h"

namespace dd = usbps::DeviceDescriptor;

namespace usbps
{
    namespace Interrupt
    {
        inline void Handler(void *HandlerRef)
        {
            // from xusbps_intr.c XUsbPs_IntrHandler()
            auto ptr = static_cast<UsbPs *>(HandlerRef);

            // Handle controller (non-endpoint) related interrupts.
            auto IrqSts = XUsbPs_ReadReg(ptr->BaseAddress, XUSBPS_ISR_OFFSET);
            // Clear the interrupt status register.
            XUsbPs_WriteReg(ptr->BaseAddress, XUSBPS_ISR_OFFSET, IrqSts);

            // Nak interrupt, used to respond to host's IN request
            if (IrqSts & XUSBPS_IXR_NAK_MASK)
            {
                // Ack the hardware
                XUsbPs_WriteReg(ptr->BaseAddress, XUSBPS_EPNAKISR_OFFSET, XUsbPs_ReadReg(ptr->BaseAddress, XUSBPS_EPNAKISR_OFFSET));
            }

            // === Handle general interrupts ===
            // RESET interrupt
            if (IrqSts & XUSBPS_IXR_UR_MASK)
            {
                IntrHandleReset(ptr, IrqSts);
                return;
            }

            // call user handker
            if ((IrqSts & ptr->IntHandlerMask) && ptr->IntHandlerFunc)
            {
                (ptr->IntHandlerFunc)(ptr->IntHandlerRef, IrqSts);
            }

            // === Handle Endpoint interrupts ===
            if (IrqSts & XUSBPS_IXR_UI_MASK)
            {
                // ENDPOINT 0 SETUP PACKET HANDLING
                auto EpStat = XUsbPs_ReadReg(ptr->BaseAddress, XUSBPS_EPSTAT_OFFSET);
                if (EpStat & 0x0001)
                {
                    // Handle the setup packet
                    {
                        // from XUsbPs_IntrHandleEp0Setup()
                        // Notify the user.
                        auto &Ep = ptr->Endpoint[0].Out;
                        if (Ep.HandlerFunc)
                        {
                            Ep.HandlerFunc(Ep.HandlerRef, 0, EndpointEvent::SETUP_DATA_RECEIVED, NULL);
                        }
                    }
                    // Re-Prime the endpoint(Endpoint is de-primed if a setup packet comes in.)
                    ptr->primeEndpoint(0, EndpointDirection::OUT);
                }

                // Check for RX and TX complete interrupts
                auto EpCompl = XUsbPs_ReadReg(ptr->BaseAddress, XUSBPS_EPCOMPL_OFFSET);

                // ACK the complete interrupts
                XUsbPs_WriteReg(ptr->BaseAddress, XUSBPS_EPCOMPL_OFFSET, EpCompl);

                // Check OUT (RX) endpoints
                if (EpCompl & XUSBPS_EP_OUT_MASK)
                {
                    IntrHandleRX(ptr, EpCompl);
                }

                // Check IN (TX) endpoints
                if (EpCompl & XUSBPS_EP_IN_MASK)
                {
                    IntrHandleTX(ptr, EpCompl);
                }
            }
        }
        inline void IntrHandleReset(UsbPs *ptr, u32 IrqSts)
        {
            // from xusbps_intr.c XUsbPs_IntrHandleReset()
            // Clear all setup token semaphores
            XUsbPs_WriteReg(ptr->BaseAddress, XUSBPS_EPSTAT_OFFSET, XUsbPs_ReadReg(ptr->BaseAddress, XUSBPS_EPSTAT_OFFSET));
            // Clear all the endpoint complete status bits
            XUsbPs_WriteReg(ptr->BaseAddress, XUSBPS_EPCOMPL_OFFSET, XUsbPs_ReadReg(ptr->BaseAddress, XUSBPS_EPCOMPL_OFFSET));

            // Cancel all endpoint prime status
            auto Timeout = TIMEOUT_COUNTER;
            while ((XUsbPs_ReadReg(ptr->BaseAddress, XUSBPS_EPPRIME_OFFSET) & XUSBPS_EP_ALL_MASK) && --Timeout)
            {
                /* NOP */
            }
            XUsbPs_WriteReg(ptr->BaseAddress, XUSBPS_EPFLUSH_OFFSET, 0xFFFFFFFF);

            // Check reset bit
            if (!(XUsbPs_ReadReg(ptr->BaseAddress, XUSBPS_PORTSCR1_OFFSET) & XUSBPS_PORTSCR_PR_MASK))
            {
                // Require hardware RESET
                if (ptr->IntHandlerFunc)
                {
                    (ptr->IntHandlerFunc)(ptr->IntHandlerRef, IrqSts);
                }
                else
                {
                    for (;;)
                        ;
                }
                return;
            }
            // call user handler
            if (ptr->IntHandlerFunc)
            {
                (ptr->IntHandlerFunc)(ptr->IntHandlerRef, IrqSts);
            }
        }
        inline void IntrHandleTX(UsbPs *ptr, u32 EpCompl)
        {
            // from XUsbPs_IntrHandleTX()
            // Check all endpoints for TX complete bits.
            u32 Mask = 0x00010000;
            auto NumEp = ptr->DeviceConfig.NumEndpoints;

            // Check for every endpoint if its TX complete bit is set.
            for (auto Index = 0; Index < NumEp; Index++, Mask <<= 1)
            {
                if (!(EpCompl & Mask))
                {
                    continue;
                }
                auto& Ep = ptr->Endpoint[Index].In;
                do
                {
                    dd::dTDInvalidateCache(Ep.dTDTail);
                    if (dd::dTDIsActive(Ep.dTDTail))
                    {
                        // buffer has not been sent yet.
                        break;
                    }
                    if (Ep.HandlerFunc)
                    {
                        void *BufPtr;
                        BufPtr = (void *)dd::ReaddTD(Ep.dTDTail, dd::dTDType::dTDUSERDATA);
                        Ep.HandlerFunc(Ep.HandlerRef, Index, EndpointEvent::DATA_TX, BufPtr);
                    }

                    Ep.dTDTail = dd::dTDGetNLP(Ep.dTDTail);
                } while (Ep.dTDTail != Ep.dTDHead);
            }
        }
        inline void IntrHandleRX(UsbPs *ptr, u32 EpCompl)
        {
            // from XUsbPs_IntrHandleRX()

            // Check all endpoints for RX complete bits.
            u32 Mask = 0x00000001;
            auto NumEp = ptr->DeviceConfig.NumEndpoints;

            /* Check for every endpoint if its RX complete bit is set.*/
            for (auto Index = 0; Index < NumEp; Index++, Mask <<= 1)
            {
                int numP = 0;

                if (!(EpCompl & Mask))
                {
                    continue;
                }
                auto& Ep = ptr->Endpoint[Index].Out;

                dd::dTDInvalidateCache(Ep.dTDCurr);

                // Handle all finished dTDs
                while (!dd::dTDIsActive(Ep.dTDCurr))
                {
                    numP += 1;
                    if (Ep.HandlerFunc)
                    {
                        Ep.HandlerFunc(Ep.HandlerRef, Index, EndpointEvent::DATA_RX, NULL);
                    }

                    Ep.dTDCurr = dd::dTDGetNLP(Ep.dTDCurr);
                    dd::dTDInvalidateCache(Ep.dTDCurr);
                }
                // Re-Prime the endpoint.
                ptr->primeEndpoint(Index, EndpointDirection::OUT);
            }
        }

    } // namespace Interrupt
} // namespace usbps
