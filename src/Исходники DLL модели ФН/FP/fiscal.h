// Leonid Moguchev (c) 2020
#pragma once

#include "pch.h"
#include <queue>
#include <map>
#include <array>
#include <list>

#include "logger.h"

const uint8_t MSG_START = 0x04; // Признак начала сообщения

const uint8_t CODE_OK = 0x00; // Успешное выполнение команды
const uint8_t ERROR_CODE_BAD_REQUEST = 0x01; // Неизвестная команда, неверный формат посылки или неизвестные параметры
const uint8_t ERROR_CODE_WRONG_STATE = 0x02; // Неверное состояние ФН 
const uint8_t ERROR_CODE_FN_ERROR = 0x03; // Ошибка ФН
const uint8_t ERROR_CODE_KC_ERROR = 0x04; // Ошибка КС
const uint8_t ERROR_CODE_FN_OUTDATE = 0x05; // Закончен срок эксплуатации ФН
const uint8_t ERROR_CODE_ARCHIVE_FULL = 0x06; // Архив ФН переполнен
const uint8_t ERROR_CODE_WRONG_DATE_TIME = 0x07; // Неверные дата и/или время
const uint8_t ERROR_CODE_NOT_EXISTS = 0x08; // Нет запрошенных данных
const uint8_t ERROR_CODE_WRONG_PARAMS = 0x09; // Некорректное значение параметров команды
const uint8_t ERROR_CODE_OVERFLOW_TVL = 0x10; // Превышение размеров TLV данных
const uint8_t ERROR_CODE_NO_CONNECT = 0x11; // Нет транспортного соединения
const uint8_t ERROR_CODE_KC_BROKEN = 0x12; // Исчерпан ресурс КС (криптографического сопроцессора)
const uint8_t ERROR_CODE_MEMORY_FULL = 0x14; // Исчерпан ресурс хранения
const uint8_t ERROR_CODE_MESSAGE_OUTDATE = 0x15; // Исчерпан ресурс Ожидания передачи сообщения
const uint8_t ERROR_CODE_SHIFT_MORE_24_H = 0x16; // Продолжительность смены более 24 часов
const uint8_t ERROR_CODE_WRONG_TIME_DIFF = 0x17; // Неверная разница во времени между 2 операциями
const uint8_t ERROR_CODE_OFD_ERROR = 0x20; // Сообщение от ОФД не может быть принято


