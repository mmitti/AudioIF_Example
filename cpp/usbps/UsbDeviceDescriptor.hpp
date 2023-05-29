#pragma once
// from xusbps_endpoint.h/c
#include <xil_types.h>
#include <xil_cache.h>
#include "UsbPsImpl.h"
#include "UsbData.hpp"

namespace usbps
{
    namespace DeviceDescriptor
    {

        // ==== Endpoint Device Transfer Descriptor ====
#pragma region dTD
        enum class dTDType : u32
        {
            // Pointer to the next descriptor
            dTDNLP = 0x00,
            // Descriptor Token
            dTDTOKEN = 0x04,
            // Buffer Pointer 0
            dTDBPTR0 = 0x08,
            // Buffer Pointer 1
            dTDBPTR1 = 0x0C,
            // Buffer Pointer 2
            dTDBPTR2 = 0x10,
            // Buffer Pointer 3
            dTDBPTR3 = 0x14,
            // Buffer Pointer 4
            dTDBPTR4 = 0x18,
            // Reserved field
            // dTDRSRVD = 0x1C,
            dTDUSERDATA = 0x1C
        };

        inline dTDType dTDBPTR(u32 n)
        {
            return static_cast<dTDType>((u32)dTDType::dTDBPTR0 + n * 0x04);
        }

        // dTD Next Link Pointer (dTDNLP) bit positions.
        // USB dTD Next Link Pointer Terminate Bit
        static constexpr u32 dTDNLP_T_MASK = 0x00000001;
        // USB dTD Next Link Pointer Address [31:5]
        static constexpr u32 dTDNLP_ADDR_MASK = 0xFFFFFFE0;

        // dTD Token (dTDTOKEN) bit positions.
        // dTD Transaction Error
        static constexpr u32 dTDTOKEN_XERR_MASK = 0x00000008;
        // dTD Data Buffer Error
        static constexpr u32 dTDTOKEN_BUFERR_MASK = 0x00000020;
        // dTD Halted Flag
        static constexpr u32 dTDTOKEN_HALT_MASK = 0x00000040;
        // dTD Active Bit
        static constexpr u32 dTDTOKEN_ACTIVE_MASK = 0x00000080;
        // Multiplier Override Field [1:0]
        static constexpr u32 dTDTOKEN_MULTO_MASK = 0x00000C00;
        // Interrupt on Complete Bit
        static constexpr u32 dTDTOKEN_IOC_MASK = 0x00008000;
        // Transfer Length Field
        static constexpr u32 dTDTOKEN_LEN_MASK = 0x7FFF0000;

        // Reads the content of a field in a Transfer Descriptor.
        inline u32 ReaddTD(USB_dTD *dTDPtr, dTDType Id)
        {
            return (*(u32 *)((u32)(dTDPtr) + (u32)(Id)));
        }

        // Writes a value to a field in a Transfer Descriptor.
        inline void WritedTD(USB_dTD *dTDPtr, dTDType Id, u32 Val)
        {
            (*(u32 *)((u32)(dTDPtr) + (u32)(Id)) = (u32)(Val));
        }

        // flash cache
        inline void dTDInvalidateCache(USB_dTD *ptr)
        {
            Xil_DCacheInvalidateRange((unsigned int)ptr, sizeof(USB_dTD));
        }

        inline void dTDFlushCache(USB_dTD *ptr)
        {
            Xil_DCacheFlushRange((unsigned int)ptr, sizeof(USB_dTD));
        }

        inline void dQHInvalidateCache(USB_dQH *ptr)
        {
            Xil_DCacheInvalidateRange((unsigned int)ptr, sizeof(USB_dQH));
        }

        inline void dQHFlushCache(USB_dQH *ptr)
        {
            Xil_DCacheFlushRange((unsigned int)ptr, sizeof(USB_dQH));
        }

        // Sets the Transfer Length for the given Transfer Descriptor.
        inline void dTDSetTransferLen(USB_dTD* dTDPtr, u32 Len)
        {
            WritedTD(dTDPtr, dTDType::dTDTOKEN,
                     (ReaddTD(dTDPtr, dTDType::dTDTOKEN) &
                      ~dTDTOKEN_LEN_MASK) |
                         ((Len) << 16));
        }

        // Gets the Next Link pointer of the given Transfer Descriptor.
        inline USB_dTD *dTDGetNLP(USB_dTD *dTDPtr)
        {
            return (USB_dTD*)(ReaddTD(dTDPtr, dTDType::dTDNLP) & dTDNLP_ADDR_MASK);
        }

