// Leonid Moguchev (c) 2020
#include "pch.h"
#include "fiscal.h"
#include "CRC16.h"

#include <map>
#include <iterator>
#include <cstdlib>

uint16_t union_bytes(uint8_t l, uint8_t h) {
    uint16_t res = (uint16_t(l) & 0x00FF);
    res |= ((uint16_t(h) & 0x00FF) << 8);
    return res;
}

std::vector<uint8_t> split(uint32_t i) {
    std::vector<uint8_t> bytes = {
        uint8_t(i & 0x000000FF),
        uint8_t((i & 0x0000FF00) >> 8),
        uint8_t((i & 0x00FF0000) >> 16),
        uint8_t((i & 0xFF000000) >> 24),
    };

    return bytes;
}

std::vector<uint8_t> split(uint16_t i) {
    std::vector<uint8_t> bytes = {
        uint8_t(i & 0x00FF),
        uint8_t((i & 0xFF00) >> 8),
    };

    return bytes;
}

uint32_t join(const std::vector<uint8_t>& bytes) {
    if (bytes.size() != 4) {
        return 0;
    }
    return (uint32_t(bytes[0]) | (uint32_t(bytes[1]) << 8) | (uint32_t(bytes[2]) << 16) | (uint32_t(bytes[3]) << 24));
}

std::vector<uint8_t> FiscDocument::to_bytes() {
    auto res = DateTime;
    auto num = split(Number);
    res.insert(res.end(), num.begin(), num.end());
    auto p = split(FiscPriznak);
    res.insert(res.end(), p.begin(), p.end());

    std::vector<uint8_t> tmp;
    switch (Type) {
    case FiscDocumentType::CHECK:
        res.push_back(TypeOperation);
        res.insert(res.end(), SumOperation.begin(), SumOperation.begin() + 5);
        break;
    case FiscDocumentType::OPEN_SHIFT:
        tmp = split(ChangeNum);
        res.insert(res.end(), tmp.begin(), tmp.end());
        break;
    case FiscDocumentType::CLOSE_SHIFT:
        tmp = split(ChangeNum);
        res.insert(res.end(), tmp.begin(), tmp.end());
        break;
    case FiscDocumentType::CLOSE_FISCAL_REGIME:
        res.insert(res.end(), Inn.begin(), Inn.begin() + 12);
        res.insert(res.end(), KKTNumber.begin(), KKTNumber.begin() + 20);
        break;
    case FiscDocumentType::REGISTRATION:
        res.insert(res.end(), Inn.begin(), Inn.begin() + 12);
        res.insert(res.end(), KKTNumber.begin(), KKTNumber.begin() + 20);
        res.push_back(NalogCode);
        res.push_back(WorkMode);
        break;
    }
    
    return res;
}

// data - list of TLV objects
std::map<uint16_t, TLVObject> Fiscal::get_tlv_objects_from_data(const std::vector<uint8_t>& data) {
    std::map<uint16_t, TLVObject> map;
    if (data.size() < 4) {
        return map;
    }

    for (size_t i = 0; i < data.size(); ) {
        auto obj = TLVObject{};
        auto data_begin = i + 4;

        if (data_begin <= data.size()) {
            uint8_t ltag = data[i];
            uint8_t htag = data[i + 1];
            uint8_t llen = data[i + 2];
            uint8_t hlen = data[i + 3];

            uint16_t tag = union_bytes(ltag, htag);
            uint16_t len = union_bytes(llen, hlen);

            obj.Tag = tag;
            obj.Length = len;
            if (len > 0) {
                auto data_end = data_begin + len;
                if (data.begin() + data_begin < data.end() && data.begin() + data_end <= data.end()) {
                    obj.Data = std::vector<uint8_t>(data.begin() + data_begin, data.begin() + data_end);
                }
            }
            map.emplace(std::pair<uint16_t, TLVObject>(tag, obj));

            i += (4 + len);
        } else {
            break;
        }
    }
    return map;
}

Fiscal::Fiscal(std::queue<uint8_t>* in, std::queue<uint8_t>* out, Logger* log) 
    : _in(in), _out(out), _log(log), _state(FiscState::READY_FOR_FISCAL), 
    _reading_command(false), _read_cmd_done(false), _fiscal_doc_number(0),
    _read_bytes(0), _msg_len(0), _cmd_code(0), _sum_bytes(0), _control_sum(0), 
    _transport_connection(false) {
    _curr_date_time = {0x15, 0x01, 0x01, 0x00, 0x00};
};

bool Fiscal::check_control_sum() {
    uint16_t sum = crc16_ccitt(_msg.data(), _msg.size());
    _log->log(string_format("IN SUM=%i, REAL SUM=%i", _control_sum, sum));
    return sum == _control_sum;
}

