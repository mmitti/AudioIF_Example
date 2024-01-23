#include "SWPDPeripheral.h"
#include "SWPDReg.h"
#include "xswpd.h"
#include <xil_io.h>
#include <xil_cache.h>

#define SWPD_WriteReg(RegOffset, Data)  Xil_Out32((baseAddress) + (RegOffset), (u32)(Data))
#define SWPD_ReadReg(RegOffset)   Xil_In32((baseAddress) + (RegOffset))
#define SWPD_SetBit(RegOffset, Mask) {auto reg = SWPD_ReadReg(RegOffset);SWPD_WriteReg(RegOffset, reg | Mask);}
#define SWPD_ClearBit(RegOffset, Mask) {auto reg = SWPD_ReadReg(RegOffset);SWPD_WriteReg(RegOffset, reg & ~Mask);}
#define SWPD_SetClearBit(RegOffset, Mask, bit) {if(bit) SWPD_SetBit(RegOffset, Mask) else SWPD_ClearBit(RegOffset, Mask)}
#define SWPD_GetBit(RegOffset, Mask) ((SWPD_ReadReg(RegOffset) & Mask) == Mask)
#define SWPD_GetValue(RegOffset, Mask, Shift) ((SWPD_ReadReg(RegOffset) & Mask) >> Shift)

extern XSWPD_Config XSWPD_ConfigTable[];

SWPDPeripheral::SWPDPeripheral(u32 baseAddress) {
    for (int i = 0; XSWPD_ConfigTable[i].Name != NULL; i++) {
        if ((XSWPD_ConfigTable[i].BaseAddress == baseAddress) || !baseAddress) {
            this->baseAddress = XSWPD_ConfigTable[i].BaseAddress;
           	break;
       }
    }
}

bool SWPDPeripheral::init(){
    SWPD_SetBit(SWPD_S_AXI_CONTROL_OFFSET, SWPD_CONTROL_ENABLE_MASK);
    return true;
}

bool SWPDPeripheral::request(u8 cmd, u8 len) {
    if (this->isRunning()) return false;
    u32 data = (cmd << SWPD_SEND_COMMAND_SHIFT) | (len << SWPD_SEND_LENGTH_SHIFT);
    auto reg = SWPD_ReadReg(SWPD_S_AXI_SEND_OFFSET);
    data = (data & (SWPD_SEND_COMMAND_MASK | SWPD_SEND_LENGTH_MASK)) | (reg & ~((SWPD_SEND_COMMAND_MASK | SWPD_SEND_LENGTH_MASK)));
    SWPD_WriteReg(SWPD_S_AXI_SEND_OFFSET, data);
    return true;
}

bool SWPDPeripheral::isRunning() {
    return SWPD_GetBit(SWPD_S_AXI_CONTROL_OFFSET, SWPD_CONTROL_RUNNING_MASK);
}

bool SWPDPeripheral::isDone() {
    return SWPD_GetBit(SWPD_S_AXI_CONTROL_OFFSET, SWPD_CONTROL_DONE_MASK);
}

bool SWPDPeripheral::isTransferError() {
    return SWPD_GetBit(SWPD_S_AXI_CONTROL_OFFSET, SWPD_CONTROL_TRANSFER_ERROR_MASK);
}

u16 SWPDPeripheral::getResult() {
    if (this->isDone()) {
        return SWPD_GetValue(SWPD_S_AXI_RECEIVE_OFFSET, SWPD_RECEIVE_REPLY_MASK, SWPD_RECEIVE_REPLY_SHIFT);
    }
    return 0;
}
