#pragma warning(disable:4996)

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "TCPClient.h"

TCPClient::TCPClient(const std::string& ip, const std::string& port) {
    // Initialize Winsock
    _err = WSAStartup(MAKEWORD(2, 2), &_wsa_data);
    if (_err) {
        std::cerr << "error while init winsock" << std::endl;
    }

    // Connect to server
    _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    _server_addr.sin_family = AF_INET;
    _server_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    _server_addr.sin_port = htons(std::atoi(port.c_str()));
}

TCPClient::~TCPClient() {
    closesocket(_socket);
    WSACleanup();
}

bool TCPClient::OpenConnection() {
    _err = connect(_socket, (sockaddr*)&_server_addr, sizeof(_server_addr));
    if (_err == SOCKET_ERROR) {
        std::cerr << "connect failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

bool TCPClient::Send(const std::vector<uint8_t>& data) {
    auto buffer = std::vector<char>(data.begin(), data.end());
    buffer.push_back('\n');

    _err = send(_socket, buffer.data(), buffer.size(), 0);
    if (_err == SOCKET_ERROR) {
        std::cerr << "send failed: " << _err << std::endl;
        return false;
    }
    return true;
}

bool TCPClient::Recive(std::vector<uint8_t>& data) {
    data.clear();
    _err = recv(_socket, recv_buffer, BUFFER_SIZE, 0);
    if (_err > 0) {
        recv_buffer[_err] = 0;
    }
    else if (_err == 0) {
        printf("Connection closing...\n");
    }
    else {
        printf("recv failed: %d\n", WSAGetLastError());
        return false;
    }

    data.insert(data.end(), recv_buffer[0], recv_buffer[strlen(recv_buffer) - 1]);
   
    return true;
}

bool TCPClient::CloseConnection() {
    return closesocket(_socket) == 0;
}