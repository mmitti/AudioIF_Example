
`timescale 1 ns / 1 ps

	module UartFetch 
	(
		// midi_clk
		input midi_system_clock,
		input keep_alive,
		input midi_rst,
		input midi_clk_locked,
        // fifo
        input [7:0] fifo_out,
        input fifo_rd_rst_busy,
        input fifo_almost_empty,
        input fifo_valid,
        output fifo_rd,
        // to uart
        input uart_done,
        input uart_idle,
        output uart_valid,
        output [7:0] uart_data
	);
    wire clk = midi_system_clock;
    wire rst = midi_rst;
    // =============
    // status
    // =============
    typedef enum logic [1:0] {S_IDLE, S_KEEP_ARRIVE, S_DATA} Status;
    Status status;
    Status next_status;
    
    always_comb begin
        case(status)
            S_IDLE:
                if (fifo_valid) next_status <= S_DATA;
                else if(keep_alive) next_status <= S_KEEP_ARRIVE;
                else next_status <= S_IDLE;
            S_KEEP_ARRIVE:
                if (uart_done) next_status <= S_IDLE;
                else next_status <= S_KEEP_ARRIVE;
            S_DATA:
                if (uart_done && fifo_almost_empty) next_status <= S_IDLE;
                else next_status <= S_DATA;
            default:
                next_status <= S_IDLE;
        endcase
    end
	
    always_ff @(posedge clk) begin
        if (rst) status <= S_IDLE;
        else if (fifo_rd_rst_busy | ~midi_clk_locked) status <= S_IDLE;
        else status <= next_status;
    end
    // =============
    assign uart_data = (status == S_KEEP_ARRIVE) ? 8'hFE : fifo_out;
    assign uart_valid = (status == S_KEEP_ARRIVE) ? 1 : (status == S_DATA) ? fifo_valid : 0;
    assign fifo_rd = (status == S_DATA) & uart_done;
	endmodule
