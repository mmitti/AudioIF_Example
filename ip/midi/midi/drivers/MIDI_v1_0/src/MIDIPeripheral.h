#pragma once


/****************** Include Files ********************/
#include "xil_types.h"
#include "xstatus.h"

#define MIDI_S_AXI_STATUS_OFFSET 0x0
#define MIDI_S_AXI_MIDI_CH_OFFSET 0x0
#define MIDI_S_AXI_SYS_EX_OFFSET 0x4

#define MIDI_S_AXI_STATUS_SYS_EX_ARRIVED_FLAG_OFFSET 27
#define MIDI_S_AXI_STATUS_MIDI_CH_ARRIVED_FLAG_OFFSET 23
#define MIDI_S_AXI_STATUS_SYS_EX_BUSY_FLAG_OFFSET 19
#define MIDI_S_AXI_STATUS_MIDI_CH_BUSY_FLAG_OFFSET 15


#define MIDI_S_AXI_MIDI_CH_VALID_MASK (1<<28)
#define MIDI_S_AXI_SYS_EX_VALID_MASK (1<<28)
#define MIDI_S_AXI_MIDI_CH_HEAD_OFFSET (16)
#define MIDI_S_AXI_MIDI_CH_DATA1_OFFSET (8)
#define MIDI_S_AXI_MIDI_CH_DATA2_OFFSET (0)
#define MIDI_S_AXI_SYS_EX_LAST_BIT (31)
#define MIDI_S_AXI_SYS_EX_LAST_MASK (1<<31)
#define MIDI_S_AXI_SYS_EX_LEN_OFFSET (29)
#define MIDI_S_AXI_SYS_EX_LEN_MASK (0x03)
#define MIDI_S_AXI_SYS_EX_DATA1_OFFSET (16)
#define MIDI_S_AXI_SYS_EX_DATA2_OFFSET (8)
#define MIDI_S_AXI_SYS_EX_DATA3_OFFSET (0)

class MIDIPeripheral{
private:
	u32 baseAddress;
public:
	MIDIPeripheral(u32 base);

	bool arrivedMidiMessage(u8 cn);
	bool arrivedSysEx(u8 cn);
	bool busyMidiMessage(u8 cn);
	bool busySysEx(u8 cn);

	u32 readMidiMessage(u8 cn);
	u32 readSysEx(u8 cn);
	void sendMidiMessage(u8 cn, u8 head, u8 data1, u8 data2);
	void sendSysEx(u8 cn, u8 len, u8 data1, u8 data2, u8 data3, bool last);
	void sendSysEx(u8 cn, u32 len, u8* data);

	static inline bool checkValidMidiMessage(u32 p){
		return p & MIDI_S_AXI_MIDI_CH_VALID_MASK;
	}

	static inline bool checkValidSysEx(u32 p){
		return p & MIDI_S_AXI_SYS_EX_VALID_MASK;
	}

	static inline u8 getMidiMessageHead(u32 p) {
		return (p>>MIDI_S_AXI_MIDI_CH_HEAD_OFFSET) & 0xFF;
	}

	static inline u8 getMidiMessageData1(u32 p) {
		return (p>>MIDI_S_AXI_MIDI_CH_DATA1_OFFSET) & 0xFF;
	}
	static inline u8 getMidiMessageData2(u32 p) {
		return (p>>MIDI_S_AXI_MIDI_CH_DATA2_OFFSET) & 0xFF;
	}

	static inline bool getSysExLast(u32 p){
		return (p & MIDI_S_AXI_SYS_EX_LAST_BIT) == MIDI_S_AXI_SYS_EX_LAST_BIT;
	}

	static inline u8 getSysExLen(u32 p){
		return (p>>MIDI_S_AXI_SYS_EX_LEN_OFFSET) & MIDI_S_AXI_SYS_EX_LEN_MASK;
	}

	static inline u8 getSysExData1(u32 p) {
		return (p>>MIDI_S_AXI_SYS_EX_DATA1_OFFSET) & 0xFF;
	}
	static inline u8 getSysExData2(u32 p) {
		return (p>>MIDI_S_AXI_SYS_EX_DATA2_OFFSET) & 0xFF;
	}
	static inline u8 getSysExData3(u32 p) {
		return (p>>MIDI_S_AXI_SYS_EX_DATA3_OFFSET) & 0xFF;
	}
};
