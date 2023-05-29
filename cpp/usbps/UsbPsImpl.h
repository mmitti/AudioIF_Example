#pragma once
namespace usbps{
    // from xusbps.h
    static constexpr int TIMEOUT_COUNTER = 1000000;
    static constexpr int MAX_PACKET_SIZE = 1024;
    static constexpr int ENDPOINT_MAXP_LENGTH = 0x400;
    static constexpr int ENDPOINT_MAXP_MULT_MASK = 0xC00;
    static constexpr int ENDPOINT_MAXP_MULT_SHIFT = 10;
    static constexpr int DEFAULT_ALT_SETTING = 0;
}