const std::array<uint8_t, 16> FISC_NUMBER = { 57, 50, 56, 51, 52, 52, 48, 51, 48, 48, 48, 48, 48, 48, 48, 49 };
const std::array<uint8_t, 3> OUT_DATE = { 0x16, 0x01, 0x01 }; // ГГ.ММ.ДД 2022.01.01
const std::array<uint8_t, 16> SOFTWARE_VERSION = { 49, 46, 49, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; // 1.1
const uint8_t DEVELOP_VERSION = 0x00;
const uint8_t PRODUCT_VERSION = 0x01;

const uint32_t FISC_PRIZNAK = 0xFFFFFFFF;

enum class FiscState {
    SETTING = 0,
    READY_FOR_FISCAL = 1,
    FISCAL_REGIME = 3,
    FISCAL_REGIME_CLOSED = 7,
    READ_ARCHIVE = 15,
};

enum class FiscDocumentType {
    REGISTRATION = 1,
    OPEN_SHIFT = 2,
    CHECK = 3,
    CLOSE_SHIFT = 5,
    CLOSE_FISCAL_REGIME = 6,
};

struct TLVObject {
    uint16_t Tag;
    uint16_t Length;
    std::vector<uint8_t> Data;
};

struct FiscDocument {
    FiscDocumentType Type;
    // Common fields
    std::vector<uint8_t> DateTime; // 5
    uint32_t Number;
    uint32_t FiscPriznak;
    // Отчет о регистрации ККТ
    std::vector<uint8_t> Inn; // 12
    std::vector<uint8_t> KKTNumber; // 20
    uint8_t NalogCode;
    uint8_t WorkMode;
    // Кассовый чек
    uint8_t TypeOperation;
    std::vector<uint8_t> SumOperation; // 5
    // Открытие/закрытие смены
    uint16_t ChangeNum;

    bool GetOFDReceipt;
    std::vector<uint8_t> to_bytes();
};

class Fiscal {
public:
    Fiscal(std::queue<uint8_t>* in, std::queue<uint8_t>* out, Logger* log);
    ~Fiscal() = default;
    void process_byte();
private:
  
    std::queue<uint8_t>* _in;
    std::queue<uint8_t>* _out;
    Logger* _log;

    FiscState _state;
    uint32_t _fiscal_doc_number;
    uint16_t _change_num;
    bool _is_change_open;
    uint16_t _check_ammount;
    bool _is_check_open;

    std::map<uint32_t, FiscDocument> _storage;

    std::vector<uint8_t> _user;
    std::vector<uint8_t> _cashir;
    std::vector<uint8_t> _address;
    std::vector<uint8_t> _inn_ofd;
    std::vector<uint8_t> _curr_date_time;
    std::vector<uint8_t> _inn;
    std::vector<uint8_t> _kkt_number;
    std::vector<uint8_t> _el;
    std::vector<uint8_t> _nal;

    uint8_t _work_mod;
    uint8_t _code_nalog;

    bool _process_ficalisation;
    bool _process_ficalisation_done;
    bool _process_close_ficalisation;
    bool _process_close_ficalisation_done;
    bool _process_start_change;
    bool _process_start_change_done;
    bool _process_close_change;
    bool _process_close_change_done;
    bool _process_check;
    bool _process_check_done;
//////////////////////////////////////////////////////////////////////////////////////////
// OFD
//////////////////////////////////////////////////////////////////////////////////////////
    bool _transport_connection;
    bool _start_read_for_ofd;
    bool _wait_ansfer_from_ofd;
    std::list<FiscDocument> _to_ofd;
//////////////////////////////////////////////////////////////////////////////////////////
// READ BYTES
//////////////////////////////////////////////////////////////////////////////////////////
    bool _reading_command;           // чтение команды
    bool _read_cmd_done;
    uint16_t _read_bytes;            // прочитано байт
    uint16_t _msg_len;               // длина сообщения: LEN = 1+N (код команды + длина данных) 
    uint8_t _cmd_code;               // код команды: CMD
    std::vector<uint8_t> _msg;       // данные команды: LEN + CMD + DATA
    uint8_t _sum_bytes;
    uint16_t _control_sum;           // CRC16
//////////////////////////////////////////////////////////////////////////////////////////
    bool check_control_sum();
    std::map<uint16_t, TLVObject> get_tlv_objects_from_data(const std::vector<uint8_t>& data);

    void respond(uint8_t code, const std::vector<uint8_t>& data);
    void respond_with_error_code(uint8_t code);

    void process_cmd();

    void __30__handler();
    void __31__handler();
    void __32__handler();
    void __33__handler();
    void __35__handler();

    void __02__handler();
    void __03__handler();
    void __04__handler();
    void __05__handler();

    void __06__handler();
    void __07__handler();

    void __10__handler();
    void __11__handler();
    void __12__handler();
    void __13__handler();
    void __14__handler();
    void __15__handler();
    void __16__handler();
    void __17__handler();
    void __18__handler();
    void __19__handler();

    void __20__handler();
    void __21__handler();
    void __22__handler();
    void __23__handler();
    void __24__handler();
    void __25__handler();
    void __26__handler();

    void __40__handler();
    void __41__handler();
    void __42__handler();
    void __43__handler();
    void __44__handler();
    void __45__handler();
    void __46__handler();
    void __47__handler();

    void __60__handler();

    void process_start_fisc_document(const std::map<uint16_t, TLVObject>& objcects);
    void process_close_fisc_document(const std::map<uint16_t, TLVObject>& objcects);
    void process_start_change_document(const std::map<uint16_t, TLVObject>& objcects);
    void process_close_change_document(const std::map<uint16_t, TLVObject>& objcects);
    void process_check_document(const std::map<uint16_t, TLVObject>& objcects);
};

