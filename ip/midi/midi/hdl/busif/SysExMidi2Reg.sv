`timescale 1ns / 1ps

module SysExMidi2Reg(
    input clk,
    input rst,
    RegBusIF.sysex_if reg_bus_if,
    MidiBus.SysExReceiver midi_in_if
);
/*
     input sysex_data, sysex_valid, sysex_last,
    output sysex_rd, sysex_busy
*/
wire reg_bus_fifo_full;
wire [23:0] reg_bus_fifo_data;
wire [1:0] reg_bus_fifo_len;
wire reg_bus_fifo_last;
wire reg_bus_fifo_wr;

// =====================================
// ctlr
// midi_inがvalidなら3bitレジスタにつめる
// lastが来たら強制的にfifoへ、3bitたまったときもfifoへ
// wdata, len, lastを詰める
// =====================================
reg [7:0] data[0:2];
reg [1:0] len;
typedef enum logic [1:0] {S_IDLE, S_RD, S_LAST, S_COMMIT} Status;
Status status;
Status next_status;


wire midi_rd = midi_in_if.sysex_valid && ~reg_bus_fifo_full && ((status == S_IDLE) || (status == S_RD));

always_ff @(posedge clk) begin
    if (rst) len <= 2'd0;
    else if(next_status == S_IDLE) len <= 2'd0;
    else if (midi_rd) len <= len + 2'd1;
end

always_ff @(posedge clk) begin
    if (rst) begin
        data[0] <= 8'd0;
        data[1] <= 8'd0;
        data[2] <= 8'd0;
    end
    else if (midi_rd) begin
        if (len == 0) begin
            data[0] <= midi_in_if.sysex_data;
            data[1] <= 8'd0;
            data[2] <= 8'd0;
        end else data[len] <= midi_in_if.sysex_data;
    end
end

always_comb begin
    case(status)
        S_IDLE: begin
            if (midi_rd) begin
                if (midi_in_if.sysex_last) next_status <= S_LAST;
                else next_status <= S_RD;
            end else next_status <= S_IDLE;
        end
        S_RD: begin
            if (midi_rd) begin
                if (midi_in_if.sysex_last) next_status <= S_LAST;
                else if(len >= 2'd2) next_status <= S_COMMIT;
                else next_status <= S_RD;
            end else next_status <= S_RD;
        end
        S_LAST: begin
            next_status <= S_IDLE;
        end
        S_COMMIT: begin
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

assign reg_bus_fifo_last = (status == S_LAST);
assign reg_bus_fifo_wr = (status == S_LAST) || (status == S_COMMIT);
////////////////////////
assign reg_bus_fifo_len = len;
assign reg_bus_fifo_data = {data[0], data[1], data[2]};
assign midi_in_if.sysex_busy = reg_bus_fifo_full;
assign midi_in_if.sysex_rd = midi_rd;
// =====================================
// regbus fifo
// =====================================
midi_sysex_fifo reg_bus_fifo (
  .clk(clk),              // input wire clk
  .srst(rst),            // input wire srst
  .din({reg_bus_fifo_len, reg_bus_fifo_last, reg_bus_fifo_data}),              // input wire [26 : 0] din
  .wr_en(reg_bus_fifo_wr),          // input wire wr_en
  .rd_en(reg_bus_if.sysex_rd),          // input wire rd_en
  .dout({reg_bus_if.sysex_rlen, reg_bus_if.sysex_rlast, reg_bus_if.sysex_rdata}),            // output wire [26 : 0] dout
//  .full(full),            // output wire full
//  .empty(empty),          // output wire empty
  .valid(reg_bus_if.sysex_rvalid),          // output wire valid
  .prog_full(reg_bus_fifo_full)  // output wire prog_full
);
endmodule
