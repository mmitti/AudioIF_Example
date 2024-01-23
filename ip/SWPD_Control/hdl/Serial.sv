`timescale 1 ns / 1 ps

module Serial #(
) (
    input clk,
    input rst,
    // clk
    input mcu_clk_send,
    input mcu_clk_recv,
    output wait_reset,
    input wait_done,
    output mcu_clk_enable,
    // mcu
    output mcu_data_out,
	input mcu_data_in,
	output mcu_chip_select,
    // control
    input enable,
    output transfer_running,
    input [1:0] reply_len,
    input [7:0] send_data,
    input transfer_request,
    output transfer_done,
    output reg [13:0] receive_data,
    output transfer_error
);
    reg [$clog2(8) : 0] send_count;
    reg [8:0] buffer;
    reg [1:0] receive_count;
    typedef enum logic [2:0] { 
        S_IDLE,
        S_CS_WAIT1,
        S_CS_WAIT2,
        S_TRANSFER,
        S_DONE_WAIT
    } Status;
    Status status;
    Status next_status;
    
    wire receive_is_data = buffer[7] == 1;
    wire receive_is_wait = buffer[7:0] == 8'b00000000;
    wire receive_is_done = buffer[7:0] == 8'b00000001;
    wire receive_is_invalid = buffer[7:0] == 8'b00000010;
    wire receive_is_unknown_status =  receive_is_data && !(receive_is_wait || receive_is_done || receive_is_invalid);
    
    always_comb begin 
        case(status)
        S_IDLE:
            if (transfer_request) next_status <= S_CS_WAIT1;
            else next_status <= status;
        S_CS_WAIT1:
            if (wait_done) next_status <= S_CS_WAIT2;
            else next_status <= status;
        S_CS_WAIT2:
            if (wait_done) next_status <= S_TRANSFER;
            else next_status <= status;
        S_TRANSFER:
            if (mcu_clk_send && send_count == 7) next_status <= S_DONE_WAIT;
            else next_status <= status;
        S_DONE_WAIT:
            if (wait_done) begin
                if (receive_is_wait || receive_is_data) next_status <= S_TRANSFER;
                else next_status <= S_IDLE;
            end
            else next_status <= status;
        default:
            next_status <= S_IDLE;
        endcase
    end

    
    always_ff @(posedge clk) begin
        if (rst) status <= S_IDLE;
        else status <= next_status;
    end
    
    always_ff @(posedge clk) begin
        if (rst) send_count <= '0;
        else if (status != S_TRANSFER) send_count <= '0;
        else if (mcu_clk_send) send_count <= send_count + 1;
    end
    
    always_ff @(posedge clk) begin
        if (rst) receive_count <= '0;
        else if (transfer_request) receive_count <= '0;
        else if (status == S_DONE_WAIT && wait_done && receive_is_data) receive_count <= receive_count + 1;
    end
    
    always_ff @(posedge clk) begin
        if (rst) receive_data <= '0;
        else if (transfer_request) receive_data <= '0;
        else if (status == S_DONE_WAIT && wait_done && receive_is_data) begin
            receive_data <= {receive_data[6:0], buffer[6:0]};
        end
    end
    
    always_ff @(posedge clk) begin
        if (rst) buffer <= '0;
        else if (transfer_request) buffer <= {1'b0, send_data};
        else if (status == S_DONE_WAIT && next_status == S_TRANSFER) buffer <= '0;
        else if (status == S_TRANSFER && mcu_clk_recv) buffer[8] <= mcu_data_in;
        else if (status == S_TRANSFER && mcu_clk_send) buffer <= {buffer[7:0], buffer[8]};
    end
    
    assign wait_reset = (status == S_IDLE && next_status == S_CS_WAIT1) || 
                        (status == S_CS_WAIT1 && next_status == S_CS_WAIT2) ||
                        (status == S_TRANSFER && next_status == S_DONE_WAIT);
    assign mcu_clk_enable = status == S_TRANSFER;
    assign transfer_running = status != S_IDLE;
    assign transfer_done = status == S_DONE_WAIT && next_status == S_IDLE;
    assign transfer_error = transfer_done && (receive_is_invalid || receive_is_unknown_status || receive_count != reply_len);
    
    assign mcu_data_out = buffer[7];
    assign mcu_chip_select = status != S_IDLE;
    

endmodule
    