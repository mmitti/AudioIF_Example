
`timescale 1 ns / 1 ps

	module MIDI_Clock_v1_0 #
	(
		parameter integer SYS_CLK_BASE = 4,
        parameter integer MIDI_CLK_BASE = 7,
        parameter integer KEEP_ARRIVE_BASE = 20,
        parameter integer CLK_BUS_NUM = 1
	)
	(
		input pll_clk_in,//8M
		input locked,
    // 1
        output midi_system_clock_1,
        output midi_clock_1,
        output keep_alive_1,
        output midi_rst_1,
        output midi_clk_locked_1,
        // 2
        output midi_system_clock_2,
        output midi_clock_2,
        output keep_alive_2,
        output midi_rst_2,
        output midi_clk_locked_2,
        // 3
        output midi_system_clock_3,
        output midi_clock_3,
        output keep_alive_3,
        output midi_rst_3,
        output midi_clk_locked_3,
        //4
        output midi_system_clock_4,
        output midi_clock_4,
        output keep_alive_4,
        output midi_rst_4,
        output midi_clk_locked_4
	);
	
    wire midi_system_clock;
    reg midi_clock;
    wire keep_alive;
    reg midi_rst;
    wire midi_clk_locked;
    
    // 1
    assign midi_system_clock_1 = midi_system_clock;
    assign midi_clock_1 = midi_clock;
    assign keep_alive_1 = keep_alive;
    assign midi_rst_1 = midi_rst;
    assign midi_clk_locked_1 = midi_clk_locked;
    // 2
    assign midi_system_clock_2 = midi_system_clock;
    assign midi_clock_2 = midi_clock;
    assign keep_alive_2 = keep_alive;
    assign midi_rst_2 = midi_rst;
    assign midi_clk_locked_2 = midi_clk_locked;
    // 3
    assign midi_system_clock_3 = midi_system_clock;
    assign midi_clock_3 = midi_clock;
    assign keep_alive_3 = keep_alive;
    assign midi_rst_3 = midi_rst;
    assign midi_clk_locked_3 = midi_clk_locked;
    //4
    assign midi_system_clock_4 = midi_system_clock;
    assign midi_clock_4 = midi_clock;
    assign keep_alive_4 = keep_alive;
    assign midi_rst_4 = midi_rst;
    assign midi_clk_locked_4 = midi_clk_locked;
    
    
	assign midi_clk_locked = locked;
    wire rst = ~locked;
    reg [20:0] clk_delay;
    reg request_rst;
    always_ff @(posedge pll_clk_in) begin
        if (rst) clk_delay <= 21'd0;
        else clk_delay <= clk_delay + 21'd1;
    end
    always_ff @(posedge pll_clk_in) begin
       if (rst) request_rst <= 1;
       else if(clk_delay == 21'h001000) request_rst <= 0;
    end
    always_ff @(posedge midi_system_clock) begin
        if (rst) midi_rst <= 1;
        else if(request_rst) midi_rst <= 1;
        else midi_rst <= 0;
    end
    always_ff @(posedge midi_system_clock) begin
       if (midi_rst) midi_clock <= 0;
       else midi_clock <= clk_delay[MIDI_CLK_BASE];//31.25k
    end
    
    assign midi_system_clock = clk_delay[SYS_CLK_BASE];//250k
    
    reg [1:0] keep_alive_tmp;
    
    always_ff @(posedge midi_system_clock) begin
        if (midi_rst) keep_alive_tmp <= 2'h0;
        else keep_alive_tmp <= {keep_alive_tmp[0], clk_delay[KEEP_ARRIVE_BASE]};
    end
    assign keep_alive = ~keep_alive_tmp[0] & keep_alive_tmp[1];//3.8 plus  
    
	endmodule
