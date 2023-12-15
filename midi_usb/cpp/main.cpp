#include <xscugic.h>
#include <xparameters.h>
#include "usb_midi.h"
#include <MIDIPeripheral.h>
#include <iostream>
#undef str
#include <sstream>
using namespace std;

XScuGic interrupt;
MIDIPeripheral midi_ph(XPAR_MIDI_0_BASEADDR);
USBMidi usb_midi(interrupt, midi_ph, XPAR_USB0_BASEADDR);

int main()
{
    // initialize interrupt handler
    auto IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_DIST_BASEADDR);
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
	<< "==============================================" << std::endl 
	<< "mi USB Audio - Zynq USB - MIDI EXAMPLE VERSION" << std::endl 
	<< "(c)mmitti 2017-2023" << std::endl 
	<< "Build Date:" << __DATE__ << " " << __TIME__ << std::endl 
	<< "==============================================" << std::endl;

	while(true){
        // transfer midi 
        // + PL to PS( read from PL and send to USB buffer )
        // + PS to PL( read ring buffer and write to PL )
		usb_midi.update();
	}
    return 0;
}

