
`timescale 1 ns / 1 ps

	module MidiBuilder
	(
		input clk,
        input rst,
        // uart fifo in
		input uart_fifo_valid,
        output uart_fifo_rd,
        input [7:0] uart_fifo_data,
        // sysex fifo out
        input sysex_fifo_busy,
        output sysex_fifo_wr,
        output [7:0] sysex_fifo_data,
        output sysex_fifo_last,
        // cmd fifo out
        input cmd_fifo_busy,
        output cmd_fifo_wr,
        output [7:0] cmd_fifo_head,
        output [6:0] cmd_fifo_data1,
        output [6:0] cmd_fifo_data2
	);
    
// =============
// common
// =============
    function [1:0] getSysComLength(input [7:0] data); //先頭データを含めた長さ
        case(data)
            8'hF2: getSysComLength = 2'd3;
            8'hF1: getSysComLength = 2'd2;
            8'hF3: getSysComLength = 2'd2;
            default: getSysComLength = 2'd1;
        endcase
    endfunction
    
    function [1:0] getCmdLength(input [7:0] data); //データのみの長さ
        case(data[7:4])
            4'hC: getCmdLength = 2'd1;
            4'hD: getCmdLength = 2'd1;
            8'h8: getCmdLength = 2'd2;
            8'h9: getCmdLength = 2'd2;
            8'hA: getCmdLength = 2'd2;
            8'hB: getCmdLength = 2'd2;
            8'hE: getCmdLength = 2'd2;
            default: getCmdLength = 2'd0;
        endcase
    endfunction
    
    wire midi_cmd_begin = uart_fifo_data[7] && uart_fifo_valid;
    wire sys_com_begin = (uart_fifo_data[7:3] == 5'h1E) && uart_fifo_valid;//F0~F7
    wire sys_ex_begin = (uart_fifo_data == 8'hF0) && uart_fifo_valid;
    wire sys_ex_end = uart_fifo_data[7] && uart_fifo_valid;
    // sys com
    reg [1:0] sys_com_cursol;
    reg [1:0] sys_com_len;
    wire sys_com_rd_wr;
    wire sys_com_last;
    // sys ex
    wire sys_ex_rd;
    wire sys_ex_wr;
    wire sys_ex_last;
    // cmd
    reg [1:0] cmd_cursol;
    reg [1:0] cmd_len;
    reg [7:0] cmd_head;
    // fifo can r/w
    wire sys_ex_fifo_can_w = ~sysex_fifo_busy;
    wire sys_ex_fifo_can_rw = uart_fifo_valid && sys_ex_fifo_can_w;
    wire cmd_fifo_can_w = ~cmd_fifo_busy;
    wire cmd_fifo_can_r = uart_fifo_valid;
// =============
// status
// =============
    typedef enum logic [3:0] {
        S_IDLE, 
        S_SYS_EX_BEGIN, S_SYS_EX, S_SYS_EX_LAST, 
        S_CMD_BEGIN, S_CMD, S_CMD_LAST, 
        S_COM, S_ERROR
    } Status;
    Status status;
    Status next_status;
    
    always_comb begin
        case(status)
            S_IDLE:
                if (uart_fifo_valid) begin
                    if (sys_ex_begin) next_status <= S_SYS_EX_BEGIN;
                    else if (sys_com_begin) next_status <= S_COM;
                    else if (midi_cmd_begin)  next_status <= S_CMD_BEGIN;
                    else next_status <= S_ERROR;
                end else next_status <= status;
                
            S_COM:
                if (sys_com_last && sys_ex_fifo_can_rw) next_status <= S_IDLE;
                else next_status <= status;
                
            S_ERROR:
                next_status <= S_IDLE;
                
            S_SYS_EX_BEGIN:
                if (sys_ex_fifo_can_rw) next_status <= S_SYS_EX;
                else next_status <= status;
            S_SYS_EX:
                if (sys_ex_end && sys_ex_fifo_can_rw) next_status <= S_SYS_EX_LAST;
                else next_status <= status;
            S_SYS_EX_LAST:
                if (sys_ex_fifo_can_w) next_status <= S_IDLE;
                else next_status <= status;
            S_CMD_BEGIN:
                if (cmd_fifo_can_r) next_status <= S_CMD;
                else next_status <= status;
            S_CMD:
                if (midi_cmd_begin || cmd_len == 2'd0) next_status <= S_IDLE;
                else if (cmd_fifo_can_r && (cmd_cursol + 2'd1) >= cmd_len) next_status <= S_CMD_LAST;
                else next_status <= status;
            S_CMD_LAST:
                if (cmd_fifo_can_w && midi_cmd_begin) next_status <= S_IDLE;
                else if (cmd_fifo_can_w) next_status <= S_CMD;
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
// sys com
// =============
    assign sys_com_rd_wr = (status == S_COM) && sys_ex_fifo_can_rw;
    assign sys_com_last = (sys_com_cursol + 2'd1) == sys_com_len;
    always_ff @(posedge clk) begin
        if (rst) sys_com_len <= 2'd0;
        else if (sys_com_begin && status == S_IDLE) sys_com_len <= getSysComLength(uart_fifo_data);
    end
    always_ff @(posedge clk) begin
        if (rst) sys_com_cursol <= 2'd0;
        else if (sys_com_begin && status == S_IDLE) sys_com_cursol <= 2'd0;
        else if(sys_com_rd_wr) sys_com_cursol <= sys_com_cursol + 2'd1;
    end
    
    wire [7:0] sys_com_data = uart_fifo_data;
// =============
// sys ex
// =============
    wire sys_ex_mode = (status == S_SYS_EX_BEGIN || status == S_SYS_EX || status == S_SYS_EX_LAST);
    
    // lastは0xF7が来ない場合が考えられるのでfifoが書けるかだけを考える
    
    assign sys_ex_rd = sys_ex_fifo_can_rw && (
            (status == S_SYS_EX && next_status == S_SYS_EX) ||
            (status == S_SYS_EX_BEGIN) ||
            (status == S_SYS_EX_LAST && uart_fifo_data == 8'hF7)
    );
    
    assign sys_ex_wr =  (status == S_SYS_EX && next_status == S_SYS_EX && sys_ex_fifo_can_rw) ||
                        (status == S_SYS_EX_LAST && sys_ex_fifo_can_w) || 
                        (status == S_SYS_EX_BEGIN && sys_ex_fifo_can_rw);
                        
    assign sys_ex_last = (status == S_SYS_EX_LAST);
    
    wire [7:0] sys_ex_data = (status == S_SYS_EX_LAST) ? 8'hF7 : uart_fifo_data;

// =============
// cmd
// =============
    always_ff @(posedge clk) begin
        if (rst) cmd_head <= 8'h0;
        else if(status == S_CMD_BEGIN && cmd_fifo_can_r) cmd_head <= uart_fifo_data;
    end
    
    assign cmd_len = getCmdLength(cmd_head);
    
    always_ff @(posedge clk) begin
        if (rst) cmd_cursol <= 2'd0;
        else if (next_status == S_CMD && status != S_CMD) cmd_cursol <= 2'd0;
        else if (status == S_CMD && cmd_fifo_can_r) cmd_cursol <= cmd_cursol + 2'd1;
    end
    
    reg [6:0] cmd_data [2];
    always_ff @(posedge clk) begin
        if (rst) begin
            cmd_data[0] <= 7'd0; cmd_data[1] <= 7'd0;
        end else if (next_status == S_CMD && status != S_CMD) begin
            cmd_data[0] <= 7'd0; cmd_data[1] <= 7'd0;
        end else if(status == S_CMD && cmd_fifo_can_r) begin
            cmd_data[cmd_cursol] <= uart_fifo_data[6:0];
        end
    end
    
    assign cmd_fifo_head = cmd_head;
    assign cmd_fifo_data1 = cmd_data[0];
    assign cmd_fifo_data2 = cmd_data[1];
    
    wire cmd_rd = cmd_fifo_can_r && ((status == S_CMD && next_status != S_IDLE) || status == S_CMD_BEGIN);
    assign cmd_fifo_wr = (status == S_CMD_LAST && cmd_fifo_can_w);
// =============
// common
// =============
    assign sysex_fifo_wr =      (status == S_COM) ? sys_com_rd_wr :
                                (sys_ex_mode) ? sys_ex_wr : 0;
    assign sysex_fifo_data =    (status == S_COM) ? sys_com_data :
                                (sys_ex_mode) ? sys_ex_data : 0;
    assign sysex_fifo_last =    (status == S_COM) ? sys_com_last :
                                (sys_ex_mode) ? sys_ex_last : 0;
    // TODO case
    assign uart_fifo_rd =   (status == S_COM) ? sys_com_rd_wr : 
                            (sys_ex_mode) ? sys_ex_rd:
                            (status == S_ERROR) ? 1 : 
                            (status == S_CMD || status == S_CMD_BEGIN) ? cmd_rd : 0;
    
	endmodule
