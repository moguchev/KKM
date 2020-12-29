// Leonid Moguchev 2020
#pragma once

#include <iostream>
#include <string>
#include <vector>

#define WIN32_LEAN_AND_MEAN

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

class TCPClient {
public:
    TCPClient(const std::string& ip, const std::string& port);

    ~TCPClient();
    bool OpenConnection();

    bool Send(const std::vector<uint8_t>& data);

    bool Recive(std::vector<uint8_t>& data);

    bool CloseConnection();

private:
    WSADATA _wsa_data;
    SOCKET _socket;
    sockaddr_in _server_addr;
    int _err;

    const size_t BUFFER_SIZE = 1024;
    char* recv_buffer = new char[BUFFER_SIZE];
};