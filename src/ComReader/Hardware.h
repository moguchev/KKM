// Leonid Moguchev (c) 2020
#pragma once
#include "ComChannel.h"
#include "FSParser.h"
#include "utils.h"
#include "CRC16.h"
#include "TLV.h"

#include <ctime>
#include <string>
#include <iostream>
#include <memory>
#include <map>

enum class NalogCode {
    COMMON = 1, // Общая
    SIMPLE_INCOME = 2, // Упрощенная Доход
    SIMPLE_INCOME_MINUS_EXPENSE = 4, // Упрощенная Доход минус Расход
    SINGLE_TAX_ON_IMPUTED_INCOME = 8, // Единый налог на вмененный доход
    UNIFIED_AGRICULTURAL_TAX = 16, // Единый сельскохозяйственный налог
    PATENT_TAXATION_SYSTEM = 32, // Патентная система налогообложения
};

enum class WorkMode {
    ENCRYPTION = 1, // Шифрование
    OFFLINE = 2, // Автономный режим
    AUTO = 4, // Автоматический режим
    SERVICE_APPLICATIONS = 8, // Применение в сфере услуг
    CHECK = 16, // Режим БСО (1) иначе Режим чеков (0)
    INTERNET_COMMERCE = 32, // Применение в Интернет-торговле
};

enum class OperationType {
    PARISH = 1, // Приход
    RETURN_PARISH = 2, // Возврат прихода
    CONSUMPTION = 3, // Расход
    RETURN_CONSUMPTION = 4, // Возврат расхода
};

enum class DocumentType {
    REGISTRATION = 1,
    OPEN_SHIFT = 2,
    CHECK = 3,
    CLOSE_SHIFT = 5,
    CLOSE_FISCAL_REGIME = 6,
};

static std::map<uint8_t, std::string> ERROR_TEXT = {
    {0x01, "Неизвестная команда, неверный формат посылки или неизвестные параметры"},
    {0x02, "Неверное состояние ФН"},
    {0x03, "Ошибка ФН"},
    {0x04, "Ошибка КС "},
    {0x05, "Закончен срок эксплуатации ФН"},
    {0x06, "Архив ФН переполнен"},
    {0x07, "Неверные дата и/или время"},
    {0x08, "Нет запрошенных данных"},
    {0x09, "Некорректное значение параметров команды"},
    {0x10, "Превышение размеров TLV данных"},
    {0x11, "Нет транспортного соединения"},
    {0x12, "Исчерпан ресурс КС (криптографического сопроцессора)"},
    {0x14, "Исчерпан ресурс хранения "},
    {0x20, "Сообщение от ОФД не может быть принято"},
};

static std::map<uint8_t, std::string> FN_LIFE_PHASES = {
    {0, "Настройка"},
    {1, "Готовность к фискализации"},
    {3, "Фискальный режим"},
    {7, "Фискальный режим закрыт, идет передача ФД в ОФД"},
    {15, "Чтение данных из Архива ФН"},
};

static std::map<uint8_t, std::string> CURRENT_DOCUMENT_TYPES = {
    {0x00, "нет открытого документа"},
    {0x01, "отчёт о фискализации"},
    {0x02, "отчёт об открытии смены"},
    {0x04, "кассовый чек"},
    {0x08, "отчёт о закрытии смены"},
    {0x10, "отчёт о закрытии фискального режима"},
};

static std::map<uint8_t, std::string> WARNING_FLAGS = {
    {0, ""},
    {1, "Срочная замена КС (до окончания срока действия 3 дня)"},
    {2, "Исчерпание ресурса КС (до окончания срока действия 30 дней)"},
    {4, "Переполнение памяти ФН (Архив ФН заполнен на 90 %)"},
    {8, "Превышено время ожидания ответа ОФД"},
};

struct Message {
    uint16_t Length;
    uint8_t Code;
    std::vector<uint8_t> Data;
    uint16_t Crc;
};

struct IRequest {
    time_t DateTime;

    virtual std::vector<uint8_t> to_bytes() const = 0;
    virtual ~IRequest() = default;
};

struct Response {
    std::string ErrorMsg;
};

