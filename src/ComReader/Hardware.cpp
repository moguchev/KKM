// Leonid Moguchev (c) 2020
#include "Hardware.h"

Hardware::Hardware(const std::wstring& port_name, uint32_t baud) {
    _com_io = std::make_unique<ComChannel>(port_name, baud);
    _connection_success = _com_io->open();
    if (!_connection_success) {
        std::cerr << "open com port failed!" << std::endl;
    }
}

std::shared_ptr<Message> Hardware::parse_bytes(std::vector<uint8_t>&& bytes) {
    auto msg = std::make_shared<Message>();
    if (bytes.size() < 6) {
        msg->Code = INTERNAL_ERROR;
        return msg;
    }

    msg->Length = utils::union_bytes(bytes[1], bytes[2]);
    msg->Code = bytes[3];
    msg->Data = std::vector<uint8_t>(bytes.begin() + 4, bytes.begin() + bytes.size() - 2);
    msg->Crc = utils::union_bytes(bytes[bytes.size() - 2], bytes[bytes.size() - 1]);
    return msg;
}

bool Hardware::check_crc(const Message& msg) {
    auto bytes = utils::split_le(msg.Length);
    bytes.push_back(msg.Code);
    bytes.insert(bytes.end(), msg.Data.begin(), msg.Data.end());

    auto real_sum = crc16_ccitt(bytes.data(), bytes.size());
    return real_sum == msg.Crc;
}

bool Hardware::get_connection_status() {
    bool good_connect = false;
    if (_connection_success) {
        auto res = this->__30__GetFNStatus();
        good_connect = res->ErrorMsg == "";
    }
    return good_connect;
};

std::shared_ptr<StartFiscalisationResponse> Hardware::__02__StartFiscalisation() {
    auto result = std::make_shared<StartFiscalisationResponse>();
    uint8_t cmd = 0x02;
    std::vector<uint8_t> request = { MSG_START, 0x01, 0x00, cmd };

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    return result;
}

std::shared_ptr<ApproveFiscalisationResponse> Hardware::__03__ApproveFiscalisation(const ApproveFiscalisationRequest& req) {
    auto result = std::make_shared<ApproveFiscalisationResponse>();
    uint8_t cmd = 0x03;

    auto data = req.to_bytes();
    auto len = utils::split_le(uint16_t(data.size() + 1));
    std::vector<uint8_t> request = { MSG_START, len[0], len[1], cmd };
    request.insert(request.end(), data.begin(), data.end());

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code: " << response->Code << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    if (response->Length - 1 != response->Data.size() || response->Length != 9) {
        std::cerr << "wrong data size" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    }

    result->FiscDocNumber = utils::union_bytes(
        response->Data[0], response->Data[1], response->Data[2], response->Data[3]);

    result->FiscSign = utils::union_bytes(
        response->Data[4], response->Data[5], response->Data[6], response->Data[7]);

    return result;
}

std::shared_ptr<StartCloseFiscalisationResponse> Hardware::__04__StartCloseFiscalisation() {
    auto result = std::make_shared<StartCloseFiscalisationResponse>();
    uint8_t cmd = 0x04;
    std::vector<uint8_t> request = { MSG_START, 0x01, 0x00, cmd };

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    return result;
}

std::shared_ptr<CloseFiscalisationResponse> Hardware::__05__CloseFiscalisation(const CloseFiscalisationRequest& req) {
    auto result = std::make_shared<CloseFiscalisationResponse>();
    uint8_t cmd = 0x05;

    auto data = req.to_bytes();
    auto len = utils::split_le(uint16_t(data.size() + 1));
    std::vector<uint8_t> request = { MSG_START, len[0], len[1], cmd };
    request.insert(request.end(), data.begin(), data.end());

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code: " << response->Code << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    if (response->Length - 1 != response->Data.size() || response->Length != 9) {
        std::cerr << "wrong data size" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    }

    result->FiscDocNumber = utils::union_bytes(
        response->Data[0], response->Data[1], response->Data[2], response->Data[3]);

    result->FiscSign = utils::union_bytes(
        response->Data[4], response->Data[5], response->Data[6], response->Data[7]);

    return result;
}

std::shared_ptr<CancelDocumentResponse> Hardware::__06__CancelDocuments() {
    auto result = std::make_shared<CancelDocumentResponse>();
    uint8_t cmd = 0x06;
    std::vector<uint8_t> request = { MSG_START, 0x01, 0x00, cmd };

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    return result;
}