void Fiscal::respond(uint8_t code, const std::vector<uint8_t>& data) {
    _out->push(MSG_START);
    std::vector<uint8_t> tmp;

    uint16_t len = data.size() + 1;

    uint16_t llen = len & 0x00FF;
    uint16_t hlen = (len & 0xFF00) >> 8;
    tmp.push_back(uint8_t(llen));
    tmp.push_back(uint8_t(hlen));
    tmp.push_back(code);
    if (data.size()) {
        tmp.insert(tmp.end(), data.begin(), data.end());
    }
    uint16_t sum = crc16_ccitt(tmp.data(), tmp.size());
    uint16_t lsum = sum & 0x00FF;
    uint16_t hsum = (sum & 0xFF00) >> 8;

    for (auto& byte : tmp) {
        _out->push(byte);
    }

    _out->push(uint8_t(lsum));
    _out->push(uint8_t(hsum));
};

void Fiscal::respond_with_error_code(uint8_t code) {
    respond(code, std::vector<uint8_t>());
}

void Fiscal::process_cmd() {
    if (!check_control_sum()) {
        _log->log("BAD CONTROL SUM");
        respond_with_error_code(ERROR_CODE_FN_ERROR);
        return;
    } else {
        _log->log("GOOD CONTROL SUM");
    }

    switch (_cmd_code) {
////////////////////////////////////////////////////////
// Служебные команды ФН
////////////////////////////////////////////////////////
    case 0x30: // запрос статуса ФН
        __30__handler();
        break;
    case 0x31: // запрос номера ФН
        __31__handler();
        break;
    case 0x32: // запрос срока действия ФН
        __32__handler();
        break;
    case 0x33: // запрос версии ФН
        __33__handler();
        break;
    case 0x35: // запрос последних ошибок ФН
        __35__handler();
        break;
////////////////////////////////////////////////////////
// Общие команды для формирования фискальных документов
////////////////////////////////////////////////////////
    case 0x06: // отменить документ
        __06__handler();
        break;
    case 0x07: // передать данные документа
        __07__handler();
        break;
////////////////////////////////////////////////////////
// Команды фискализации и завершения фискального режима
////////////////////////////////////////////////////////
    case 0x02: // начать фискализацию ФН
        __02__handler();
        break;
    case 0x03: // фискализация ФН
        __03__handler();
        break;
    case 0x04: // начать закрытие фискального режима ФН
        __04__handler();
        break;
    case 0x05: // закрыть фискальный режим ФН
        __05__handler();
        break;
////////////////////////////////////////////////////////
// Команды формирования фискальных документов о расчётах
////////////////////////////////////////////////////////
    case 0x10: // запрос параметров текущей смены
        __10__handler();
        break;
    case 0x11: // начать открытие смены
        __11__handler();
        break;
    case 0x12: // открыть смену
        __12__handler();
        break;
    case 0x13: // начать закрытие смены
        __13__handler();
        break;
    case 0x14: // закрыть смену
        __14__handler();
        break; 
    case 0x15: // начать формирование чека
        __15__handler();
        break;
    case 0x16: // сформировать чек
        __16__handler();
        break;
    case 0x17: // начать формирование чека коррекции
        __17__handler();
        break;
    case 0x18: // начать формирование отчета о состоянии расчетов
        __18__handler();
        break;
    case 0x19: // сформировать отчет о состоянии расчетов
        __19__handler();
        break;
////////////////////////////////////////////////////////
// Команды информационного обена с ОФД
////////////////////////////////////////////////////////
    case 0x20: // получить статус информационного обмена
        __20__handler();
        break;
    case 0x21: // передать статус транспортного соединения с ОФД
        __21__handler();
        break;
    case 0x22: // начать чтение сообщения для ОФД
        __22__handler();
        break;
    case 0x23: // прочитать блок сообщения для ОФД
        __23__handler();
        break;
    case 0x24: // отменить блок сообщения для ОФД
        __24__handler();
        break;
    case 0x25: // завершить чтение сообщения для ОФД
        __25__handler();
        break;
    case 0x26: // передать квитанцию от ОФД
        __26__handler();
        break;
////////////////////////////////////////////////////////
// Команды получения данных из архива ФН
////////////////////////////////////////////////////////
    case 0x40: // найти фискальный документ по номеру
        __40__handler();
        break;
    case 0x41: // запрос квитанции о получении фискального документа в ОФД по номеру документа
        __41__handler();
        break;
    case 0x42: // запрос количества ФД, на которые нет квитанции
        __42__handler();
        break;
    case 0x43: // запрос итогов фискализации ФН
        __43__handler();
        break;
    case 0x44: // запрос параметра фискализации ФН
        __44__handler();
        break;
    case 0x45: // запрос фискального документа в TLV формате 
        __45__handler();
        break;
    case 0x46: // чтение TLV фискального документа
        __46__handler();
        break;
    case 0x47: // чтение TLV параметров фискализации
        __47__handler();
        break;
////////////////////////////////////////////////////////
// Отладочные программы
////////////////////////////////////////////////////////
    case 0x60: // сброс состояния ФН
        __60__handler(); 
        break;
    default:
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        break;
    }
}

