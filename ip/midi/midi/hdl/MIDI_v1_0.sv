
`timescale 1 ns / 1 ps

	module MIDI_v1_0 #
	(
		// Users to add parameters here

		// User parameters ends
		// Do not modify the parameters beyond this line


		// Parameters of Axi Slave Bus Interface S_AXI
		parameter integer C_S_AXI_DATA_WIDTH	= 32,
		parameter integer C_S_AXI_ADDR_WIDTH	= 6
	)
	(
		// Users to add ports here

		// User ports ends
		// Do not modify the ports beyond this line


		// Ports of Axi Slave Bus Interface S_AXI
		input wire  s_axi_aclk,
		input wire  s_axi_aresetn,
		input wire [C_S_AXI_ADDR_WIDTH-1 : 0] s_axi_awaddr,
		input wire [2 : 0] s_axi_awprot,
		input wire  s_axi_awvalid,
		output wire  s_axi_awready,
		input wire [C_S_AXI_DATA_WIDTH-1 : 0] s_axi_wdata,
		input wire [(C_S_AXI_DATA_WIDTH/8)-1 : 0] s_axi_wstrb,
		input wire  s_axi_wvalid,
		output wire  s_axi_wready,
		output wire [1 : 0] s_axi_bresp,
		output wire  s_axi_bvalid,
		input wire  s_axi_bready,
		input wire [C_S_AXI_ADDR_WIDTH-1 : 0] s_axi_araddr,
		input wire [2 : 0] s_axi_arprot,
		input wire  s_axi_arvalid,
		output wire  s_axi_arready,
		output wire [C_S_AXI_DATA_WIDTH-1 : 0] s_axi_rdata,
		output wire [1 : 0] s_axi_rresp,
		output wire  s_axi_rvalid,
		input wire  s_axi_rready,
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
    	output wire midi_out_sysex_last,
		//midi in
		input wire [3:0] midi_in_midi_cmd,
    	input wire [3:0] midi_in_midi_ch,
    	input wire [6:0] midi_in_midi_data1,
    	input wire [6:0] midi_in_midi_data2,
    	output wire midi_in_midi_rd,
    	input wire midi_in_midi_valid,
    	output wire midi_in_midi_busy,
    	input wire [7:0] midi_in_sysex_data,
    	output wire midi_in_sysex_rd,
    	input wire midi_in_sysex_valid,
    	output wire midi_in_sysex_busy,
    	input wire midi_in_sysex_last
	);
	RegBusIF reg_bus_if();
	MidiBus midi_out_if();
	assign midi_out_midi_cmd = midi_out_if.midi_cmd ;
	assign midi_out_midi_ch = midi_out_if.midi_ch ;
	assign midi_out_midi_data1 = midi_out_if.midi_data1 ;
	assign midi_out_midi_data2 = midi_out_if.midi_data2 ;
	assign midi_out_if.midi_rd = midi_out_midi_rd;
	assign midi_out_midi_valid = midi_out_if.midi_valid ;
	assign midi_out_if.midi_busy = midi_out_midi_busy;
	assign midi_out_sysex_data = midi_out_if.sysex_data ;
	assign midi_out_if.sysex_rd = midi_out_sysex_rd ;
	assign midi_out_sysex_valid = midi_out_if.sysex_valid ;
	assign midi_out_if.sysex_busy = midi_out_sysex_busy ;
	assign midi_out_sysex_last = midi_out_if.sysex_last ;
	
	MidiBus midi_in_if();
	assign midi_in_if.midi_cmd = midi_in_midi_cmd;
	assign midi_in_if.midi_ch = midi_in_midi_ch;
	assign midi_in_if.midi_data1 = midi_in_midi_data1;
	assign midi_in_if.midi_data2 = midi_in_midi_data2;
	assign midi_in_midi_rd = midi_in_if.midi_rd;
	assign midi_in_if.midi_valid = midi_in_midi_valid;
	assign midi_in_midi_busy = midi_in_if.midi_busy;
	assign midi_in_if.sysex_data = midi_in_sysex_data;
	assign midi_in_sysex_rd = midi_in_if.sysex_rd;
	assign midi_in_if.sysex_valid = midi_in_sysex_valid;
	assign midi_in_sysex_busy = midi_in_if.sysex_busy;
	assign midi_in_if.sysex_last = midi_in_sysex_last;
	
// Instantiation of Axi Bus Interface S_AXI
	MIDI_v1_0_S_AXI # ( 
		.C_S_AXI_DATA_WIDTH(C_S_AXI_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_S_AXI_ADDR_WIDTH)
	) MIDI_v1_0_S_AXI_inst (
		.S_AXI_ACLK(s_axi_aclk),
		.S_AXI_ARESETN(s_axi_aresetn),
		.S_AXI_AWADDR(s_axi_awaddr),
		.S_AXI_AWPROT(s_axi_awprot),
		.S_AXI_AWVALID(s_axi_awvalid),
		.S_AXI_AWREADY(s_axi_awready),
		.S_AXI_WDATA(s_axi_wdata),
		.S_AXI_WSTRB(s_axi_wstrb),
		.S_AXI_WVALID(s_axi_wvalid),
		.S_AXI_WREADY(s_axi_wready),
		.S_AXI_BRESP(s_axi_bresp),
		.S_AXI_BVALID(s_axi_bvalid),
		.S_AXI_BREADY(s_axi_bready),
		.S_AXI_ARADDR(s_axi_araddr),
		.S_AXI_ARPROT(s_axi_arprot),
		.S_AXI_ARVALID(s_axi_arvalid),
		.S_AXI_ARREADY(s_axi_arready),
		.S_AXI_RDATA(s_axi_rdata),
		.S_AXI_RRESP(s_axi_rresp),
		.S_AXI_RVALID(s_axi_rvalid),
		.S_AXI_RREADY(s_axi_rready),
		.reg_bus_if1(reg_bus_if)
	);
	wire rst = ~s_axi_aresetn;
	BufInterface bus_if(
	   .clk(s_axi_aclk),
	   .rst,
       .reg_bus_if(reg_bus_if),
	   .midi_out_if(midi_out_if),
	   .midi_in_if(midi_in_if)
     );

	endmodule