struct GetFNStatusResponse : public Response {
    std::string PhaseOfLife;  // Фаза жизни
    std::string CurrentDocument;
    bool DocumentDataRecived;
    bool ShiftIsOpen;         // Состояние смены
    std::string Warnings;     // Флаги предупреждения
    std::string DateTime;
    std::string Number_cp866; // Номер ФН
    uint32_t LastFDNumber;    // Номер последнего ФД 
};

struct GetFNNumberResponse : public Response {
    std::string Number_cp866;
};

struct GetFNEndDateResponse : public Response {
    std::string Date;
    uint8_t LeftRegistrations;
    uint8_t DoneRegistrations;
};

struct GetFNVersionResponse : public Response {
    std::string VersionSoftWare_crc866;
    std::string TypeSoftWare;
};

struct StartFiscalisationResponse : public Response {};

struct SendDocumentsResponse : public Response {};

struct CancelDocumentResponse : public Response {};

struct StartCloseFiscalisationResponse : public Response {};

struct FiscData {
    uint32_t FiscDocNumber;
    uint32_t FiscSign;
};

struct ApproveFiscalisationResponse : public Response, public FiscData {};

struct CloseFiscalisationResponse : public Response, public FiscData {};

struct StartOpeningShiftResponse : public Response {};

struct ApproveOpeningShiftResponse : public Response, public FiscData {
    uint16_t ShiftNum;
};

struct GetShiftStatusResponse : public Response {
    bool ShiftOpen;
    uint16_t ShiftNum;
    uint16_t CheckAmmount;
};

struct StartCheckResponse : public Response {};

struct StartCheckCorrectionResponse : public Response {};

struct StartCloseShiftResponse : public Response {};

struct ApproveCloseShiftResponse : public Response, public FiscData {
    uint16_t ShiftNum;
};

struct CreateCheckResponse : public Response, public FiscData {
    uint16_t CheckNum;
};

struct IFiscalDocument {
    std::string DateTime;
    uint32_t Number;
    uint32_t FiscSign;
    virtual void init(const std::vector<uint8_t>& data) = 0;
    virtual ~IFiscalDocument() = default;
};

struct GetFiscDocumentResponse : public  Response {
    DocumentType DocumentType;
    bool GetOFDReceipt;
    IFiscalDocument* Document;
};

struct GetFDAmmountWithoutOFDResponse : public Response {
    uint16_t Ammount;
};

struct GetOFDStatusResponse : public  Response {
    std::vector<std::string> Status;
    bool StartRead;
    uint16_t MsgAmmount;
    uint32_t FirstDocNum;
    std::string FirstDocDateTime;
};

struct SetTransportStatusResponse : public  Response {};

struct StartReadMessageResponse : public  Response{
    uint16_t MsgLen;
};

struct ReadBlockResponse : public Response {
    std::vector<uint8_t> Message;
};

struct CancelReadMessageResponse : public  Response {};

struct EndReadMessageResponse : public Response {};

struct SendOFDAnswerResponse : public Response {
    uint8_t Code;
};

struct SendOFDAnswerRequest {
    std::vector<uint8_t> Answer;
};

struct RegistrationDocument : public IFiscalDocument {
    std::string Inn;
    std::string KTTNumber;
    NalogCode NCode;
    WorkMode WMode;

    virtual void init(const std::vector<uint8_t>& data);
    virtual ~RegistrationDocument() = default;
};

struct ShiftDocument : public IFiscalDocument {
    uint16_t ShiftNum;

    virtual void init(const std::vector<uint8_t>& data);
    virtual ~ShiftDocument() = default;
};

struct CheckDocument : public IFiscalDocument {
    OperationType OpType;
    uint64_t Sum;

    virtual void init(const std::vector<uint8_t>& data);
    virtual ~CheckDocument() = default;
};

struct CloseFiscDocumnet : public IFiscalDocument {
    std::string Inn;
    std::string KTTNumber;
  
    virtual void init(const std::vector<uint8_t>& data);
    virtual ~CloseFiscDocumnet() = default;
};

struct ApproveFiscalisationRequest : public IRequest {
    std::string Inn_cp866;
    std::string KKTNumber_cp866;
    NalogCode NalogCode;
    WorkMode WorkMode;

    virtual std::vector<uint8_t> to_bytes() const;
};

struct CloseFiscalisationRequest : public IRequest {
    std::string KKTNumber_cp866;

    virtual std::vector<uint8_t> to_bytes() const;
};