std::shared_ptr<SendDocumentsResponse> Hardware::__07__SendDocuments(const TLVList& list) {
    auto result = std::make_shared<SendDocumentsResponse>();
    uint8_t cmd = 0x07;

    auto chunks = list.to_bytes_with_limit(TLV_LIMIT);
    for (const auto& ch : chunks) {
        auto len = utils::split_le(uint16_t(ch.size() + 1));
        std::vector<uint8_t> request = { MSG_START, len[0], len[1], cmd };
        request.insert(request.end(), ch.begin(), ch.end());

        auto ptr = request.data();
        // crc считается без байта MSG_START
        auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
        request.insert(request.end(), crc.begin(), crc.end());

        if (!_com_io->write_bytes(request)) {
            std::cerr << "write bytes" << std::endl;
            result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
            return result;
        };

        auto bytes = _com_io->read_bytes_until(_parser.process_byte);
        auto response = parse_bytes(std::move(bytes));

        if (!check_crc(*response)) {
            std::cerr << "bad control sum" << std::endl;
            result->ErrorMsg = CRC_ERROR;
            return result;
        }

        if (response->Code != STATUS_OK) {
            std::cerr << "error code" << std::endl;
            result->ErrorMsg = ERROR_TEXT[response->Code];
            return result;
        }
    }

    return result;
}

std::shared_ptr<GetShiftStatusResponse> Hardware::__10__GetShiftStatus() {
    auto result = std::make_shared<GetShiftStatusResponse>();
    uint8_t cmd = 0x10;
    std::vector<uint8_t> request = { MSG_START, 0x01, 0x00, cmd };

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    if (response->Length - 1 != response->Data.size() || response->Length != 6) {
        std::cerr << "wrong data size" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    }

    result->ShiftOpen = response->Data[0] == 0x01;
    result->ShiftNum = utils::union_bytes(response->Data[1], response->Data[2]);
    result->CheckAmmount = utils::union_bytes(response->Data[3], response->Data[4]);

    return result;
}

std::shared_ptr<StartOpeningShiftResponse> Hardware::__11__StartOpeningShift(const StartOpeningShiftRequest& req) {
    auto result = std::make_shared<StartOpeningShiftResponse>();
    uint8_t cmd = 0x11;

    auto data = req.to_bytes();
    auto len = utils::split_le(uint16_t(data.size() + 1));
    std::vector<uint8_t> request = { MSG_START, len[0], len[1], cmd };
    request.insert(request.end(), data.begin(), data.end());

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    return result;
}

std::shared_ptr<ApproveOpeningShiftResponse> Hardware::__12__ApproveOpeningShift() {
    auto result = std::make_shared<ApproveOpeningShiftResponse>();
    uint8_t cmd = 0x12;
    std::vector<uint8_t> request = { MSG_START, 0x01, 0x00, cmd };

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    if (response->Length - 1 != response->Data.size() || response->Length != 11) {
        std::cerr << "wrong data size" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    }

    result->ShiftNum = utils::union_bytes(response->Data[0], response->Data[1]);

    result->FiscDocNumber = utils::union_bytes(
        response->Data[2], response->Data[3], response->Data[4], response->Data[5]);

    result->FiscSign = utils::union_bytes(
        response->Data[6], response->Data[7], response->Data[8], response->Data[9]);

    return result;
}

std::shared_ptr<StartCloseShiftResponse> Hardware::__13__StartCloseShift(const StartCloseShiftRequest& req) {
    auto result = std::make_shared<StartCloseShiftResponse>();
    uint8_t cmd = 0x13;

    auto data = req.to_bytes();
    auto len = utils::split_le(uint16_t(data.size() + 1));
    std::vector<uint8_t> request = { MSG_START, len[0], len[1], cmd };
    request.insert(request.end(), data.begin(), data.end());

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    return result;
}

std::shared_ptr<ApproveCloseShiftResponse> Hardware::__14__ApproveCloseShift() {
    auto result = std::make_shared<ApproveCloseShiftResponse>();
    uint8_t cmd = 0x14;
    std::vector<uint8_t> request = { MSG_START, 0x01, 0x00, cmd };

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    if (response->Length - 1 != response->Data.size() || response->Length != 11) {
        std::cerr << "wrong data size" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    }

    result->ShiftNum = utils::union_bytes(response->Data[0], response->Data[1]);

    result->FiscDocNumber = utils::union_bytes(
        response->Data[2], response->Data[3], response->Data[4], response->Data[5]);

    result->FiscSign = utils::union_bytes(
        response->Data[6], response->Data[7], response->Data[8], response->Data[9]);

    return result;
}

