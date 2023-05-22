`timescale 1ns / 1ps

module BufInterface(
    input clk,
    input rst,
    RegBusIF reg_bus_if,
    MidiBus midi_out_if,
    MidiBus midi_in_if
    );
    MidiCmd midi_cmd(.*);
    SysEx sysex(.*);

endmodule
