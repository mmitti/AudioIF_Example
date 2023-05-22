module transpose_midi(

    input wire  aclk,
    input wire  aresetn,
    //midi out
    output wire [3:0] midi_out_midi_cmd,
    output wire [3:0] midi_out_midi_ch,
    output wire [6:0] midi_out_midi_data1,
    output wire [6:0] midi_out_midi_data2,
    input wire midi_out_midi_rd,
    output wire midi_out_midi_valid,
    input wire midi_out_midi_busy,
    output wire [7:0] midi_out_sysex_data,
    input wire midi_out_sysex_rd,
    output wire midi_out_sysex_valid,
    input wire midi_out_sysex_busy,
    output wire midi_out_sysex_last,
    //midi in
    input wire [3:0] midi_in_midi_cmd,
    input wire [3:0] midi_in_midi_ch,
    input wire [6:0] midi_in_midi_data1,
    input wire [6:0] midi_in_midi_data2,
    output wire midi_in_midi_rd,
    input wire midi_in_midi_valid,
    output wire midi_in_midi_busy,
    input wire [7:0] midi_in_sysex_data,
    output wire midi_in_sysex_rd,
    input wire midi_in_sysex_valid,
    output wire midi_in_sysex_busy,
    input wire midi_in_sysex_last
);

assign midi_out_midi_cmd = midi_in_midi_cmd;
assign midi_out_midi_ch = midi_in_midi_ch;
assign midi_out_midi_data1 = midi_in_midi_cmd == 4'h9 ? midi_in_midi_data1 + 1 : midi_in_midi_data1; // note shift
assign midi_out_midi_data2 = midi_in_midi_data2;

assign midi_out_midi_valid = midi_in_midi_valid;
assign midi_out_sysex_data = midi_in_sysex_data;
assign midi_out_sysex_valid = midi_in_sysex_valid;
assign midi_out_sysex_last = midi_in_sysex_last;

assign midi_in_midi_rd = midi_out_midi_rd;
assign midi_in_midi_busy = midi_out_midi_busy;
assign midi_in_sysex_rd = midi_out_sysex_rd;
assign midi_in_sysex_busy = midi_out_sysex_busy;

endmodule