std::shared_ptr<StartCheckResponse> Hardware::__15__StartCheck(const StartCheckRequest& req) {
    auto result = std::make_shared<StartCheckResponse>();
    uint8_t cmd = 0x15;

    auto data = req.to_bytes();
    auto len = utils::split_le(uint16_t(data.size() + 1));
    std::vector<uint8_t> request = { MSG_START, len[0], len[1], cmd };
    request.insert(request.end(), data.begin(), data.end());

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    return result;
}

std::shared_ptr<CreateCheckResponse> Hardware::__16__CreateCheck(const CreateCheckRequest& req) {
    auto result = std::make_shared<CreateCheckResponse>();
    uint8_t cmd = 0x16;

    auto data = req.to_bytes();
    auto len = utils::split_le(uint16_t(data.size() + 1));
    std::vector<uint8_t> request = { MSG_START, len[0], len[1], cmd };
    request.insert(request.end(), data.begin(), data.end());

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    if (response->Length - 1 != response->Data.size() || response->Length != 11) {
        std::cerr << "wrong data size" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    }

    result->CheckNum = utils::union_bytes(response->Data[0], response->Data[1]);

    result->FiscDocNumber = utils::union_bytes(
        response->Data[2], response->Data[3], response->Data[4], response->Data[5]);

    result->FiscSign = utils::union_bytes(
        response->Data[6], response->Data[7], response->Data[8], response->Data[9]);

    return result;
}

std::shared_ptr<StartCheckCorrectionResponse> Hardware::__17__StartCheckCorrection(const StartCheckCorrectionRequest& req) {
    auto result = std::make_shared<StartCheckCorrectionResponse>();
    uint8_t cmd = 0x17;

    auto data = req.to_bytes();
    auto len = utils::split_le(uint16_t(data.size() + 1));
    std::vector<uint8_t> request = { MSG_START, len[0], len[1], cmd };
    request.insert(request.end(), data.begin(), data.end());

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    return result;
}

std::shared_ptr<GetOFDStatusResponse> Hardware::__20__GetOFDStatus() {
    auto result = std::make_shared<GetOFDStatusResponse>();
    uint8_t cmd = 0x20;
    std::vector<uint8_t> request = { MSG_START, 0x01, 0x00, cmd };

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    if (response->Length - 1 != response->Data.size() || response->Length != 14) {
        std::cerr << "wrong data size" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    }

    auto status = response->Data[0];
    if (status & 0x01) {
        result->Status.push_back("транспортное соединение установлено");
    } else {
        result->Status.push_back("транспортное соединение не установлено");
    }
    if (status & 0x02) {
        result->Status.push_back("есть сообщение для передачи в ОФД");
    } else {
        result->Status.push_back("нет сообщение для передачи в ОФД");
    }
    if (status & 0x04) {
        result->Status.push_back("ожидание ответного сообщения(квитанции) от ОФД");
    }

    result->StartRead = response->Data[1] > 0;
    result->MsgAmmount = utils::union_bytes(response->Data[2], response->Data[3]);
    result->FirstDocNum = utils::union_bytes(response->Data[4], response->Data[5], response->Data[6], response->Data[7]);
    result->FirstDocDateTime = utils::date_time_from_bytes(std::vector<uint8_t>(response->Data.begin() + 8, response->Data.begin() + 13));
    
    return result;
}

std::shared_ptr<SetTransportStatusResponse> Hardware::__21__SetTransportStatus(bool connected) {
    auto result = std::make_shared<SetTransportStatusResponse>();
    uint8_t cmd = 0x21;

    std::vector<uint8_t> data = { uint8_t(connected) };
    auto len = utils::split_le(uint16_t(data.size() + 1));
    std::vector<uint8_t> request = { MSG_START, len[0], len[1], cmd };
    request.insert(request.end(), data.begin(), data.end());

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    return result;
}

std::shared_ptr<StartReadMessageResponse> Hardware::__22__StartReadMessage() {
    auto result = std::make_shared<StartReadMessageResponse>();
    uint8_t cmd = 0x22;
    std::vector<uint8_t> request = { MSG_START, 0x01, 0x00, cmd };

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    if (response->Length - 1 != response->Data.size() || response->Length != 3) {
        std::cerr << "wrong data size" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    }

    result->MsgLen = utils::union_bytes(response->Data[0], response->Data[1]);

    return result;
}

