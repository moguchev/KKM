#include "com_utils.h"
#include <QDebug>

com_utils::com_utils(int baud_rate) {
    init(baud_rate);
}

void com_utils::init(int baud_rate) {
    LPCTSTR sPortName = L"COM2";
    hSerial = ::CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE,
                           0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (hSerial == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            std::cout << "serial port does not exist.\n";
        }
        std::cout << "some other error occurred.\n";
    }

    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        std::cout << "getting state error\n";
    }
    dcbSerialParams.BaudRate = baud_rate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if(!SetCommState(hSerial, &dcbSerialParams)) {
        std::cout << "error setting serial port state\n";
    }
}

bool com_utils::send_data(const std::vector<byte> &data) {
    DWORD dwSize = data.size();
    DWORD dwBytesWritten;

    char buffer[1031];
    std::copy(data.begin(), data.end(), buffer);

    BOOL iRet = WriteFile(hSerial, buffer, dwSize, &dwBytesWritten, NULL);
    return iRet;
}


byte com_utils::ReadByteCOM() {
    DWORD iSize;
    char sReceivedChar;
    while (true)
    {
       ReadFile(hSerial, &sReceivedChar, 1, &iSize, 0);  // получаем 1 байт
       if (iSize > 0)   // если что-то принято, выводим
           qDebug() << sReceivedChar;
    }
    return sReceivedChar;
}

std::vector<byte> com_utils::read_data() {
    std::vector<byte> data;
    byte res = 0;
    res = ReadByteCOM();
    return data;
}