struct StartOpeningShiftRequest : public IRequest {
    virtual std::vector<uint8_t> to_bytes() const;
};

struct StartCloseShiftRequest : public IRequest {
    virtual std::vector<uint8_t> to_bytes() const;
};

struct StartCheckRequest : public IRequest {
    virtual std::vector<uint8_t> to_bytes() const;
};

struct StartCheckCorrectionRequest : public IRequest {
    virtual std::vector<uint8_t> to_bytes() const;
};

struct CreateCheckRequest : public IRequest {
    OperationType OperationType;
    uint64_t Total;

    virtual std::vector<uint8_t> to_bytes() const;
};

struct CommonData {
    std::string UserName;
    std::string Cashier;
    std::string Address;
    std::string InnOFD;
    
    TLVList to_tlv_list() const;
};

class Hardware {
public:
    Hardware(const std::wstring& port_name, uint32_t baud);
    ~Hardware() = default;

    bool get_connection_status();

    std::shared_ptr<StartFiscalisationResponse> __02__StartFiscalisation();
    std::shared_ptr<ApproveFiscalisationResponse> __03__ApproveFiscalisation(const ApproveFiscalisationRequest& req);
    std::shared_ptr<StartCloseFiscalisationResponse> __04__StartCloseFiscalisation();
    std::shared_ptr<CloseFiscalisationResponse> __05__CloseFiscalisation(const CloseFiscalisationRequest& req);

    std::shared_ptr<CancelDocumentResponse> __06__CancelDocuments();
    std::shared_ptr<SendDocumentsResponse> __07__SendDocuments(const TLVList& list);

    std::shared_ptr<GetShiftStatusResponse> __10__GetShiftStatus();
    std::shared_ptr<StartOpeningShiftResponse> __11__StartOpeningShift(const StartOpeningShiftRequest& req);
    std::shared_ptr<ApproveOpeningShiftResponse> __12__ApproveOpeningShift();
    std::shared_ptr<StartCloseShiftResponse> __13__StartCloseShift(const StartCloseShiftRequest& req);
    std::shared_ptr<ApproveCloseShiftResponse> __14__ApproveCloseShift();
    std::shared_ptr<StartCheckResponse> __15__StartCheck(const StartCheckRequest& req);
    std::shared_ptr<CreateCheckResponse> __16__CreateCheck(const CreateCheckRequest& req);
    std::shared_ptr<StartCheckCorrectionResponse> __17__StartCheckCorrection(const StartCheckCorrectionRequest& req);
    // ОФД
    std::shared_ptr<GetOFDStatusResponse> __20__GetOFDStatus();
    std::shared_ptr<SetTransportStatusResponse> __21__SetTransportStatus(bool connected);
    std::shared_ptr<StartReadMessageResponse> __22__StartReadMessage();
    std::shared_ptr<ReadBlockResponse> __23__ReadBlock(uint16_t offset, uint16_t limit);
    std::shared_ptr<CancelReadMessageResponse> __24__CancelReadMessage();
    std::shared_ptr<EndReadMessageResponse> __25__EndReadMessage();
    std::shared_ptr<SendOFDAnswerResponse> __26__SendOFDAnswer(const SendOFDAnswerRequest& req);
    // Общие команды
    std::shared_ptr<GetFNStatusResponse> __30__GetFNStatus();
    std::shared_ptr<GetFNNumberResponse> __31__GetFNNumber();
    std::shared_ptr<GetFNEndDateResponse> __32__GetFNEndDate();
    std::shared_ptr<GetFNVersionResponse> __33__GetFNVersion();
    // Работа с архивом ФН
    std::shared_ptr<GetFiscDocumentResponse> __40__GetFiscDocument(uint32_t num);
    std::shared_ptr<GetFDAmmountWithoutOFDResponse> __42__GetFDAmmountWithoutOFD();
private:
    const uint8_t MSG_START = 0x04;
    const uint8_t STATUS_OK = 0x00;
    const uint8_t INTERNAL_ERROR = 0x03;
    const std::string CRC_ERROR = "Ошибка контрольной суммы";
    const size_t TLV_LIMIT = 1024;

    std::unique_ptr<ComChannel> _com_io;
    bool _connection_success;

    FSParser _parser;

    std::shared_ptr<Message> parse_bytes(std::vector<uint8_t>&& bytes);

    bool check_crc(const Message& msg);
};