std::shared_ptr<ReadBlockResponse> Hardware::__23__ReadBlock(uint16_t offset, uint16_t limit) {
    auto result = std::make_shared<ReadBlockResponse>();
    uint8_t cmd = 0x23;

    auto data = utils::split_le(offset);
    auto l = utils::split_le(limit);
    data.insert(data.end(), l.begin(), l.end());
    auto len = utils::split_le(uint16_t(data.size() + 1));
    std::vector<uint8_t> request = { MSG_START, len[0], len[1], cmd };
    request.insert(request.end(), data.begin(), data.end());

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    result->Message = response->Data;

    return result;
}

std::shared_ptr<CancelReadMessageResponse>  Hardware::__24__CancelReadMessage() {
    auto result = std::make_shared<CancelReadMessageResponse>();
    uint8_t cmd = 0x24;
    std::vector<uint8_t> request = { MSG_START, 0x01, 0x00, cmd };

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    return result;
}

std::shared_ptr<EndReadMessageResponse>  Hardware::__25__EndReadMessage() {
    auto result = std::make_shared<EndReadMessageResponse>();
    uint8_t cmd = 0x25;
    std::vector<uint8_t> request = { MSG_START, 0x01, 0x00, cmd };

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    return result;
}

std::shared_ptr<SendOFDAnswerResponse> Hardware::__26__SendOFDAnswer(const SendOFDAnswerRequest& req) {
    auto result = std::make_shared<SendOFDAnswerResponse>();
    uint8_t cmd = 0x26;

    auto len = utils::split_le(uint16_t(req.Answer.size() + 1));
    std::vector<uint8_t> request = { MSG_START, len[0], len[1], cmd };
    request.insert(request.end(), req.Answer.begin(), req.Answer.end());

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    if (response->Length - 1 != response->Data.size() || response->Length != 2) {
        std::cerr << "wrong data size" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    }

    result->Code = response->Data[0];

    return result;
}

std::shared_ptr<GetFNStatusResponse> Hardware::__30__GetFNStatus() {
    auto result = std::make_shared<GetFNStatusResponse>();
    uint8_t cmd = 0x30;
    std::vector<uint8_t> request = { MSG_START, 0x01, 0x00, cmd };

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    if (response->Length - 1 != response->Data.size() || response->Length != 31) {
        std::cerr << "wrong data size" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    }

    result->PhaseOfLife = FN_LIFE_PHASES[response->Data[0]];
    result->CurrentDocument = CURRENT_DOCUMENT_TYPES[response->Data[1]];
    result->DocumentDataRecived = response->Data[2] == 1;
    result->ShiftIsOpen = response->Data[3] == 1;
    result->Warnings = WARNING_FLAGS[response->Data[4]];
    result->DateTime = utils::date_time_from_bytes(
        std::vector<uint8_t>(response->Data.begin() + 5, response->Data.begin() + 10));
    result->Number_cp866 = std::string(response->Data.begin() + 10, response->Data.begin() + 26);
    result->LastFDNumber = utils::union_bytes(
        response->Data[26], response->Data[27], response->Data[28], response->Data[29]);

    return result;
}

std::shared_ptr<GetFNNumberResponse> Hardware::__31__GetFNNumber() {
    auto result = std::make_shared<GetFNNumberResponse>();
    uint8_t cmd = 0x31;
    std::vector<uint8_t> request = { MSG_START, 0x01, 0x00, cmd };

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    if (response->Length - 1 != response->Data.size() || response->Length != 17) {
        std::cerr << "wrong data size" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    }

    result->Number_cp866 = std::string(response->Data.begin(), response->Data.end());

    return result;
}

std::shared_ptr<GetFNEndDateResponse> Hardware::__32__GetFNEndDate() {
    auto result = std::make_shared<GetFNEndDateResponse>();
    uint8_t cmd = 0x32;
    std::vector<uint8_t> request = { MSG_START, 0x01, 0x00, cmd };

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    if (response->Length - 1 != response->Data.size() || response->Length != 6) {
        std::cerr << "wrong data size" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    }

    result->Date = utils::date_from_bytes(std::vector<uint8_t>(response->Data.begin(), response->Data.begin() + 3));
    result->LeftRegistrations = (response->Data[3]);
    result->DoneRegistrations = (response->Data[4]);

    return result;
}