void Fiscal::__30__handler() {
    _log->log("GET FN STATUS");

    auto response = std::vector<uint8_t>();
    response.push_back(uint8_t(_state));

    uint8_t curr_doc = 0;
    if (_process_check) {
        curr_doc = 0x04;
    }
    else if (_process_close_change) {
        curr_doc = 0x08;
    }
    else if (_process_close_ficalisation) {
        curr_doc = 0x10;
    }
    else if (_process_start_change) {
        curr_doc = 0x02;
    }
    else if (_process_ficalisation) {
        curr_doc = 0x01;
    }
    else {
        curr_doc = 0x00;
    }
    response.push_back(curr_doc);

    if (_process_check_done || _process_close_change_done || _process_close_ficalisation_done ||
        _process_start_change_done || _process_ficalisation_done) {
        response.push_back(0x01);
    }
    else {
        response.push_back(0x00);
    }
    response.push_back(uint8_t(_is_change_open));
    response.push_back(0x00);
    response.insert(response.end(), _curr_date_time.begin(), _curr_date_time.end());
    response.insert(response.end(), FISC_NUMBER.begin(), FISC_NUMBER.end());
    auto num_last_fd = split(_fiscal_doc_number);
    response.insert(response.end(), num_last_fd.begin(), num_last_fd.end());

    respond(CODE_OK, response);
}

void Fiscal::__31__handler() {
    _log->log("GET FN NUMBER HANDLER");

    respond(CODE_OK, std::vector<uint8_t>(FISC_NUMBER.begin(), FISC_NUMBER.end()));
};

void Fiscal::__32__handler() {
    _log->log("GET OUTDATE HANDLER");
    auto res = std::vector<uint8_t>(OUT_DATE.begin(), OUT_DATE.end());
    // Оставшееся количество возможности сделать отчет о Регистрации (перерегистрации) ККТ
    res.push_back(2);
    // Кол-во уже сделанных отчетов о регистрации (перерегистрации) ККТ
    if (_state < FiscState::FISCAL_REGIME) {
        res.push_back(0);
    } else {
        res.push_back(1);
    }

    respond(CODE_OK, res);
}

void Fiscal::__33__handler() {
    _log->log("GET FN VERSION HANDLER");
    auto res = std::vector<uint8_t>(SOFTWARE_VERSION.begin(), SOFTWARE_VERSION.end());
    res.push_back(DEVELOP_VERSION);

    respond(CODE_OK, res);
}

void Fiscal::__35__handler() {
    _log->log("GET LAST ERROR");
    respond(CODE_OK, std::vector<uint8_t>());
}

void Fiscal::__02__handler() {
    _log->log("START FISCALISATION");
    if (_state != FiscState::READY_FOR_FISCAL) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }
    _process_ficalisation = true;
    _process_ficalisation_done = false;

    respond(CODE_OK, std::vector<uint8_t>());
};

void Fiscal::__03__handler() {
    _log->log("FISCALISATION");
    if (_state > FiscState::READY_FOR_FISCAL || !_process_ficalisation_done) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }

    if (_msg.size() != 42) {
        _log->log("BAD INPUT DATA");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }

    _curr_date_time = std::vector<uint8_t>(_msg.begin() + 3, _msg.begin() + 8);
    _inn = std::vector<uint8_t>(_msg.begin() + 8, _msg.begin() + 20);
    _kkt_number = std::vector<uint8_t>(_msg.begin() + 20, _msg.begin() + 40);
    _code_nalog = _msg[_msg.size() - 2];
    _work_mod = _msg[_msg.size() - 1];

    std::srand(_curr_date_time[0] + 3);
    uint32_t fisc_sign = std::rand() * 100 + std::rand();

    auto doc = FiscDocument();
    doc.Type = FiscDocumentType::REGISTRATION;
    doc.DateTime = _curr_date_time;
    doc.Number = ++_fiscal_doc_number;
    doc.FiscPriznak = fisc_sign;
    doc.Inn = _inn;
    doc.KKTNumber = _kkt_number;
    doc.NalogCode = _code_nalog;
    doc.WorkMode = _work_mod;
    _storage.emplace(std::pair<uint32_t, FiscDocument>(_fiscal_doc_number, doc));

    _state = FiscState::FISCAL_REGIME;
    _process_ficalisation = false;
    _process_ficalisation_done = false;

    auto response = split(_fiscal_doc_number);
    auto priznak = split(fisc_sign);
    response.insert(response.end(), priznak.begin(), priznak.end());

    respond(CODE_OK, response);
};

