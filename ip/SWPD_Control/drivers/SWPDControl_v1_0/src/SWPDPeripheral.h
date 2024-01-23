#pragma once

#include "xil_types.h"
#include "xstatus.h"
#include <cstdint>

class SWPDPeripheral{
private:
    u32 baseAddress;
public:
    SWPDPeripheral(u32 baseAddress);
    
    bool init();
    
    bool request(u8 cmd, u8 len);
    bool isRunning();
    bool isDone();
    bool isTransferError();
    u16 getResult();
};
