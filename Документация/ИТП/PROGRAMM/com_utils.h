#ifndef COM_UTILS_H
#define COM_UTILS_H

#include <stdio.h>
#include <tchar.h>
#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>

class com_utils {

public:
    com_utils(int baud_rate);

    bool send_data(const std::vector<byte> &data);

    std::vector<byte> read_data();

private:
    HANDLE hSerial;
    void init(int baud_rate);
    byte ReadByteCOM();
};

#endif // COM_UTILS_H