void Fiscal::__04__handler() {
    _log->log("START CLOSE FISCALISATION");
    if (_state != FiscState::FISCAL_REGIME) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }
    _process_close_ficalisation = true;
    _process_close_ficalisation_done = false;

    respond(CODE_OK, std::vector<uint8_t>());
}

void Fiscal::__05__handler() {
    _log->log("CLOSE FISCALISATION");
    if (_state != FiscState::FISCAL_REGIME || !_process_close_ficalisation_done) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }

    if (_msg.size() != 28) {
        _log->log("BAD INPUT DATA");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }

    _curr_date_time = std::vector<uint8_t>(_msg.begin() + 3, _msg.begin() + 8);
    _kkt_number = std::vector<uint8_t>(_msg.begin() + 8, _msg.begin() + 28);

    std::srand(_curr_date_time[0] + 5);
    uint32_t fisc_sign = std::rand() * 10 + std::rand();

    auto doc = FiscDocument();
    doc.Type = FiscDocumentType::CLOSE_FISCAL_REGIME;
    doc.DateTime = _curr_date_time;
    doc.Number = ++_fiscal_doc_number;
    doc.FiscPriznak = fisc_sign;
    doc.Inn = _inn;
    doc.KKTNumber = _kkt_number;

    _storage.emplace(std::pair<uint32_t, FiscDocument>(_fiscal_doc_number, doc));

    _state = FiscState::FISCAL_REGIME_CLOSED;
    _process_close_ficalisation = false;
    _process_close_ficalisation_done = false;

    std::vector<uint8_t> response = split(_fiscal_doc_number);
    auto priznak = split(fisc_sign);
    response.insert(response.end(), priznak.begin(), priznak.end());

    respond(CODE_OK, response);
}

void Fiscal::__06__handler() {
    _process_ficalisation_done = false;
    _process_ficalisation = false;
    _process_close_ficalisation_done = false;
    _process_close_ficalisation = false;
    _process_start_change_done = false;
    _process_start_change = false;

    respond(CODE_OK, std::vector<uint8_t>());
}

void Fiscal::__07__handler() {
    if (_msg.size() > 1027) {
        _log->log("ERROR: BAD DATA LENGTH");
        respond_with_error_code(ERROR_CODE_OVERFLOW_TVL);
        return;
    }

    if (_msg.size() < 4) {
        _log->log("NO DATA IN DOCUMENT");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }

    auto tlv_objects = get_tlv_objects_from_data(std::vector<uint8_t>(_msg.begin() + 3, _msg.end()));

    if (_process_ficalisation) {
        _log->log("DOCUMENT: START FISCALISATION");
        process_start_fisc_document(tlv_objects);
        return;
    }

    if (_process_close_ficalisation) {
        _log->log("DOCUMENT: CLOSE FISCALISATION");
        process_close_fisc_document(tlv_objects);
        return;
    }
    
    if (_process_start_change) {
        _log->log("DOCUMENT: START CHANGE");
        process_start_change_document(tlv_objects);
        return;
    }

    if (_process_close_change) {
        _log->log("DOCUMENT: CLOSE CHANGE");
        process_close_change_document(tlv_objects);
        return;
    }

    if (_process_check) {
        _log->log("DOCUMENT: CHECK");
        process_check_document(tlv_objects);
        return;
    }

    respond_with_error_code(ERROR_CODE_WRONG_STATE);
};

void Fiscal::process_close_fisc_document(const std::map<uint16_t, TLVObject>& objcects) {
    auto it = objcects.find(0x03FD);
    if (it == objcects.end()) {
        _log->log("NO CASHIR IN TLV LIST");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }
    _cashir = it->second.Data;

    it = objcects.find(0x03F1);
    if (it == objcects.end()) {
        _log->log("NO ADDRESS IN TLV LIST");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }
    _address = it->second.Data;

    _process_close_ficalisation_done = true;  // всего 1 документ
    respond(CODE_OK, std::vector<uint8_t>());
}

void Fiscal::process_start_fisc_document(const std::map<uint16_t, TLVObject>& objcects) {
    auto it = objcects.find(0x0418);
    if (it == objcects.end()) {
        _log->log("NO USER IN TLV LIST");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }
    _user = it->second.Data;

    it = objcects.find(0x03FD);
    if (it == objcects.end()) {
        _log->log("NO CASHIR IN TLV LIST");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }
    _cashir = it->second.Data;

    it = objcects.find(0x03F1);
    if (it == objcects.end()) {
        _log->log("NO ADDRESS IN TLV LIST");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }
    _address = it->second.Data;

    it = objcects.find(0x03F9);
    if (it == objcects.end()) {
        _log->log("NO INN OFD IN TLV LIST");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }
    _inn_ofd = it->second.Data;

    _process_ficalisation_done = true;
    respond(CODE_OK, std::vector<uint8_t>());
};

