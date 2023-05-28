#include <iostream>
#include <MIDIPeripheral.h>
#include <xparameters.h>
#include <xscugic.h>
#include <xscutimer.h>
#include <sleep.h>
#include <list>

constexpr int midi_bpqn = 120; // tick per beat(midi event resolution)
constexpr int delay_time_us_per_tick_without_tempo = 1000 * 1000 * 60 / midi_bpqn;
constexpr int midi_cn = 1; // we support cable number=1 only
constexpr int tempo = 140;
constexpr int replay_delay_bar = 8; // replay delay in bar

int tick = 0;
int beat = 0;
int bar = 0;

static void TimerHandler(void *callback)
{
	// timer interrupt handler
	XScuTimer *timer = (XScuTimer *) callback;
	if (XScuTimer_IsExpired(timer)) {
		XScuTimer_ClearInterruptStatus(timer);
	}
	tick += 1;
	if (tick >= midi_bpqn) {
		tick -= midi_bpqn;
		beat += 1;
		if (beat >= 4) {
			beat = 0;
			bar += 1;
		}
	}
}

struct Note{
	u8 note_number;
	u8 velocity;
};

struct Event {
	Note note;
	int tick = 0;
	int beat = 0;
	int bar = 0;
};

int main() {
	// initialize midi driver with MIDI's AXI base address(refer BlockDesign>Address Assign)
	MIDIPeripheral midi(XPAR_MIDI_0_S_AXI_BASEADDR);
	// initialize interuppt 
	XScuGic_Config *interrupt_config;
	XScuGic interrupt;
	interrupt_config = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
	XScuGic_CfgInitialize(&interrupt, interrupt_config,
					interrupt_config->CpuBaseAddress);
	// intiialize timer
	XScuTimer_Config *timer_config;
	XScuTimer timer;
	timer_config = XScuTimer_LookupConfig(XPAR_XSCUTIMER_0_DEVICE_ID);
	XScuTimer_CfgInitialize(&timer, timer_config,
					timer_config->BaseAddr);
	XScuTimer_EnableAutoReload(&timer);
					
	// connect timer interrupt
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				&interrupt);
	XScuGic_Connect(&interrupt, XPAR_SCUTIMER_INTR,
				(Xil_ExceptionHandler)TimerHandler,
				(void *)&timer);
		
	XScuGic_Enable(&interrupt, XPAR_SCUTIMER_INTR);
	XScuTimer_EnableInterrupt(&timer);
	Xil_ExceptionEnable();

	// set timer interbal depend on tempo
	float timer_count_per_us = 1.0 * 1000 * 1000 / (XPAR_PS7_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2); // 0.003us/count @ CPU clock=650MHz
	float tick_per_us = 1.0 * delay_time_us_per_tick_without_tempo / tempo; // 4116us/tick @ tempo=120
	int count = tick_per_us / timer_count_per_us; // timer count/tick
	XScuTimer_LoadTimer(&timer, count);
	std::cout << timer_count_per_us << "," << tick_per_us << ","<< count << std::endl;
	XScuTimer_Start(&timer);
	
	std::cout << "start" << std::endl;
	
	// MIDI event loop
	int prev_tick = 0;
	std::list<Event> midi_events_note_on;
	std::list<Event> midi_events_note_off;
	while(true) {
		std::list<Note> note_on;
		std::list<Note> note_off;
		if (tick == prev_tick) {
			usleep(10);
			continue;
		}
		prev_tick = tick;
		if (tick == 0) {
			std::cout << bar << ":" << beat << std::endl;
		}
		
		// receive midi
		while (midi.arrivedMidiMessage(midi_cn)) {
			auto msg = midi.readMidiMessage(midi_cn);
			if (MIDIPeripheral::checkValidMidiMessage(msg)) {
				u8 head = MIDIPeripheral::getMidiMessageHead(msg);
				if (head == 0x90) {
					// note on ch0
					u8 note = MIDIPeripheral::getMidiMessageData1(msg);
					u8 velocity = MIDIPeripheral::getMidiMessageData2(msg);
					if (velocity == 0) {
						// note off may implement send 0x80(note off) or 0x90(note on) with 0 velocity
						note_off.emplace_back(Note{note, 0});
					}else {
						note_on.emplace_back(Note{note, velocity});
					}
				}
				if (head == 0x80) {
					// note off ch0
					u8 note = MIDIPeripheral::getMidiMessageData1(msg);
					note_off.emplace_back(Note{note, 0});
				}
			}
		}
		while(midi.arrivedSysEx(1)){
			midi.readSysEx(1);
		}
		
		// relay midi and record midi event
		for(auto& note : note_on) {
			midi.sendMidiMessage(midi_cn, 0x90, note.note_number, note.velocity);
			std::cout << "NOTE ON" << (int)note.note_number << ":" << (int)note.velocity << std::endl;
			midi_events_note_on.emplace_back(Event{note, tick, beat, bar});
		}
		for(auto& note : note_off) {
			midi.sendMidiMessage(midi_cn, 0x80, note.note_number, 0);
			std::cout << "NOTE OFF" << (int)note.note_number << std::endl;
			midi_events_note_off.emplace_back(Event{note, tick, beat, bar});
		}

		// playback midi event with delay
		for(auto itr = midi_events_note_on.begin(); itr != midi_events_note_on.end();) {
			if (itr->bar + replay_delay_bar > bar || (itr->bar + replay_delay_bar == bar && (itr->beat > beat || (itr->beat == beat && itr->tick > tick)))) {
				itr++;
				continue;	
			}
			midi.sendMidiMessage(midi_cn, 0x90, itr->note.note_number, itr->note.velocity);
			std::cout << "NOTE ON REPLAY" << (int)itr->note.note_number << ":" << (int)itr->note.velocity << std::endl;
			itr = midi_events_note_on.erase(itr);
		}
		for(auto itr = midi_events_note_off.begin(); itr != midi_events_note_off.end();) {
			if (itr->bar + replay_delay_bar > bar || (itr->bar + replay_delay_bar == bar && (itr->beat > beat || (itr->beat == beat && itr->tick > tick)))) {
				itr++;
				continue;	
			}
			midi.sendMidiMessage(midi_cn, 0x80, itr->note.note_number, 0);
			std::cout << "NOTE OFF REPLAY" << (int)itr->note.note_number << std::endl;
			itr = midi_events_note_off.erase(itr);
		}
	}

	return 0;
}
