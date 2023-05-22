`timescale 1 ns / 1 ps
	module Selector
	(
		input clk,
		input rst,
        // midi bus
		MidiBus.Receiver midi_in,
        // fifo
	    input fifo_wr_rst_busy,
        output [7:0] fifo_in,
        input fifo_busy,
        output fifo_wr
	);
    // ==================
    // common
    // ==================
    reg [1:0] cmd_max_len;
    reg [1:0] cmd_len;
    
    // ==================
    // status
    // ==================
    wire fifo_ready = ~fifo_wr_rst_busy & ~fifo_busy;
    
    typedef enum logic [1:0] {S_IDLE, S_EX, S_CMD} Status;
    Status status;
    Status next_status;
    
    wire sysex_last = fifo_ready & midi_in.sysex_last & midi_in.sysex_valid;

	always_comb begin
        case(status)
            S_IDLE:
                if (midi_in.sysex_valid) next_status <= S_EX;
                else if(midi_in.midi_valid) next_status <= S_CMD;
                else next_status <= S_IDLE;
            S_EX:
                if (sysex_last) begin
                    if (midi_in.midi_valid) next_status <= S_CMD;
                    else next_status <= S_IDLE;
                end else next_status <= S_EX;
            S_CMD:
                if ( (cmd_len == cmd_max_len) ) next_status <= S_IDLE;
                else next_status <= S_CMD;
            default:
                next_status <= S_IDLE;
        endcase
    end
    
    always_ff @(posedge clk) begin
        if (rst) status <= S_IDLE;
        else if (fifo_wr_rst_busy) status <= S_IDLE;
        else status <= next_status;
    end
    
// ==================
// ex
// ==================
    wire ex_rd; // 1データ読み込み
    wire ex_fifo_wr;
    wire [7:0] ex_data;
    assign ex_data = midi_in.sysex_data;
    
    assign midi_in.sysex_rd = ex_rd;
    assign ex_fifo_wr = ex_rd;
    assign ex_rd = (status == S_EX) & fifo_ready & midi_in.sysex_valid;
    assign midi_in.sysex_busy = fifo_busy;
    
// ==================
// cmd
// ==================
    wire cmd_rd; // 1データ読み込み
    wire cmd_fifo_wr;
    wire [7:0] cmd_data;
    reg [7:0] prev_cmd_head;
    wire [7:0] cmd_head = {midi_in.midi_cmd, midi_in.midi_ch};
    always_comb begin
        case (midi_in.midi_cmd)
            4'h8: cmd_max_len <= 2'd3;
            4'h9: cmd_max_len <= 2'd3;
            4'hA: cmd_max_len <= 2'd3;
            4'hB: cmd_max_len <= 2'd3;
            4'hC: cmd_max_len <= 2'd2;
            4'hD: cmd_max_len <= 2'd2;
            4'hE: cmd_max_len <= 2'd3;
            // TODO SYS COM対応
            default: cmd_max_len <= 2'd0;
        endcase
    end
    
    always_ff @(posedge clk) begin
        if (rst) cmd_len <= 2'd0;
        else if (status != S_CMD & next_status == S_CMD) cmd_len <= (cmd_head == prev_cmd_head) ? 2'd1 : 2'd0;
        else if (cmd_rd) cmd_len <= cmd_len + 2'd1;
    end
    
    assign cmd_data = cmd_len == 2'd2 ? {1'b0, midi_in.midi_data2} : 
                      cmd_len == 2'd1 ? {1'b0, midi_in.midi_data1} :
                      cmd_head;
    assign cmd_fifo_wr = cmd_rd;
    assign cmd_rd = (cmd_len < cmd_max_len) & (status == S_CMD) & fifo_ready & midi_in.midi_valid;
    assign midi_in.midi_busy = fifo_busy;
    assign midi_in.midi_rd = (status == S_CMD) & (next_status == S_IDLE);
    
    always_ff @(posedge clk) begin
        if (rst) prev_cmd_head <= 8'd0;
        else if (status == S_EX)  prev_cmd_head <= 8'd0;
        else if ((status == S_CMD) & (next_status == S_IDLE)) prev_cmd_head <= cmd_head;
    end
    
// ==================
// common
// ==================
    assign fifo_wr = (status == S_EX) ? ex_fifo_wr : cmd_fifo_wr;
    assign fifo_in = (status == S_EX) ? ex_data : cmd_data;
    
	endmodule