void Fiscal::process_start_change_document(const std::map<uint16_t, TLVObject>& objcects) {
    auto it = objcects.find(0x0418);
    if (it == objcects.end()) {
        _log->log("NO USER IN TLV LIST");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }
    _user = it->second.Data;

    it = objcects.find(0x03FD);
    if (it == objcects.end()) {
        _log->log("NO CASHIR IN TLV LIST");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }
    _cashir = it->second.Data;

    it = objcects.find(0x03F1);
    if (it == objcects.end()) {
        _log->log("NO ADDRESS IN TLV LIST");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }
    _address = it->second.Data;

    _process_start_change_done = true;
    respond(CODE_OK, std::vector<uint8_t>());
};

void Fiscal::process_close_change_document(const std::map<uint16_t, TLVObject>& objcects) {
    auto it = objcects.find(0x0418);
    if (it == objcects.end()) {
        _log->log("NO USER IN TLV LIST");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }
    _user = it->second.Data;

    it = objcects.find(0x03FD);
    if (it == objcects.end()) {
        _log->log("NO CASHIR IN TLV LIST");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }
    _cashir = it->second.Data;

    it = objcects.find(0x03F1);
    if (it == objcects.end()) {
        _log->log("NO ADDRESS IN TLV LIST");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }
    _address = it->second.Data;

    _process_close_change_done = true;
    respond(CODE_OK, std::vector<uint8_t>());
}

void Fiscal::process_check_document(const std::map<uint16_t, TLVObject>& objcects) {
    auto it = objcects.find(0x0418);
    if (it != objcects.end()) {
        _user = it->second.Data;
    }
   
    it = objcects.find(0x03FD);
    if (it != objcects.end()) {
        _cashir = it->second.Data;
    }
    
    it = objcects.find(0x03F1);
    if (it != objcects.end()) {
        _address = it->second.Data;
    }
    
    it = objcects.find(0x0407);
    if (it != objcects.end()) {
        _nal = it->second.Data;
    }

    it = objcects.find(0x0439);
    if (it != objcects.end()) {
        _el = it->second.Data;
    }

    _process_check_done = true;
    respond(CODE_OK, std::vector<uint8_t>());
}

void Fiscal::__10__handler() {
    _log->log("SHIFT STATUS");
    if (_state != FiscState::FISCAL_REGIME) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }

    std::vector<uint8_t> response = { 0x00 };
    if (_is_change_open) {
        response[0] = 0x01;
    }

    auto shift_num = split(_change_num);
    response.insert(response.end(), shift_num.begin(), shift_num.end());

    auto check_num = split(_check_ammount);
    response.insert(response.end(), check_num.begin(), check_num.end());

    respond(CODE_OK, response);
}

void Fiscal::__11__handler() {
    _log->log("START OPEN CHANGE");
    if (_state != FiscState::FISCAL_REGIME || _is_change_open) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }

    if (_msg.size() != 8) {
        _log->log("BAD INPUT DATA");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }

    _curr_date_time = std::vector<uint8_t>(_msg.begin() + 3, _msg.begin() + 8);

    _process_start_change = true;
    _process_start_change_done = false;

    respond(CODE_OK, std::vector<uint8_t>());
}

void Fiscal::__12__handler() {
    _log->log("OPEN SHIFT");
    if (_state != FiscState::FISCAL_REGIME || _is_change_open || !_process_start_change_done) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }
    _process_start_change_done = false;
    _process_start_change = false;
    _is_change_open = true;
    _check_ammount = 0;

    std::srand(_curr_date_time[0] + 12);
    uint32_t fisc_sign = std::rand() * 5 + std::rand() * 5;

    auto doc = FiscDocument();
    doc.Type = FiscDocumentType::OPEN_SHIFT;
    doc.DateTime = _curr_date_time;
    doc.Number = ++_fiscal_doc_number;
    doc.FiscPriznak = fisc_sign;
    doc.ChangeNum = ++_change_num;

    _storage.emplace(std::pair<uint32_t, FiscDocument>(_fiscal_doc_number, doc));

    std::vector<uint8_t> response = split(_change_num);
    auto doc_num = split(_fiscal_doc_number);
    response.insert(response.end(), doc_num.begin(), doc_num.end());
    auto priznak = split(fisc_sign);
    response.insert(response.end(), priznak.begin(), priznak.end());

    respond(CODE_OK, response);
}

