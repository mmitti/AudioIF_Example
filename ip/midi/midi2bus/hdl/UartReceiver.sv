
`timescale 1 ns / 1 ps

	module UartReceiver
	(
		// midi_clk
		input midi_system_clock,
		input midi_clock,
		input midi_rst,
		input midi_clk_locked,
        // midi in
		input midi_uart_in,
        // fifo
        output reg [7:0] fifo_data,
        output data_fifo_wr,
        output real_fifo_wr
	);
    wire clk = midi_system_clock;
    wire rst = midi_rst | ~midi_clk_locked;
    
// =============
// negedge 
// =============
    reg [1:0] uart_begin_tmp;
    wire uart_begin;
    always_ff @(posedge clk) begin
        if (rst) uart_begin_tmp <= 2'd0;
        else uart_begin_tmp <= {uart_begin_tmp[0], midi_uart_in};
    end
    assign uart_begin = uart_begin_tmp[1] & ~uart_begin_tmp[0];
    
// =============
// common
// =============
    reg [2:0] cursol;
    reg [3:0] len;
    wire fifo_wr;
    
// =============
// status
// =============
    typedef enum logic [1:0] {S_IDLE, S_START, S_DATA, S_STOP} Status;
    Status status;
    Status next_status;
    
    always_comb begin
        case(status)
            S_IDLE:
                if (uart_begin) next_status <= S_START;
                else next_status <= status;
            S_START:
                if (cursol == 3'd3 && ~midi_uart_in) next_status <= S_DATA;
                else if (midi_uart_in && (cursol == 3'd1 || cursol == 3'd2 || cursol == 3'd3))
                     next_status <= S_IDLE;
                else next_status <= status;
            S_DATA:
                if (len >= 4'd8 && cursol >= 3'd7) next_status <= S_STOP;
                else next_status <= status;
            S_STOP:
                next_status <= S_IDLE;
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
        if (rst) cursol <= 3'd0;
        else if(status == S_IDLE && uart_begin) cursol <= 3'd0;
        else if(status == S_START && next_status == S_DATA) cursol <= 3'd1;
        else cursol <= cursol + 3'd1;
    end
    
    always_ff @(posedge clk) begin
        if (rst) len <= 4'd0;
        else if(status == S_START && next_status == S_DATA) len <= 4'd0;
        else if(cursol == 3'd7) len <= len + 4'd1;
    end
    
    always_ff @(posedge clk) begin
        if (rst) fifo_data <= 8'd0;
        else if(len < 4'd8 && status == S_DATA && cursol == 3'd7) fifo_data[len] = midi_uart_in;
    end
    
    assign fifo_wr = status == S_STOP && midi_uart_in;
    
    wire sys_real_msg = fifo_data[7:3] == 5'h1F;//F8~FF
    assign data_fifo_wr = fifo_wr && ~sys_real_msg;
    assign real_fifo_wr = fifo_wr && sys_real_msg;

	endmodule