std::shared_ptr<GetFNVersionResponse> Hardware::__33__GetFNVersion() {
    auto result = std::make_shared<GetFNVersionResponse>();
    uint8_t cmd = 0x33;
    std::vector<uint8_t> request = { MSG_START, 0x01, 0x00, cmd };

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    if (response->Length - 1 != response->Data.size() || response->Length != 18) {
        std::cerr << "wrong data size" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    }

    result->VersionSoftWare_crc866 = std::string(response->Data.begin(), response->Data.begin() + 16);

    switch (response->Data[16]) {
    case 0:
        result->TypeSoftWare = "отладочная версия";
        break;
    case 1:
        result->TypeSoftWare = "серийная версия";
        break;
    }

    return result;
}

std::shared_ptr<GetFiscDocumentResponse> Hardware::__40__GetFiscDocument(uint32_t num) {
    auto result = std::make_shared<GetFiscDocumentResponse>();
    uint8_t cmd = 0x40;

    auto data = utils::split_le(num);
    auto len = utils::split_le(uint16_t(data.size() + 1));
    std::vector<uint8_t> request = { MSG_START, len[0], len[1], cmd };
    request.insert(request.end(), data.begin(), data.end());

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    if (response->Length - 1 != response->Data.size() || response->Data.size() < 3) {
        std::cerr << "wrong data size" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    }

    result->DocumentType = DocumentType(response->Data[0]);
    result->GetOFDReceipt = response->Data[1] == 1;

    auto doc_data = std::vector<uint8_t>(response->Data.begin() + 2, response->Data.end());

    switch (result->DocumentType) {
    case DocumentType::CHECK:
        result->Document = new CheckDocument();
        break;
    case DocumentType::CLOSE_FISCAL_REGIME:
        result->Document = new CloseFiscDocumnet();
        break;
    case DocumentType::CLOSE_SHIFT:
        result->Document = new ShiftDocument();
        break;
    case DocumentType::OPEN_SHIFT:
        result->Document = new ShiftDocument();
        break;
    case DocumentType::REGISTRATION:
        result->Document = new RegistrationDocument();
        break;
    }
    result->Document->init(doc_data);

    return result;
}

std::shared_ptr<GetFDAmmountWithoutOFDResponse> Hardware::__42__GetFDAmmountWithoutOFD() {
    auto result = std::make_shared<GetFDAmmountWithoutOFDResponse>();
    uint8_t cmd = 0x42;
    std::vector<uint8_t> request = { MSG_START, 0x01, 0x00, cmd };

    auto ptr = request.data();
    // crc считается без байта MSG_START
    auto crc = utils::split_le(crc16_ccitt(++ptr, request.size() - 1));
    request.insert(request.end(), crc.begin(), crc.end());

    if (!_com_io->write_bytes(request)) {
        std::cerr << "write bytes" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    };

    auto bytes = _com_io->read_bytes_until(_parser.process_byte);
    auto response = parse_bytes(std::move(bytes));

    if (!check_crc(*response)) {
        std::cerr << "bad control sum" << std::endl;
        result->ErrorMsg = CRC_ERROR;
        return result;
    }

    if (response->Code != STATUS_OK) {
        std::cerr << "error code" << std::endl;
        result->ErrorMsg = ERROR_TEXT[response->Code];
        return result;
    }

    if (response->Length - 1 != response->Data.size() || response->Length != 3) {
        std::cerr << "wrong data size" << std::endl;
        result->ErrorMsg = ERROR_TEXT[INTERNAL_ERROR];
        return result;
    }

    result->Ammount = utils::union_bytes(response->Data[0], response->Data[1]);

    return result;
}

std::vector<uint8_t> ApproveFiscalisationRequest::to_bytes() const {
    std::vector<uint8_t> res = utils::time_to_bytes(this->DateTime);

    auto inn = std::vector<uint8_t>(this->Inn_cp866.begin(), this->Inn_cp866.end());
    while (inn.size() < 12) {
        inn.push_back(0x00);
    }
    res.insert(res.end(), inn.begin(), inn.begin() + 12);

    auto kkt = std::vector<uint8_t>(this->KKTNumber_cp866.begin(), this->KKTNumber_cp866.end());
    while (kkt.size() < 20) {
        kkt.push_back(0x00);
    }
    res.insert(res.end(), kkt.begin(), kkt.begin() + 20);

    res.push_back(uint8_t(this->NalogCode));
    res.push_back(uint8_t(this->WorkMode));

    return res;
}

