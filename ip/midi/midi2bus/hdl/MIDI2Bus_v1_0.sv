
`timescale 1 ns / 1 ps

	module MIDI2Bus_v1_0 #
	(
	)
	(
		input wire  aclk,
		input wire  aresetn,
		// midi_clk
		input midi_system_clock,
		input midi_clock,
		input midi_rst,
		input midi_clk_locked,
		// midi_in
		input midi_uart_in,
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
    	output wire midi_out_sysex_last
	);
	
	MidiBus midi_out_if();
	assign midi_out_midi_cmd = midi_out_if.midi_cmd;
	assign midi_out_midi_ch = midi_out_if.midi_ch;
	assign midi_out_midi_data1 = midi_out_if.midi_data1;
	assign midi_out_midi_data2 = midi_out_if.midi_data2;
	assign midi_out_if.midi_rd = midi_out_midi_rd;
	assign midi_out_midi_valid = midi_out_if.midi_valid;
	assign midi_out_if.midi_busy = midi_out_midi_busy;
	assign midi_out_sysex_data = midi_out_if.sysex_data;
	assign midi_out_if.sysex_rd = midi_out_sysex_rd;
	assign midi_out_sysex_valid = midi_out_if.sysex_valid;
	assign midi_out_if.sysex_busy = midi_out_sysex_busy;
	assign midi_out_sysex_last = midi_out_if.sysex_last;
	
	wire clk = aclk;
	wire rst = ~aresetn;
	
	// uart fifo in
	wire uart_fifo_valid;
	wire uart_fifo_rd;
	wire [7:0] uart_fifo_data;
	// sysex fifo out
	wire sysex_fifo_busy;
	wire sysex_fifo_wr;
	wire [7:0] sysex_fifo_data;
	wire sysex_fifo_last;
	// cmd fifo out
	wire cmd_fifo_busy;
	wire cmd_fifo_wr;
	wire [7:0] cmd_fifo_head;
	wire [6:0] cmd_fifo_data1;
	wire [6:0] cmd_fifo_data2;
	
	// uart rcvr fifo
	wire [7:0] fifo_data;
	wire data_fifo_wr;
	wire real_fifo_wr;
	
	UartReceiver uart(.*);
	
	fifo_bridge_data data_bridge (
		.rst(rst | midi_rst),
		.wr_clk(midi_system_clock),
		.rd_clk(clk),
		.din(fifo_data),
		.wr_en(data_fifo_wr),
		.rd_en(uart_fifo_rd),
		.dout(uart_fifo_data),
		//.full(full),
		//.empty(empty),
		.valid(uart_fifo_valid)
		//.prog_full(prog_full)
	);
	
	wire real_rd;
	wire [7:0] real_data;
	wire real_valid;
	wire ex_rd;
	wire [7:0] ex_data;
	wire ex_last;
	wire ex_valid;
	
	fifo_bridge_data real_bridge (
		.rst(rst | midi_rst),
		.wr_clk(midi_system_clock),
		.rd_clk(clk),
		.din(fifo_data),
		.wr_en(real_fifo_wr),
		.rd_en(real_rd),
		.dout(real_data),
		//.full(full),
		//.empty(empty),
		.valid(real_valid)
		//.prog_full(prog_full)
	);
	
	MidiBuilder build(.*);
	SysExMediate ex_mediate(.*);
	
	fifo_sysex fifo_sysex (
		.clk(clk),
		.srst(rst),
		.din({sysex_fifo_last,sysex_fifo_data}),
		.wr_en(sysex_fifo_wr),
		.rd_en(ex_rd),
		.dout({ex_last, ex_data}),
		//.full(full),
		//.empty(empty),
		.valid(ex_valid),
		.prog_full(sysex_fifo_busy)
	);
	
	fifo_cmd fifo_cmd (
		.clk(clk),
		.srst(rst),
		.din({cmd_fifo_head, cmd_fifo_data1, cmd_fifo_data2}),
		.wr_en(cmd_fifo_wr),
		.rd_en(midi_out_if.midi_rd),
		.dout({midi_out_if.midi_cmd, midi_out_if.midi_ch, midi_out_if.midi_data1, midi_out_if.midi_data2}),
		//.full(full),
		//.empty(empty),
		.valid(midi_out_if.midi_valid),
		.prog_full(cmd_fifo_busy)
	);

	endmodule
