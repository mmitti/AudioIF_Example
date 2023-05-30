#include <xscugic.h>
#include "usb_midi.h"
#include <MIDIPeripheral.h>
#include <iostream>
#undef str
#include <sstream>
using namespace std;

#define XPAR_PS7_USB_0_DEVICE_ID 0
#define XPAR_PS7_USB_0_BASEADDR 0xE0002000


XScuGic interrupt;
MIDIPeripheral midi_ph(XPAR_MIDI_BASEADDR);
USBMidi usb_midi(interrupt, midi_ph, XPAR_PS7_USB_0_DEVICE_ID, XPAR_PS7_USB_0_BASEADDR);

int main()
{
    // initialize interrupt handler
    auto IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
    if (NULL == IntcConfig) while(1){};
    auto Status = XScuGic_CfgInitialize(&interrupt, IntcConfig, IntcConfig->CpuBaseAddress);
    if (Status != XST_SUCCESS) while(1){};

    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
                                 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
                                 &interrupt);
    Xil_ExceptionDisableMask(XIL_EXCEPTION_IRQ);
	// initialize usb
	usb_midi.init();
    // enable interrupts
    Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
	
	std::cout << std::endl 
	<< "================" << std::endl 
	<< "mi USB Audio - Zynq USB - MIDI EXAMPLE VERSION" << std::endl 
	<< "(c)mmitti 2017-2023" << std::endl 
	<< "Build Date:" << __DATE__ << " " << __TIME__ << std::endl 
	<< "==Debug Console==" << std::endl 
	<< "================" << std::endl;

	while(true){
		usb_midi.update();
	}
    return 0;
}

