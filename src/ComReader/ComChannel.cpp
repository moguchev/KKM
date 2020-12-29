// Leonid Moguchev (c) 2020
#include "ComChannel.h"

ComChannel::ComChannel(const std::wstring& port_name, uint32_t baud_rate)
    : _hSerial(nullptr), _connected(false),
    _port_name(port_name), _baud_rate(baud_rate)
{}

ComChannel::~ComChannel() {
    if (!close()) {
        std::cerr << "potential memory leaks!\n";
    }
}

bool ComChannel::open() {
    _hSerial = ::CreateFile(
        LPCTSTR(_port_name.c_str()),
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        0
    );

    if (_hSerial == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            std::cerr << "serial port does not exist.\n";
            return false;
        }
        std::cerr << "some other error occurred.\n";
        return false;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(_hSerial, &dcbSerialParams)) {
        std::cerr << "getting state error\n";
        return false;
    }
    dcbSerialParams.BaudRate = _baud_rate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(_hSerial, &dcbSerialParams)) {
        std::cerr << "error setting serial port state\n";
        return false;
    }

    _connected = true;

    return true;
}

bool ComChannel::close() {
    // проверяем статус подключение
    if (_connected) {
        // отключаемся
        _connected = false;
        // закрываем дескриптор порта
        return CloseHandle(_hSerial);
    }
    return true;
}

bool ComChannel::write_bytes(const std::vector<uint8_t>& bytes) {
    DWORD dwSize = bytes.size();
    DWORD dwBytesWritten;

    return WriteFile(_hSerial, bytes.data(), dwSize, &dwBytesWritten, NULL);
}

// return {START_BYTE L_LEN H_LEN CMD_CODE DATA_0 ... DATA_N L_CRC H_CRC}
std::vector<uint8_t> ComChannel::read_bytes_until(const std::function<bool(uint8_t)>& condition) {
    auto bytes = std::vector<uint8_t>();
    DWORD iSize;
    uint8_t byte;
    do {
        auto ok = ReadFile(_hSerial, &byte, 1, &iSize, 0);  // читаем по одному байту
        if (ok) {
            bytes.push_back(byte);
        }
        else {
            std::cerr << "error while reading\n";
            break;
        }
    } while (condition(byte));

    return bytes;
}