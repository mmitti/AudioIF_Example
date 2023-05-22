
`timescale 1 ns / 1 ps

	module Bus2MIDI_v1_0 #
	(
	)
	(
		input aclk,
		input aresetn,
		// midi_clk
		input midi_system_clock,
		input midi_clock,
		input keep_alive,
		input midi_rst,
		input midi_clk_locked,
		// midi
		input [3:0] midi_midi_cmd,
        input [3:0] midi_midi_ch,
        input [6:0] midi_midi_data1,
        input [6:0] midi_midi_data2,
        output midi_midi_rd,
        input midi_midi_valid,
        output midi_midi_busy,
        input [7:0] midi_sysex_data,
        output midi_sysex_rd,
        input midi_sysex_valid,
        output midi_sysex_busy,
        input midi_sysex_last,
		// midi out
		output midi_uart_out
	);
	
	MidiBus midi_if();
	assign midi_if.midi_cmd = midi_midi_cmd;
	assign midi_if.midi_ch = midi_midi_ch;
	assign midi_if.midi_data1 = midi_midi_data1;
	assign midi_if.midi_data2 = midi_midi_data2;
	assign midi_midi_rd = midi_if.midi_rd;
	assign midi_if.midi_valid = midi_midi_valid;
	assign midi_midi_busy = midi_if.midi_busy;
	assign midi_if.sysex_data = midi_sysex_data;
	assign midi_sysex_rd = midi_if.sysex_rd;
	assign midi_if.sysex_valid = midi_sysex_valid;
	assign midi_sysex_busy = midi_if.sysex_busy;
	assign midi_if.sysex_last = midi_sysex_last;
	
	wire rst = ~aresetn | midi_rst | ~midi_clk_locked;
	
	wire [7:0] fifo_in;
	wire [7:0] fifo_out;
	
	wire fifo_wr_rst_busy;
	wire fifo_rd_rst_busy;
	
	wire fifo_busy;
	wire fifo_almost_empty;
	wire fifo_valid;
	wire fifo_wr;
	wire fifo_rd;
	
	bus2midi_data_fifo bus2midi_data_fifo (
		.rst(rst),
		.wr_clk(aclk),
		.rd_clk(midi_system_clock),
		.din(fifo_in),
		.wr_en(fifo_wr),
		.rd_en(fifo_rd),
		.dout(fifo_out),
		.almost_empty(fifo_almost_empty),
		.valid(fifo_valid),
		.prog_full(fifo_busy),
		.wr_rst_busy(fifo_wr_rst_busy),
		.rd_rst_busy(fifo_rd_rst_busy)
	);
	
	Selector sel(
		.clk(aclk),
		.rst(rst),
        // midi bus
		.midi_in(midi_if),
        // fifo
	    .fifo_wr_rst_busy(fifo_wr_rst_busy),
        .fifo_in(fifo_in),
        .fifo_busy(fifo_busy),
        .fifo_wr(fifo_wr)
	);
	
	wire uart_done;
	wire uart_idle;
	wire uart_valid;
	wire [7:0] uart_data;
	
	UartFetch fetch
	(
		.*
	);
	Uart uart(
		.*
	);


	endmodule
