// Leonid Moguchev (c) 2020
#pragma once
#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <functional>

class ComChannel {
public:
    ComChannel(const std::wstring& port_name, uint32_t baud_rate);
    ~ComChannel();
    bool open();
    bool close();

    bool write_bytes(const std::vector<byte>& bytes);
    std::vector<byte> read_bytes_until(const std::function<bool(byte)>& condition);
private:
    // дескриптор COM-порта
    HANDLE _hSerial;
    bool _connected;

    std::wstring _port_name;
    uint32_t _baud_rate;
};