std::vector<uint8_t> CloseFiscalisationRequest::to_bytes() const {
    auto res = utils::time_to_bytes(this->DateTime);

    auto num = std::vector<uint8_t>(this->KKTNumber_cp866.begin(), this->KKTNumber_cp866.end());
    while (num.size() < 20) {
        num.push_back(0x00);
    }
    res.insert(res.end(), num.begin(), num.begin() + 20);

    return res;
}

std::vector<uint8_t> StartOpeningShiftRequest::to_bytes() const {
    return utils::time_to_bytes(this->DateTime);
}

std::vector<uint8_t> StartCloseShiftRequest::to_bytes() const {
    return utils::time_to_bytes(this->DateTime);
}

std::vector<uint8_t> StartCheckRequest::to_bytes() const {
    return utils::time_to_bytes(this->DateTime);
}

std::vector<uint8_t> CreateCheckRequest::to_bytes() const {
    auto res = utils::time_to_bytes(this->DateTime);
    res.push_back(uint8_t(this->OperationType));

    auto total = utils::split_le(this->Total);
    res.insert(res.end(), total.begin(), total.begin() + 5);

    return res;
}

std::vector<uint8_t> StartCheckCorrectionRequest::to_bytes() const {
    return utils::time_to_bytes(this->DateTime);
}

TLVList CommonData::to_tlv_list() const {
    TLVList list;

    if (this->UserName.size() != 0) {
        auto user_name = std::vector<uint8_t>(this->UserName.begin(), this->UserName.end());
        list.push(std::move(TLV(0x0418, user_name)));
    }

    if (this->Cashier.size() != 0) {
        auto cashier = std::vector<uint8_t>(this->Cashier.begin(), this->Cashier.end());
        list.push(std::move(TLV(0x03FD, cashier)));
    }

    if (this->Address.size() != 0) {
        auto address = std::vector<uint8_t>(this->Address.begin(), this->Address.end());
        list.push(std::move(TLV(0x03F1, address)));
    }

    if (this->InnOFD.size() != 0) {
        auto inn_ofd = std::vector<uint8_t>(this->InnOFD.begin(), this->InnOFD.end());
        list.push(std::move(TLV(0x03F9, inn_ofd)));
    }

    return list;
}

void CheckDocument::init(const std::vector<uint8_t>& data) {
    if (data.size() != 19) {
        return;
    }
    DateTime = utils::date_time_from_bytes(std::vector<uint8_t>(data.begin(), data.begin() + 5));
    Number = utils::union_bytes(data[5], data[6], data[7], data[8]);
    FiscSign = utils::union_bytes(data[9], data[10], data[11], data[12]);
    OpType = OperationType(data[13]);
    Sum = utils::union_bytes(data[14], data[15], data[16], data[17], data[18], 0, 0, 0);
}

void CloseFiscDocumnet::init(const std::vector<uint8_t>& data) {
    if (data.size() != 45) {
        return;
    }
    DateTime = utils::date_time_from_bytes(std::vector<uint8_t>(data.begin(), data.begin() + 5));
    Number = utils::union_bytes(data[5], data[6], data[7], data[8]);
    FiscSign = utils::union_bytes(data[9], data[10], data[11], data[12]);
    Inn = std::string(data.begin() + 13, data.begin() + 25);
    KTTNumber = std::string(data.begin() + 25, data.begin() + 45);
}

void ShiftDocument::init(const std::vector<uint8_t>& data) {
    if (data.size() != 15) {
        return;
    }
    DateTime = utils::date_time_from_bytes(std::vector<uint8_t>(data.begin(), data.begin() + 5));
    Number = utils::union_bytes(data[5], data[6], data[7], data[8]);
    FiscSign = utils::union_bytes(data[9], data[10], data[11], data[12]);
    ShiftNum = utils::union_bytes(data[13], data[14]);
}

void RegistrationDocument::init(const std::vector<uint8_t>& data) {
    if (data.size() != 47) {
        return;
    }
    DateTime = utils::date_time_from_bytes(std::vector<uint8_t>(data.begin(), data.begin() + 5));
    Number = utils::union_bytes(data[5], data[6], data[7], data[8]);
    FiscSign = utils::union_bytes(data[9], data[10], data[11], data[12]);
    Inn = std::string(data.begin() + 13, data.begin() + 25);
    KTTNumber = std::string(data.begin() + 25, data.begin() + 45);
    NCode = NalogCode(data[45]);
    WMode = WorkMode(data[46]);
}