        // Sets the Next Link pointer of the given Transfer Descriptor.
        // inline void dTDSetNLP(USB_dTD* dTDPtr, USB_dTD* NLP)

        // Gets the Transfer Length for the given Transfer Descriptor.
        inline u32 dTDGetTransferLen(USB_dTD *dTDPtr)
        {
            return (ReaddTD(dTDPtr, dTDType::dTDTOKEN) & dTDTOKEN_LEN_MASK) >> 16;
        }

        // Sets the Interrupt On Complete (IOC) bit for the given Transfer
        inline void dTDSetIOC(USB_dTD *dTDPtr)
        {
            WritedTD(dTDPtr, dTDType::dTDTOKEN,
                     ReaddTD(dTDPtr, dTDType::dTDTOKEN) | dTDTOKEN_IOC_MASK);
        }

        // Sets the Terminate bit for the given Transfer Descriptor.
        inline void dTDSetTerminate(USB_dTD *dTDPtr)
        {
            WritedTD(dTDPtr, dTDType::dTDNLP,
                     ReaddTD(dTDPtr, dTDType::dTDNLP) | dTDNLP_T_MASK);
        }

        // Clears the Terminate bit for the given Transfer Descriptor.
        inline void dTDClrTerminate(USB_dTD *dTDPtr)
        {
            WritedTD(dTDPtr, dTDType::dTDNLP,
                     ReaddTD(dTDPtr, dTDType::dTDNLP) & ~dTDNLP_T_MASK);
        }

        // Checks if the given descriptor is active.
        inline bool dTDIsActive(USB_dTD *dTDPtr)
        {
            return ReaddTD(dTDPtr, dTDType::dTDTOKEN) & dTDTOKEN_ACTIVE_MASK;
        }

        // Sets the Active bit for the given Transfer Descriptor.
        inline void dTDSetActive(USB_dTD *dTDPtr)
        {
            WritedTD(dTDPtr, dTDType::dTDTOKEN,
                     ReaddTD(dTDPtr, dTDType::dTDTOKEN) | dTDTOKEN_ACTIVE_MASK);
        }
#pragma endregion
        // ==== Endpoint Device Queue Head ====
#pragma region dQH
        enum class dQHType : u32
        {
            // dQH Configuration
            dQHCFG = 0x00,
            // dQH Current dTD Pointer
            dQHCPTR = 0x04,
            // dTD Next Link Ptr in dQH overlay
            dQHdTDNLP = 0x08,
            // dTD Token in dQH overlay
            dQHdTDTOKEN = 0x0C,
            // USB dQH Setup Buffer 0
            dQHSUB0 = 0x28,
            // USB dQH Setup Buffer 1
            dQHSUB1 = 0x2C
        };

        // dQH Configuration (dQHCFG) bit positions.
        // USB dQH Interrupt on Setup Bit
        static constexpr u32 dQHCFG_IOS_MASK = 0x00008000;
        // USB dQH Maximum Packet Length  Field [10:0]
        static constexpr u32 dQHCFG_MPL_MASK = 0x07FF0000;
        static constexpr u32 dQHCFG_MPL_SHIFT = 16;
        // USB dQH Zero Length Termination Select Bit
        static constexpr u32 dQHCFG_ZLT_MASK = 0x20000000;
        // USB dQH Number of Transactions Field [1:0]
        static constexpr u32 dQHCFG_MULT_MASK = 0xC0000000;
        static constexpr u32 dQHCFG_MULT_SHIFT = 30;

        // Reads the content of a field in a Queue Head.
        inline u32 ReaddQH(USB_dQH *dQHPtr, dQHType Id)
        {
            return (*(u32 *)((u32)(dQHPtr) + (u32)(Id)));
        }

        // Writes a value to a field in a Queue Head.
        inline void WritedQH(USB_dQH *dQHPtr, dQHType Id, u32 Val)
        {
            (*(u32 *)((u32)(dQHPtr) + (u32)(Id)) = (u32)(Val));
        }

        // Sets the Maximum Packet Length field of the give Queue Head.
        inline void dQHSetMaxPacketLen(USB_dQH *dQHPtr, u32 Len)
        {
            WritedQH(dQHPtr, dQHType::dQHCFG,
                     (ReaddQH(dQHPtr, dQHType::dQHCFG) & ~dQHCFG_MPL_MASK) |
                ((Len) << 16));
        }

