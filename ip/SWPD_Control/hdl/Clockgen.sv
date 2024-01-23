`timescale 1 ns / 1 ps

module ClockGen #(
    parameter integer MCU_CLK_DIVIDE = 6,
	parameter integer MCU_WAIT_CYCLE = 128
) (
    input clk,
    input rst,
    output mcu_clk,
    output mcu_clk_send,
    output mcu_clk_recv,
    output wait_done,
    input wait_reset,
    input mcu_clk_enable
);
    reg [$clog2(MCU_WAIT_CYCLE + 1):0] wait_count;
    reg [MCU_CLK_DIVIDE - 1 : 0] clk_delay;
    reg prev_enable;
    always_ff @(posedge clk) begin
        if (rst) prev_enable <= 0;
        else prev_enable <= mcu_clk_enable;
    end
    wire clk_rst = ~prev_enable && mcu_clk_enable;
    always_ff @(posedge clk) begin
        if (rst) clk_delay <= '0;
        else if(clk_rst) clk_delay <= '0;
        else clk_delay <= clk_delay + 1;
    end
    wire output_enable = mcu_clk_enable && ~clk_rst;
    assign mcu_clk = output_enable && clk_delay[MCU_CLK_DIVIDE - 1];
    reg mcl_prev_clk;
    always_ff @(posedge clk) begin
        if (rst) mcl_prev_clk <= 0;
        else mcl_prev_clk <= mcu_clk;
    end
    assign mcu_clk_send = mcl_prev_clk && ~mcu_clk && output_enable; // negedge
    assign mcu_clk_recv = ~mcl_prev_clk && mcu_clk && output_enable; // posedge
    
    always_ff @(posedge clk) begin
        if (rst) wait_count <= '0;
        else if (wait_reset) wait_count <= 1;
        else if (clk_delay == '1 && wait_count != 0) wait_count <= wait_count + 1;
    end
    
    assign wait_done = wait_count == '1;
endmodule
