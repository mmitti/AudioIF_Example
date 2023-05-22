`timescale 1ns / 1ps

module SysExReg2Midi(
    input clk,
    input rst,
    RegBusIF.sysex_if reg_bus_if,
    MidiBus.SysExSender midi_out_if
);
wire reg_fifo_rd;
wire midi_bus_fifo_full;

// =====================================
// reg bus fifo
// wdataとlen, lastをfifoへ
// =====================================
wire reg_bus_fifo_full;
wire [26 : 0] reg_bus_fifo_out;
wire reg_bus_fifo_valid;
midi_sysex_fifo reg_bus_fifo (
  .clk(clk),              // input wire clk
  .srst(rst),            // input wire srst
  .din({reg_bus_if.sysex_wlen, reg_bus_if.sysex_wlast, reg_bus_if.sysex_wdata}),              // input wire [26 : 0] din
  .wr_en(reg_bus_if.sysex_wr),          // input wire wr_en
  .rd_en(reg_fifo_rd),          // input wire rd_en
  .dout(reg_bus_fifo_out),            // output wire [26 : 0] dout
//  .full(full),            // output wire full
//  .empty(empty),          // output wire empty
  .valid(reg_bus_fifo_valid),          // output wire valid
  .prog_full(reg_bus_fifo_full)  // output wire prog_full
);
assign reg_bus_if.sysex_wbusy = reg_bus_fifo_full || midi_bus_fifo_full;
wire [23:0] reg_bus_data = reg_bus_fifo_out[23:0];
wire [1:0] reg_bus_len = reg_bus_fifo_out[26:25];
wire reg_bus_last = reg_bus_fifo_out[24];
// =====================================
// conv
// fifoから1つ取り出し3クロックかけて分解する
// =====================================
typedef enum logic [1:0] {S_IDLE, S_DATA0, S_DATA1, S_DATA2} Status;
Status status;
Status next_status;

wire can_read = reg_bus_fifo_valid && ~midi_bus_fifo_full;
wire midi_fifo_wr = can_read && next_status != S_IDLE;

wire last_l1 = (reg_bus_len == 2'd1) && next_status == S_DATA0;
wire last_l2 = (reg_bus_len == 2'd2) && next_status == S_DATA1;
wire last_l3 = (reg_bus_len == 2'd3) && next_status == S_DATA2;
wire midi_fifo_last = (last_l1 | last_l2 | last_l3) && reg_bus_last;
wire empty_cmd = reg_bus_len == 2'd0;
assign reg_fifo_rd = ((status != S_IDLE ) || empty_cmd) && (next_status == S_IDLE);

function [7:0] midi_fifo_data_select;
    input [23:0] data;
    input Status s;
    case(s)
        S_DATA0: midi_fifo_data_select = data[23:16];
        S_DATA1: midi_fifo_data_select = data[15:8];
        default: midi_fifo_data_select = data[7:0];
    endcase
endfunction
wire [7:0] midi_fifo_in = midi_fifo_data_select(reg_bus_data, next_status);

always_comb begin
    case(status)
        S_IDLE: begin
            if (can_read) begin
                if (empty_cmd) next_status <= S_IDLE;
                else next_status <= S_DATA0;
            end
            else next_status <= S_IDLE;
        end
        S_DATA0: begin
            if (reg_bus_len == 2'd1) next_status <= S_IDLE;
            else if(can_read) next_status <= S_DATA1;
            else next_status <= S_DATA0;
        end
        S_DATA1: begin
            if (reg_bus_len == 2'd2) next_status <= S_IDLE;
            else if(can_read) next_status <= S_DATA2;
            else next_status <= S_DATA1;
        end
        S_DATA2: begin
            next_status <= S_IDLE;
        end
        default: begin
            next_status <= S_IDLE;
        end
    endcase
end

always_ff @(posedge clk) begin
    if (rst) status <= S_IDLE;
    else status <= next_status;
end
// =====================================
// sysex fifo
// midi_outとそのまま接続
// =====================================
midi_sysex_midi_bus_fifo midi_bus_fifo (
  .clk(clk),              // input wire clk
  .srst(rst),            // input wire srst
  .din({midi_fifo_last, midi_fifo_in}),              // input wire [8 : 0] din
  .wr_en(midi_fifo_wr),          // input wire wr_en
  .rd_en(midi_out_if.sysex_rd),          // input wire rd_en
  .dout({midi_out_if.sysex_last, midi_out_if.sysex_data}),            // output wire [8 : 0] dout
//  .full(full),            // output wire full
//  .empty(empty),          // output wire empty
  .valid(midi_out_if.sysex_valid),          // output wire valid
  .prog_full(midi_bus_fifo_full)  // output wire prog_full
);

endmodule