        // Sets the Interrupt On Setup (IOS) bit for an endpoint.
        inline void dQHSetIOS(USB_dQH *dQHPtr)
        {
            WritedQH(dQHPtr, dQHType::dQHCFG,
                     ReaddQH(dQHPtr, dQHType::dQHCFG) | dQHCFG_IOS_MASK);
        }

        // Clears the Interrupt On Setup (IOS) bit for an endpoint.
        inline void dQHClrIOS(USB_dQH *dQHPtr)
        {
            WritedQH(dQHPtr, dQHType::dQHCFG,
                     ReaddQH(dQHPtr, dQHType::dQHCFG) & ~dQHCFG_IOS_MASK);
        }

        // Enables Zero Length Termination for the endpoint.
        inline void dQHEnableZLT(USB_dQH *dQHPtr)
        {
            WritedQH(dQHPtr, dQHType::dQHCFG,
                     ReaddQH(dQHPtr, dQHType::dQHCFG) & ~dQHCFG_ZLT_MASK);
        }

        // Disables Zero Length Termination for the endpoint.
        inline void dQHDisableZLT(USB_dQH *dQHPtr)
        {
            WritedQH(dQHPtr, dQHType::dQHCFG,
                     ReaddQH(dQHPtr, dQHType::dQHCFG) | dQHCFG_ZLT_MASK);
        }
#pragma endregion
        // ==== Utility functions for dTD/dQH ====
#pragma region dTD_dQH_Util 
        // This function associates a buffer with a Transfer Descriptor.
        inline int dTDAttachBuffer(USB_dTD *dTDPtr, const u8 *BufferPtr, u32 BufferLen)
        {
            // from XUsbPs_dTDAttachBuffer()
            // Check if the buffer is smaller than 16kB.
            if (BufferLen > XUSBPS_dTD_BUF_MAX_SIZE)
            {
                return XST_USB_BUF_TOO_BIG;
            }
            // Get a u32 of the buffer pointer to avoid casting in the following logic operations.
            auto BufAddr = (u32)BufferPtr;
            //Set the buffer pointer 0.
            WritedTD(dTDPtr, dTDBPTR(0), BufAddr);

            // Check if the buffer spans a 4kB boundary.
            if (BufferLen > 0)
            {
                auto BufEnd = BufAddr + BufferLen - 1;
                auto PtrNum = 1;
                while ((BufAddr & 0xFFFFF000) != (BufEnd & 0xFFFFF000))
                {
                    BufAddr = (BufAddr + 0x1000) & 0xFFFFF000;
                    WritedTD(dTDPtr, dTDBPTR(PtrNum), BufAddr);
                    PtrNum++;
                }
            }

            // Set the length of the buffer.
            dTDSetTransferLen(dTDPtr, BufferLen);

            // We remember the buffer pointer in the user data field
            WritedTD(dTDPtr, dTDType::dTDUSERDATA, (u32)BufferPtr);

            return XST_SUCCESS;
        }

        // set the Max PacketLen for the queue head for isochronous EP.
        inline void dQHSetMaxPacketLenISO(USB_dQH *dQHPtr, u32 Len)
        {
            // from XUsbPs_dQHSetMaxPacketLenISO()
            // patch from https://forums.xilinx.com/t5/Processor-System-Design-and-AXI/UsbPs-Mult-bits-settings-for-isochronous-EP/td-p/570014
            u32 Mult = ((Len + ENDPOINT_MAXP_LENGTH - 1) & ENDPOINT_MAXP_MULT_MASK) >> ENDPOINT_MAXP_MULT_SHIFT;
            u32 MaxPktSize = (Mult > 1) ? ENDPOINT_MAXP_LENGTH : Len;
            
            if (MaxPktSize > MAX_PACKET_SIZE)
            {
                return;
            }

            if (Mult > 3)
            {
                return;
            }

            // Set Max packet size 
            WritedQH(dQHPtr, dQHType::dQHCFG,
                (ReaddQH(dQHPtr, dQHType::dQHCFG) & ~dQHCFG_MPL_MASK) | 
                (MaxPktSize << dQHCFG_MPL_SHIFT) );

            // Set Mult to tell hardware how many transactions in each microframe
            WritedQH(dQHPtr, dQHType::dQHCFG,
                (ReaddQH(dQHPtr, dQHType::dQHCFG) & ~dQHCFG_MULT_MASK) |
                (Mult << dQHCFG_MULT_SHIFT));
        }
#pragma endregion
    } // namespace DeviceDescriptor
} // namespace usbps