void Fiscal::__13__handler() {
    _log->log("START CLOSE SHIFT");
    if (_state != FiscState::FISCAL_REGIME || !_is_change_open || _is_check_open) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }

    if (_msg.size() != 8) {
        _log->log("BAD INPUT DATA");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }

    _curr_date_time = std::vector<uint8_t>(_msg.begin() + 3, _msg.begin() + 8);
    _process_close_change = true;
    _process_close_change_done = false;

    respond(CODE_OK, std::vector<uint8_t>());
}

void Fiscal::__14__handler() {
    _log->log("CLOSE SHIFT");
    if (_state != FiscState::FISCAL_REGIME || !_is_change_open || !_process_close_change_done) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }
    _process_close_change_done = false;
    _process_close_change = false;
    _is_change_open = false;

    std::srand(_curr_date_time[0] + 14);
    uint32_t fisc_sign = std::rand() * 50 + std::rand() * 2;

    auto doc = FiscDocument();
    doc.Type = FiscDocumentType::CLOSE_SHIFT;
    doc.DateTime = _curr_date_time;
    doc.Number = ++_fiscal_doc_number;
    doc.FiscPriznak = fisc_sign;
    doc.ChangeNum = _change_num;

    _storage.emplace(std::pair<uint32_t, FiscDocument>(_fiscal_doc_number, doc));

    std::vector<uint8_t> response = split(_change_num);
    auto doc_num = split(_fiscal_doc_number);
    response.insert(response.end(), doc_num.begin(), doc_num.end());
    auto priznak = split(fisc_sign);
    response.insert(response.end(), priznak.begin(), priznak.end());

    respond(CODE_OK, response);
}

void Fiscal::__15__handler() {
    _log->log("START CHECK");
    if (_state != FiscState::FISCAL_REGIME || !_is_change_open || _is_check_open) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }

    if (_msg.size() != 8) {
        _log->log("BAD INPUT DATA");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }

    _curr_date_time = std::vector<uint8_t>(_msg.begin() + 3, _msg.begin() + 8);
    _is_check_open = true;
    _process_check = true;
    _process_check_done = false;

    respond(CODE_OK, std::vector<uint8_t>());
}

void Fiscal::__16__handler() {
    _log->log("CHECK");
    if (_state != FiscState::FISCAL_REGIME || !_is_change_open || !_process_check_done) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }

    if (_msg.size() < 14) {
        _log->log("BAD INPUT DATA");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }

    _curr_date_time = std::vector<uint8_t>(_msg.begin() + 3, _msg.begin() + 8);
    auto _op_type = _msg[8];
    auto _sum = std::vector<uint8_t>(_msg.begin() + 9, _msg.begin() + 14);

    _process_check_done = false;
    _process_check = false;
    _is_check_open = false;
    _check_ammount++;

    std::srand(_curr_date_time[0] + _fiscal_doc_number);
    uint32_t fisc_sign = std::rand() * 33 + std::rand() * 3;

    auto doc = FiscDocument();
    doc.Type = FiscDocumentType::CHECK;
    doc.DateTime = _curr_date_time;
    doc.Number = ++_fiscal_doc_number;
    doc.FiscPriznak = fisc_sign;
    doc.TypeOperation = _op_type;
    doc.SumOperation = _sum;
 
    _log->log(string_format("16: OPERATION_TYPE: %d", _op_type));

    _storage.emplace(std::pair<uint32_t, FiscDocument>(_fiscal_doc_number, doc));

    std::vector<uint8_t> response = split(_check_ammount);
    auto doc_num = split(_fiscal_doc_number);
    response.insert(response.end(), doc_num.begin(), doc_num.end());
    auto priznak = split(fisc_sign);
    response.insert(response.end(), priznak.begin(), priznak.end());

    respond(CODE_OK, response);
}

void Fiscal::__17__handler() {
    _log->log("START CHECK CORRECTION");
    if (_state != FiscState::FISCAL_REGIME || !_is_change_open || _is_check_open) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }

    if (_msg.size() != 8) {
        _log->log("BAD INPUT DATA");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }

    _curr_date_time = std::vector<uint8_t>(_msg.begin() + 3, _msg.begin() + 8);
    _is_check_open = true;
    _process_check = true;
    _process_check_done = false;

    respond(CODE_OK, std::vector<uint8_t>());
}

void Fiscal::__18__handler() {
    _log->log("NOT IMPLEMENTED!");
    respond(ERROR_CODE_FN_ERROR, std::vector<uint8_t>());
}

void Fiscal::__19__handler() {
    _log->log("NOT IMPLEMENTED!");
    respond(ERROR_CODE_FN_ERROR, std::vector<uint8_t>());
}

