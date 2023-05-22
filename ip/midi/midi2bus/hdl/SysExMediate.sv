`timescale 1 ns / 1 ps

module SysExMediate
(
    input clk,
    input rst,
    // uart fifo in
    output real_rd,
	input [7:0] real_data,
	input real_valid,
	output ex_rd,
	input [7:0] ex_data,
	input ex_last,
	input ex_valid,
    // midi bus
    MidiBus.SysExSender midi_out_if
);
    typedef enum logic [1:0] {S_IDLE, S_REAL, S_EX} Status;
    Status status;
    Status next_status;
    wire [7:0] out_data;
    wire out_valid;
    wire out_last;
    wire rd = midi_out_if.sysex_rd;
    
    always_comb begin
        case(status)
            S_IDLE:
                if (real_valid) next_status <= S_REAL;
                else if (ex_valid && rd && ~ex_last) next_status <= S_EX;
                else next_status <= status;
            S_REAL:
                if (~real_valid) next_status <= S_IDLE;
                else next_status <= status;
            S_EX:
                if (ex_valid && rd && ex_last) next_status <= S_IDLE;
                else next_status <= status;
            default:
                next_status <= S_IDLE;
        endcase
    end
	
    always_ff @(posedge clk) begin
        if (rst) status <= S_IDLE;
        else status <= next_status;
    end
    
    assign out_data = (status == S_REAL) ? real_data : ex_data;
    assign out_valid = (status == S_REAL) ? real_valid : (ex_valid && next_status != S_REAL);
    assign out_last = (status == S_REAL) ? 1 : ex_last;
    
    assign midi_out_if.sysex_data = out_data;
    assign midi_out_if.sysex_valid = out_valid;
    assign midi_out_if.sysex_last = out_last;
    
    assign real_rd = rd && (status == S_REAL);
    assign ex_rd = rd && (next_status != S_REAL) && (status != S_REAL);
    
endmodule
