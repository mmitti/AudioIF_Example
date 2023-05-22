`timescale 1ns / 1ps

module SysEx(
    input clk,
    input rst,
    RegBusIF.sysex_if reg_bus_if,
    MidiBus.SysExSender midi_out_if,
    MidiBus.SysExReceiver midi_in_if
);
    SysExMidi2Reg m2r(.*);
    SysExReg2Midi r2m(.*);

endmodule
