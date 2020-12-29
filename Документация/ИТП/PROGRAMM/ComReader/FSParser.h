// Leonid Moguchev (c) 2020
#pragma once
#include "utils.h"
#include <string>
#include <iostream>

const uint8_t MSG_START = 0x04;

class FSParser {
public:
    FSParser() = default;
    ~FSParser() = default;
    bool static process_byte(uint8_t b);
private:
    bool static _reading_command;
    uint16_t static _msg_len;
    size_t static _sum_bytes;
    size_t static _read_bytes;
};
