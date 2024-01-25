`timescale 1 ns / 1 ps

module SWPDControl_v1_0 #
(
	parameter integer C_S_AXI_DATA_WIDTH	= 32,
	parameter integer C_S_AXI_ADDR_WIDTH	= 4,
	parameter integer MCU_CLK_DIVIDE = 6, // 1.5MHz
	parameter integer MCU_WAIT_CYCLE = 128
)
(
	// axi lite register
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
	// MCU control
	output wire mcu_clk,
	output wire mcu_data_out,
	input wire mcu_data_in,
	output wire mcu_chip_select_n
);
	wire rst = ~s_axi_aresetn;
	// register
	wire enable;
	wire transfer_running;
	wire [1:0] reply_len;
	wire [7:0] send_data;
	wire transfer_request;
	wire transfer_done;
	wire transfer_error;
	wire [13:0] receive_data;
	// clk gen
	wire mcu_clk_send;
	wire mcu_clk_recv;
	wire wait_done;
	wire wait_reset;
	wire mcu_clk_enable;

	Register # ( 
		.C_S_AXI_DATA_WIDTH(C_S_AXI_DATA_WIDTH),
		.C_S_AXI_ADDR_WIDTH(C_S_AXI_ADDR_WIDTH)
	) register (
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
		.*
	);
	ClockGen #(
		.MCU_CLK_DIVIDE(MCU_CLK_DIVIDE),
		.MCU_WAIT_CYCLE(MCU_WAIT_CYCLE)
	) clk_gen (
		.clk(s_axi_aclk),
		.*
	);
	wire mcu_chip_select;
	Serial #() serial (
		.clk(s_axi_aclk),
		.*
	);
	assign mcu_chip_select_n = ~mcu_chip_select;
endmodule
