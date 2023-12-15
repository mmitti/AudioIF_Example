

/***************************** Include Files *******************************/
#include "MIDIPeripheral.h"
#include "xmidi.h"
#include <xil_io.h>
/************************** Function Definitions ***************************/
#define MIDI_WriteReg(BaseAddress, RegOffset, Data)  Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define MIDI_ReadReg(BaseAddress, RegOffset)   Xil_In32((BaseAddress) + (RegOffset))

extern XMIDI_Config XMIDI_ConfigTable[];

MIDIPeripheral::MIDIPeripheral(u32 base)
{
	for (int i = 0; XMIDI_ConfigTable[i].Name != NULL; i++) {
        if ((XMIDI_ConfigTable[i].BaseAddress == base) || !base) {
            this->baseAddress = XMIDI_ConfigTable[i].BaseAddress;
           	break;
       }
    }
}

bool MIDIPeripheral::arrivedMidiMessage(u8 cn)
{
	auto flags = MIDI_ReadReg(baseAddress, MIDI_S_AXI_STATUS_OFFSET);
	return (flags >> (MIDI_S_AXI_STATUS_MIDI_CH_ARRIVED_FLAG_OFFSET+cn)) & 0x01;
}

bool MIDIPeripheral::arrivedSysEx(u8 cn)
{
	auto flags = MIDI_ReadReg(baseAddress, MIDI_S_AXI_STATUS_OFFSET);
	return (flags >> (MIDI_S_AXI_STATUS_SYS_EX_ARRIVED_FLAG_OFFSET+cn)) & 0x01;
}

bool MIDIPeripheral::busyMidiMessage(u8 cn)
{
	auto flags = MIDI_ReadReg(baseAddress, MIDI_S_AXI_STATUS_OFFSET);
		return (flags >> (MIDI_S_AXI_STATUS_MIDI_CH_BUSY_FLAG_OFFSET+cn)) & 0x01;
}

bool MIDIPeripheral::busySysEx(u8 cn)
{
	auto flags = MIDI_ReadReg(baseAddress, MIDI_S_AXI_STATUS_OFFSET);
		return (flags >> (MIDI_S_AXI_STATUS_SYS_EX_BUSY_FLAG_OFFSET+cn)) & 0x01;
}

u32 MIDIPeripheral::readMidiMessage(u8 cn)
{
	return MIDI_ReadReg(baseAddress, MIDI_S_AXI_MIDI_CH_OFFSET | (cn<<4));
}

u32 MIDIPeripheral::readSysEx(u8 cn)
{
	return MIDI_ReadReg(baseAddress, MIDI_S_AXI_SYS_EX_OFFSET | (cn<<4));
}

void MIDIPeripheral::sendMidiMessage(u8 cn, u8 head, u8 data1, u8 data2)
{
	MIDI_WriteReg(baseAddress, MIDI_S_AXI_MIDI_CH_OFFSET | (cn<<4),
			(head << MIDI_S_AXI_MIDI_CH_HEAD_OFFSET) |
			(data1 << MIDI_S_AXI_MIDI_CH_DATA1_OFFSET) |
			(data2 << MIDI_S_AXI_MIDI_CH_DATA2_OFFSET)
	);
}

void MIDIPeripheral::sendSysEx(u8 cn, u8 len, u8 data1, u8 data2, u8 data3, bool last)
{
	MIDI_WriteReg(baseAddress, MIDI_S_AXI_SYS_EX_OFFSET | (cn<<4),
				(last ? MIDI_S_AXI_SYS_EX_LAST_MASK : 0) |
				((len & MIDI_S_AXI_SYS_EX_LEN_MASK) << MIDI_S_AXI_SYS_EX_LEN_OFFSET) |
				(data1 << MIDI_S_AXI_SYS_EX_DATA1_OFFSET) |
				(data2 << MIDI_S_AXI_SYS_EX_DATA2_OFFSET) |
				(data3 << MIDI_S_AXI_SYS_EX_DATA3_OFFSET)
		);
}

void MIDIPeripheral::sendSysEx(u8 cn, u32 len, u8* data)
{
	u32 i = 0;
	for(;i+3<len; i+=3) sendSysEx(cn, 3, data[i], data[i+1], data[i+2], false);
	switch(len - i){
	case 1:
		sendSysEx(cn, 1, data[i], 0x0, 0x0, true);
		break;
	case 2:
		sendSysEx(cn, 2, data[i], data[i+1], 0x0, true);
		break;
	case 3:
		sendSysEx(cn, 3, data[i], data[i+1], data[i+2], true);
		break;
	}

}
