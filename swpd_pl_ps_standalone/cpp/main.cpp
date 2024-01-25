#include <iostream>
#include <SWPDPeripheral.h>
#include <xparameters.h>
#include <sleep.h>
#include <xil_types.h>

constexpr u8 CMD_ID_GET_VERSION = 0b10000000;
constexpr u8 CMD_ID_GET_SWPD_STATUS = 0b10000001;
constexpr u8 CMD_ID_GET_CFG_SWPD_ENABLE = 0b10000010;
constexpr u8 CMD_ID_GET_CFG_SWPD_POLARITY = 0b10000011;
constexpr u8 CMD_ID_GET_CFG_SWPD_DETECTION = 0b10000100;
constexpr u8 CMD_ID_GET_SWPD_VALUE = 0b10010000;

constexpr u8 REPLY_LEN_GET_VERSION = 1;
constexpr u8 REPLY_LEN_GET_SWPD_STATUS = 1;
constexpr u8 REPLY_LEN_GET_CFG_SWPD_ENABLE = 1;
constexpr u8 REPLY_LEN_GET_CFG_SWPD_POLARITY = 1;
constexpr u8 REPLY_LEN_GET_CFG_SWPD_DETECTION = 1;
constexpr u8 REPLY_LEN_GET_SWPD_VALUE = 2;

int main() {
	// initialize swpd IP driver with AXI base address(refer BlockDesign>Address Assign)
    SWPDPeripheral ph(XPAR_SWPDCONTROL_0_BASEADDR);
    ph.init();

    auto do_command = [&ph](u8 cmd, u8 len) -> u16{
        ph.request(cmd, len);
        while(ph.isRunning()) {
            usleep(1);
        }
        if (ph.isDone() && !ph.isTransferError())
            return ph.getResult();
        return 0;
    };

    // read current status
    std::cout << "VERSION: " << std::hex << do_command(CMD_ID_GET_VERSION, REPLY_LEN_GET_VERSION) << std::endl;
    std::cout << "ENABLE: " << std::hex << do_command(CMD_ID_GET_CFG_SWPD_ENABLE, REPLY_LEN_GET_CFG_SWPD_ENABLE) << std::endl;
    std::cout << "POLARITY: " << std::hex << do_command(CMD_ID_GET_CFG_SWPD_POLARITY, REPLY_LEN_GET_CFG_SWPD_POLARITY) << std::endl;
    std::cout << "DETECTION: " << std::hex << do_command(CMD_ID_GET_CFG_SWPD_DETECTION, REPLY_LEN_GET_CFG_SWPD_DETECTION) << std::endl;

    // read value loop
	while(true) {
        sleep(1);
        std::cout << "======" << std::endl;
        std::cout << "STATUS:" << std::hex << do_command(CMD_ID_GET_SWPD_STATUS, REPLY_LEN_GET_SWPD_STATUS) << std::endl;
        std::cout << "VALUE A:" << std::hex << do_command(CMD_ID_GET_SWPD_VALUE | 0, REPLY_LEN_GET_SWPD_VALUE) << std::endl;
        std::cout << "VALUE B:" << std::hex << do_command(CMD_ID_GET_SWPD_VALUE | 1, REPLY_LEN_GET_SWPD_VALUE) << std::endl;
	}

	return 0;
}
