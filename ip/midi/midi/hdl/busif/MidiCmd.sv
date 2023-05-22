`timescale 1ns / 1ps

module MidiCmd(
    input clk,
    input rst,
    RegBusIF.cmd_if reg_bus_if,
    MidiBus.MidiSender midi_out_if,
    MidiBus.MidiReceiver midi_in_if
);

wire prog_full;
wire [21:0] dout;
// reg_bus->midi_out
assign reg_bus_if.cmd_wbusy = prog_full | midi_out_if.midi_busy;
midi_cmd_fifo midi_cmd_fifo (
  .clk(clk),              // input wire clk
  .srst(rst),            // input wire srst
  .din({reg_bus_if.cmd_wdata[23:16], reg_bus_if.cmd_wdata[14:8], reg_bus_if.cmd_wdata[6:0]}),              // input wire [23 : 0] din
  .wr_en(reg_bus_if.cmd_wr),          // input wire wr_en
  .rd_en(midi_out_if.midi_rd),          // input wire rd_en
  .dout(dout),            // output wire [23 : 0] dout
//  .full(full),            // output wire full
//  .empty(empty),          // output wire empty
  .valid(midi_out_if.midi_valid),          // output wire valid
  .prog_full(prog_full)  // output wire prog_full
);
assign midi_out_if.midi_cmd = dout[21:18];
assign midi_out_if.midi_ch = dout[17:14];
assign midi_out_if.midi_data1 = dout[13:7];
assign midi_out_if.midi_data2 = dout[6:0];
//midi_in->reg_bus
assign reg_bus_if.cmd_rdata = {
    midi_in_if.midi_cmd,
    midi_in_if.midi_ch,
    1'd0, midi_in_if.midi_data1,
    1'd0, midi_in_if.midi_data2};
assign midi_in_if.midi_rd = reg_bus_if.cmd_rd;
assign reg_bus_if.cmd_rvalid = midi_in_if.midi_valid;
assign midi_in_if.midi_busy = 0;

endmodule