void Fiscal::__20__handler() {
    _log->log("GET OFD STATUS");
    auto resp = std::vector<uint8_t>();
    uint8_t status = 0x00;
    if (_transport_connection) {
        status |= 0x01;
    }
    uint16_t count = 0;
    uint16_t doc_ammount = 0;
    uint32_t doc_num = 0;
    bool first = false;
    std::vector<uint8_t> date_time;
    for (auto& doc : _storage) {
        if (!doc.second.GetOFDReceipt) {
            if (!first) {
                first = true;
                doc_num = doc.second.Number;
                date_time = doc.second.DateTime;
            }
            doc_ammount++;
            count += doc.second.to_bytes().size() + 1;
        }
    }
    if (count > 0) {
        status |= 0x02;
    }
    if (_wait_ansfer_from_ofd) {
        status |= 0x04;
    }
    resp.push_back(status);
    resp.push_back(uint8_t(_wait_ansfer_from_ofd));
    auto tmp1 = split(doc_ammount);
    resp.insert(resp.end(), tmp1.begin(), tmp1.end());
    auto tmp2 = split(doc_num);
    resp.insert(resp.end(), tmp2.begin(), tmp2.end());
    resp.insert(resp.end(), date_time.begin(), date_time.end());

    respond(CODE_OK, resp);
}

void Fiscal::__21__handler() {
    _log->log("SET TRANSPORT CONNECTION");
    if (_state != FiscState::FISCAL_REGIME) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }

    if (_msg.size() < 4) {
        _log->log("BAD INPUT DATA");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }

    _transport_connection = _msg[3] > 0;

    respond(CODE_OK, std::vector<uint8_t>());
}

void Fiscal::__22__handler() {
    _log->log("START READ MSG FOR OFD");
    if (_state != FiscState::FISCAL_REGIME) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }

    if (!_transport_connection) {
        _log->log("NO CONNECTION WITH OFD");
        respond_with_error_code(ERROR_CODE_NO_CONNECT);
        return;
    }

    uint16_t count = 0;
    for (auto& doc : _storage) {
        if (!doc.second.GetOFDReceipt) {
            _to_ofd.push_back(doc.second);
            count += doc.second.to_bytes().size() + 1;
        }
    }

    if (count == 0) {
        _log->log("NO NEW DOCUMENTS");
        respond_with_error_code(ERROR_CODE_NOT_EXISTS);
        return;
    }

    _start_read_for_ofd = true;

    respond(CODE_OK, split(count));
}

void Fiscal::__23__handler() {
    _log->log("READ MSG FOR OFD");
    if (_state != FiscState::FISCAL_REGIME || !_start_read_for_ofd) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }

    if (!_transport_connection) {
        _log->log("NO CONNECTION WITH OFD");
        respond_with_error_code(ERROR_CODE_NO_CONNECT);
        return;
    }

    if (_msg.size() < 7) {
        _log->log("BAD INPUT DATA");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }

    auto offset = union_bytes(_msg[3], _msg[4]);
    auto limit = union_bytes(_msg[5], _msg[6]);

    auto response = std::vector<uint8_t>();
    for (auto it = _to_ofd.begin(); it != _to_ofd.end(); ) {
        auto tmp = it->to_bytes();
        response.push_back(uint8_t(it->Type));
        response.insert(response.end(), tmp.begin(), tmp.end());
        _to_ofd.pop_front();
        it = _to_ofd.begin();
    }
    _wait_ansfer_from_ofd = true;
    respond(CODE_OK, response);
}

void Fiscal::__24__handler() {
    _log->log("CANCEL READ MSG FOR OFD");
    if (_state != FiscState::FISCAL_REGIME || !_start_read_for_ofd) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }
    _to_ofd.clear();
    _start_read_for_ofd = false;

    respond(CODE_OK, std::vector<uint8_t>());
}

void Fiscal::__25__handler() {
    _log->log("END READ MSG FOR OFD!");
    if (_state != FiscState::FISCAL_REGIME || !_start_read_for_ofd) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }
    _to_ofd.clear();
    _start_read_for_ofd = false;

    respond(CODE_OK, std::vector<uint8_t>());
}

void Fiscal::__26__handler() {
    _log->log("GET FROM OFD STATUS");

    if (_state != FiscState::FISCAL_REGIME) {
        _log->log("WRONG STATE");
        respond_with_error_code(ERROR_CODE_WRONG_STATE);
        return;
    }

    for (auto it = _storage.begin(); it != _storage.end(); it++) {
        it->second.GetOFDReceipt = true;
    }

    _wait_ansfer_from_ofd = false;
    respond(CODE_OK, std::vector<uint8_t>{0x00});
}

