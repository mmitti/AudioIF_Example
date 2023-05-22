
`timescale 1 ns / 1 ps

	module Uart
	(
		// midi_clk
		input midi_system_clock,
		input midi_clock,
		input midi_rst,
		input midi_clk_locked,
        // uart
        output uart_done,
        output uart_idle,
        input uart_valid,
        input [7:0] uart_data,
        // midi out
		output midi_uart_out
	);
    wire clk = midi_system_clock;
    wire rst = midi_rst | ~midi_clk_locked;
    
    // =============
    // posedge 
    // =============
    reg uart_update_tmp;
    wire uart_update;
    always_ff @(posedge clk) begin
        if (rst) uart_update_tmp <= 0;
        else uart_update_tmp <= midi_clock;
    end
    assign uart_update = ~uart_update_tmp & midi_clock;
    // =============
    // common
    // =============
    reg [3:0] cursol;
    wire uart_data_current;
    // =============
    // status
    // =============
    typedef enum logic [1:0] {S_IDLE, S_START, S_DATA, S_STOP} Status;
    Status status;
    Status next_status;
    
    always_comb begin
        case(status)
            S_IDLE:
                if (uart_update & uart_valid) next_status <= S_START;
                else next_status <= status;
            S_START:
                if (uart_update) next_status <= S_DATA;
                else next_status <= status;
            S_DATA:
                if (uart_update && cursol >= 4'd7) next_status <= S_STOP;
                else next_status <= status;
            S_STOP:
                if (uart_update) next_status <= S_IDLE;
                else next_status <= status;
            default:
                next_status <= S_IDLE;
        endcase
    end
	
    always_ff @(posedge clk) begin
        if (rst) status <= S_IDLE;
        else status <= next_status;
    end
    
    
    // =============
    // common
    // =============
    
    always_ff @(posedge clk) begin
        if (rst) cursol <= 4'd0;
        else if (status == S_START) cursol <= 4'd0;
        else if (status == S_DATA && uart_update) cursol <= cursol + 4'd1;
    end
    
    assign uart_data_current = uart_data[cursol];
    assign midi_uart_out =  (status == S_START) ? 0 : 
                            (status == S_DATA) ? uart_data_current :
                            1;
    assign uart_idle = (status == S_IDLE);
    assign uart_done = (status == S_STOP) && uart_update;
	endmodule
