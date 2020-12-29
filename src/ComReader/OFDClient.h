#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <memory>

#include "Hardware.h"
#include "TCPClient.h"

class OFDClient {
public:
    explicit OFDClient(std::shared_ptr<Hardware> hw) :
        _hw(hw) {};

    bool Connect(const std::string& ip, const std::string& port) {
        _cli = std::make_shared<TCPClient>(ip, port);
        return _cli->OpenConnection();
    }

    bool SendDocuments(const std::string& kkt_number) {
        //_cli->OpenConnection();
        auto resp21 = _hw->__21__SetTransportStatus(true);
        if (resp21->ErrorMsg != "") {
            std::cerr << "set transport status: " + resp21->ErrorMsg << std::endl;
            return false;
        }

        std::vector<uint8_t> req;
        // KKT_NUMBER
        req.push_back(kkt_number.size());
        req.insert(req.end(), kkt_number.begin(), kkt_number.end());
        // FN_NUMBER
        auto resp31 = _hw->__31__GetFNNumber();
        if (resp31->ErrorMsg != "") {
            std::cerr << "get fn number: " + resp31->ErrorMsg << std::endl;
            return false;
        }
        req.push_back(resp31->Number_cp866.size());
        req.insert(req.end(), resp31->Number_cp866.begin(), resp31->Number_cp866.end());
        // DATA
        auto resp22 = _hw->__22__StartReadMessage();
        if (resp22->ErrorMsg != "") {
            std::cerr << "start read fn: " + resp22->ErrorMsg << std::endl;
            return false;
        }
        auto resp23 = _hw->__23__ReadBlock(0, 1024);
        if (resp23->ErrorMsg != "") {
            std::cerr << "read fn: " + resp23->ErrorMsg << std::endl;
            return false;
        }
        req.insert(req.end(), resp23->Message.begin(), resp23->Message.end());
        auto resp25 = _hw->__25__EndReadMessage();
        if (resp25->ErrorMsg != "") {
            std::cerr << "end read fn: " + resp25->ErrorMsg << std::endl;
            return false;
        }

        // SEND TO OFD
        if (!_cli->Send(req)) {
            std::cerr << "send to ofd" << std::endl;
            return false;
        }
        // RECIVE FROM OFD
        std::vector<uint8_t> answer;
        if (!_cli->Recive(answer)) {
            std::cerr << "recive from ofd" << std::endl;
            return false;
        }
        // SEND TO FN
        auto resp26 = _hw->__26__SendOFDAnswer(SendOFDAnswerRequest{ answer });
        if (resp26->ErrorMsg != "") {
            std::cerr << "send to fn: " + resp26->ErrorMsg << std::endl;
            _cli->CloseConnection();
            return false;
        }

        return true;
    }
private:
    std::shared_ptr<Hardware> _hw;
    std::shared_ptr<TCPClient> _cli;
};