void Fiscal::__40__handler() {
    _log->log("FIND FISC DOCUMENT");
    if (_msg.size() != 7) {
        _log->log("BAD INPUT DATA");
        respond_with_error_code(ERROR_CODE_BAD_REQUEST);
        return;
    }

    uint32_t doc_num = join(std::vector<uint8_t>(_msg.begin() + 3, _msg.end()));
    
    auto it = _storage.find(doc_num);
    if (it == _storage.end()) {
        _log->log("DOCUMENT NOT EXISTS");
        respond_with_error_code(ERROR_CODE_NOT_EXISTS);
        return;
    }

    auto response = std::vector<uint8_t>();
    response.push_back(uint8_t(it->second.Type));
    response.push_back(uint8_t(it->second.GetOFDReceipt)); // получена ли квитанция из ОФД
    auto data = it->second.to_bytes();
    response.insert(response.end(), data.begin(), data.end());

    _log->log(string_format("40: OPERATION_TYPE: %d", data[13]));

    respond(CODE_OK, response);
}

void Fiscal::__41__handler() {
    _log->log("NOT IMPLEMENTED!");
    respond(ERROR_CODE_FN_ERROR, std::vector<uint8_t>());
}

void Fiscal::__42__handler() {
    _log->log("COUNT FD WITHOUT OFD RECEIPT");

    uint16_t count = 0;
    for (const auto& doc : _storage) {
        if (!doc.second.GetOFDReceipt) {
            count++;
        }
    }

    respond(CODE_OK, split(count));
}

void Fiscal::__43__handler() {
    _log->log("NOT IMPLEMENTED!");
    respond(ERROR_CODE_FN_ERROR, std::vector<uint8_t>());
}

void Fiscal::__44__handler() {
    _log->log("NOT IMPLEMENTED!");
    respond(ERROR_CODE_FN_ERROR, std::vector<uint8_t>());
}

void Fiscal::__45__handler() {
    _log->log("NOT IMPLEMENTED!");
    respond(ERROR_CODE_FN_ERROR, std::vector<uint8_t>());
}

void Fiscal::__46__handler() {
    _log->log("NOT IMPLEMENTED!");
    respond(ERROR_CODE_FN_ERROR, std::vector<uint8_t>());
}

void Fiscal::__47__handler() {
    _log->log("NOT IMPLEMENTED!");
    respond(ERROR_CODE_FN_ERROR, std::vector<uint8_t>());
}

void Fiscal::__60__handler() {
    _log->log("NOT IMPLEMENTED!");
    respond(ERROR_CODE_FN_ERROR, std::vector<uint8_t>());
}

void Fiscal::process_byte() {
    uint8_t byte = _in->front();
    _in->pop();
    
    if (byte == MSG_START && !_reading_command) {
        _reading_command = true;
        _read_cmd_done = false;
        _msg_len = 0x0000;
        _control_sum = 0x0000;
        _sum_bytes = 0;
        _msg.clear();
        _read_bytes = 1;
        _log->log("START_MSG");
        return;
    }

    if (_reading_command) {
        switch (_read_bytes) {
        case 1: // младший байт LEN
            _msg.push_back(byte);
            _msg_len = (uint16_t)byte & 0x00FF;
            _log->log("L LEN");
            break;
        case 2: // старший байт LEN
            _msg.push_back(byte);
            _msg_len |= (((uint16_t)byte & 0x00FF) << 8);
            _log->log("H LEN");
            _log->log(string_format("LEN=%i", _msg_len));
            break;
        case 3: // код команды CMD
            _msg.push_back(byte);
            _cmd_code = byte;
            _msg_len--;
            _log->log(string_format("CMD=%i", _cmd_code));
            break;
        default:
            //_log->log(string_format("PROCESS=%i", _msg_len));
            if (_msg_len > 0) { // DATA
                //_log->log("PUSH TO DATA");

                _msg.push_back(byte);
                _msg_len--;
            } else { // контрольная сумма
                if (_sum_bytes == 0) { // младший байт CRC
                    _log->log("L CRC");

                    _control_sum = (uint16_t)byte & 0x00FF;
                    _sum_bytes++;
                } else if (_sum_bytes == 1) {
                    _log->log("H CRC");

                    _control_sum |= (((uint16_t)byte & 0x00FF) << 8);
                    _sum_bytes++; // старший байт CRC
                    _reading_command = false;
                    _read_cmd_done = true;
                     
                    _log->log(string_format("CRC=%i", _control_sum));
                } else {
                    _log->log("WTF AFTER CONTROL SUM?");
                }
            }
            break;
        }
        _read_bytes++;
    }

    if (_read_cmd_done) {
        _log->log("PROCESS COMAND");
        process_cmd();
    }
}