interface RegBusIF;
    logic [23:0] cmd_wdata;
    logic cmd_wr;
    logic cmd_wbusy;
    
    logic cmd_rd;
    logic [23:0] cmd_rdata;
    logic cmd_rvalid;
    
    logic [23:0] sysex_wdata;
    logic sysex_wr;
    logic sysex_wbusy;
    logic sysex_wlast;
    logic [1:0] sysex_wlen;
    
    logic [23:0] sysex_rdata;
    logic sysex_rd;
    logic sysex_rvalid;
    logic sysex_rlast;
    logic [1:0] sysex_rlen;
    
    modport reg_ctl(
        output cmd_wdata, cmd_wr, cmd_rd,
        input cmd_wbusy, cmd_rdata, cmd_rvalid,
        output sysex_wdata, sysex_wr, sysex_wlast, sysex_wlen, sysex_rd,
        input sysex_wbusy, sysex_rdata, sysex_rvalid, sysex_rlast, sysex_rlen
    );
    modport bus_if(
        input cmd_wdata, cmd_wr, cmd_rd,
        output cmd_wbusy, cmd_rdata, cmd_rvalid,
        input sysex_wdata, sysex_wr, sysex_wlast, sysex_wlen, sysex_rd,
        output sysex_wbusy, sysex_rdata, sysex_rvalid, sysex_rlast, sysex_rlen
    );
    modport cmd_if(
        input cmd_wdata, cmd_wr, cmd_rd,
        output cmd_wbusy, cmd_rdata, cmd_rvalid
    );
    modport sysex_if(
        input sysex_wdata, sysex_wr, sysex_wlast, sysex_wlen, sysex_rd,
        output sysex_wbusy, sysex_rdata, sysex_rvalid, sysex_rlast, sysex_rlen
    );
    
endinterface

interface MidiBus;
    logic [3:0] midi_cmd;
    logic [3:0] midi_ch;
    logic [6:0] midi_data1;
    logic [6:0] midi_data2;
    logic midi_rd;
    logic midi_valid;
    logic midi_busy;

    logic [7:0] sysex_data;
    logic sysex_rd;
    logic sysex_valid;
    logic sysex_busy;
    logic sysex_last;

    modport Sender(
        output midi_cmd, midi_ch, midi_data1, midi_data2, midi_valid,
        input midi_rd, midi_busy,
        output sysex_data, sysex_valid, sysex_last,
        input sysex_rd, sysex_busy
    );
    
    modport Receiver(
        input midi_cmd, midi_ch, midi_data1, midi_data2, midi_valid,
        output midi_rd, midi_busy,
        input sysex_data, sysex_valid, sysex_last,
        output sysex_rd, sysex_busy
    );
    
    modport MidiSender(
        output midi_cmd, midi_ch, midi_data1, midi_data2, midi_valid,
        input midi_rd, midi_busy
    );
    
    modport MidiReceiver(
        input midi_cmd, midi_ch, midi_data1, midi_data2, midi_valid,
        output midi_rd, midi_busy
    );
    
    modport SysExSender(
        output sysex_data, sysex_valid, sysex_last,
        input sysex_rd, sysex_busy
    );
    
    modport SysExReceiver(
        input sysex_data, sysex_valid, sysex_last,
        output sysex_rd, sysex_busy
    );

endinterface
