// Leonid Moguchev (c) 2020

/////////////////////////////////////////////////////////////////
// REQUERIED!!!!!
#define _WINSOCKAPI_    // stops windows.h including winsock.h
#include <windows.h>
/////////////////////////////////////////////////////////////////

#include <iostream>
#include <memory>

#include "utils.h"
#include "Hardware.h"
#include "TLV.h"

#include "OFDClient.h"

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    // Создаем класс общения с Proteus
    auto service = std::make_shared<Hardware>(L"COM2", 10000);
    // Получаем статус соединения
    auto connected = service->get_connection_status();
    std::cout << "CONNECTION WITH FN: " << std::boolalpha << connected << std::endl;
    if (!connected) {
        return EXIT_FAILURE;
    }
    // Создаем класс общения с OFD сервером
    auto ofd_client = std::make_shared<OFDClient>(service);
    // Получаем статус соединения
    connected = ofd_client->Connect("127.0.0.1", "4321");
    std::cout << "CONNECTION WITH OFD: " << std::boolalpha << connected << std::endl;
    if (!connected) {
        return EXIT_FAILURE;
    }
/////////////////////////////////////////////////////////////////////////////
// 30
/////////////////////////////////////////////////////////////////////////////
    {
        auto response = service->__30__GetFNStatus();
        std::cout << "COMMAND: 30h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }

        std::cout << response->PhaseOfLife << std::endl;
        std::cout << response->CurrentDocument << std::endl;
        std::cout << std::boolalpha << response->DocumentDataRecived << std::endl;
        std::cout << std::boolalpha << response->ShiftIsOpen << std::endl;
        std::cout << response->Warnings << std::endl;
        std::cout << response->Number_cp866 << std::endl;
        std::cout << response->DateTime << std::endl;
        std::cout << response->LastFDNumber << std::endl;
    }  
/////////////////////////////////////////////////////////////////////////////
// 31
/////////////////////////////////////////////////////////////////////////////
    {
        auto response = service->__31__GetFNNumber();
        std::cout << "COMMAND: 31h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
        std::cout << response->Number_cp866 << std::endl;
    }
/////////////////////////////////////////////////////////////////////////////
// 32
/////////////////////////////////////////////////////////////////////////////
    {
        auto response = service->__32__GetFNEndDate();
        std::cout << "COMMAND: 32h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
        std::cout << response->Date << std::endl;
    }
/////////////////////////////////////////////////////////////////////////////
// 33
/////////////////////////////////////////////////////////////////////////////
    {
        auto response = service->__33__GetFNVersion();
        std::cout << "COMMAND: 33h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
        std::cout << response->VersionSoftWare_crc866 << std::endl;
        std::cout << response->TypeSoftWare << std::endl;
    }
/////////////////////////////////////////////////////////////////////////////
// 02 - Начать фискализацию (регистрацию ККТ)
/////////////////////////////////////////////////////////////////////////////
    {
        auto response = service->__02__StartFiscalisation();
        std::cout << "COMMAND: 02h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
    }
/////////////////////////////////////////////////////////////////////////////
// 07 - Передать необходимые документы
/////////////////////////////////////////////////////////////////////////////
    {
        auto doc = CommonData{};
        doc.UserName = "Иванов И.И.";
        doc.Cashier = "Петров П.П.";
        doc.Address = "ул.Тверская, д.1";
        doc.InnOFD = "790123456789";

        auto response = service->__07__SendDocuments(doc.to_tlv_list());
        std::cout << "COMMAND: 07h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
    }
/////////////////////////////////////////////////////////////////////////////
// 03 - Фискализация (регистрация ККТ)
/////////////////////////////////////////////////////////////////////////////
    {
        auto req = ApproveFiscalisationRequest{};
        req.DateTime = time(0);
        req.Inn_cp866 = "112233445566";
        req.KKTNumber_cp866 = "KKT-772-233-445-566";
        req.NalogCode = NalogCode::COMMON;
        req.WorkMode = WorkMode::AUTO;

        auto response = service->__03__ApproveFiscalisation(req);
        std::cout << "COMMAND: 03h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
        std::cout << response->FiscDocNumber << std::endl;
        std::cout << response->FiscSign << std::endl;
    }
/////////////////////////////////////////////////////////////////////////////
// 11 - Начать открытие смены
/////////////////////////////////////////////////////////////////////////////
    {
        auto req = StartOpeningShiftRequest{};
        req.DateTime = time(0);

        auto response = service->__11__StartOpeningShift(req);
        std::cout << "COMMAND: 11h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
    }
/////////////////////////////////////////////////////////////////////////////
// 07 - Передать необходимые документы
/////////////////////////////////////////////////////////////////////////////
    {
        auto doc = CommonData{};
        doc.UserName = "Иванов И.И.";
        doc.Cashier = "Петров П.П.";
        doc.Address = "ул.Тверская, д.1";

        auto response = service->__07__SendDocuments(doc.to_tlv_list());
        std::cout << "COMMAND: 07h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
    }
/////////////////////////////////////////////////////////////////////////////
// 12 - Открыть смену
/////////////////////////////////////////////////////////////////////////////
    {
        auto response = service->__12__ApproveOpeningShift();
        std::cout << "COMMAND: 12h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
        std::cout << response->ShiftNum << std::endl;
        std::cout << response->FiscDocNumber << std::endl;
        std::cout << response->FiscSign << std::endl;
    }
/////////////////////////////////////////////////////////////////////////////
// 10 - Запрос параметров текущей смены
/////////////////////////////////////////////////////////////////////////////
    {
        auto response = service->__10__GetShiftStatus();
        std::cout << "COMMAND: 10h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
        std::cout << response->ShiftNum << std::endl;
        std::cout << std::boolalpha << response->ShiftOpen << std::endl;
        std::cout << response->CheckAmmount << std::endl;
    }
/////////////////////////////////////////////////////////////////////////////
// 15 - Начать формирование чека
/////////////////////////////////////////////////////////////////////////////
    {
        auto req = StartCheckRequest{};
        req.DateTime = time(0);

        auto response = service->__15__StartCheck(req);
        std::cout << "COMMAND: 15h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
    }
/////////////////////////////////////////////////////////////////////////////
// 07 - Передать необходимые документы
/////////////////////////////////////////////////////////////////////////////
    {
        // ВООБЩЕ ПОХЕР ЧТО ОТПРАВЛЯЕТСЯ, ГЛАВНОЕ ХОТЯБЫ РАЗ ЭТО СДЕЛАТЬ
        auto doc = CommonData{};
        doc.UserName = "Иванов И.И.";
        doc.Cashier = "Петров П.П.";
        doc.Address = "ул.Тверская, д.1";

        auto response = service->__07__SendDocuments(doc.to_tlv_list());
        std::cout << "COMMAND: 07h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
    }
/////////////////////////////////////////////////////////////////////////////
// 16 - Сформировать чек
/////////////////////////////////////////////////////////////////////////////
    {
        auto req = CreateCheckRequest();
        req.DateTime = time(0);
        req.OperationType = OperationType::PARISH; // Приход
        req.Total = 1000 * 100; // в копейках

        auto response = service->__16__CreateCheck(req);
        std::cout << "COMMAND: 16h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
        std::cout << response->CheckNum << std::endl;
        std::cout << std::boolalpha << response->FiscDocNumber << std::endl;
        std::cout << response->FiscSign << std::endl;
    }
/////////////////////////////////////////////////////////////////////////////
// 17 - Начать формирование чека коррекции
/////////////////////////////////////////////////////////////////////////////
    {
        auto req = StartCheckCorrectionRequest{};
        req.DateTime = time(0);

        auto response = service->__17__StartCheckCorrection(req);
        std::cout << "COMMAND: 17h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
    }
/////////////////////////////////////////////////////////////////////////////
// 07 - Передать необходимые документы
/////////////////////////////////////////////////////////////////////////////
    {
        // ВООБЩЕ ПОХЕР ЧТО ОТПРАВЛЯЕТСЯ, ГЛАВНОЕ ХОТЯБЫ РАЗ ЭТО СДЕЛАТЬ
        auto doc = CommonData{};
        doc.UserName = "Иванов И.И.";
        doc.Cashier = "Петров П.П.";
        doc.Address = "ул.Тверская, д.1";

        auto response = service->__07__SendDocuments(doc.to_tlv_list());
        std::cout << "COMMAND: 07h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
    }
/////////////////////////////////////////////////////////////////////////////
// 16 - Сформировать чек (коррекции)
/////////////////////////////////////////////////////////////////////////////
    {
        auto req = CreateCheckRequest();
        req.DateTime = time(0);
        req.OperationType = OperationType::PARISH; // Приход
        req.Total = 1000 * 100; // в копейках

        auto response = service->__16__CreateCheck(req);
        std::cout << "COMMAND: 16h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
        std::cout << response->CheckNum << std::endl;
        std::cout << std::boolalpha << response->FiscDocNumber << std::endl;
        std::cout << response->FiscSign << std::endl;
    }
/////////////////////////////////////////////////////////////////////////////
// 13 - Начать закрытие смены
/////////////////////////////////////////////////////////////////////////////
    {
        auto req = StartCloseShiftRequest{};
        req.DateTime = time(0);

        auto response = service->__13__StartCloseShift(req);
        std::cout << "COMMAND: 13h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
    }
/////////////////////////////////////////////////////////////////////////////
// 07 - Передать необходимые документы
/////////////////////////////////////////////////////////////////////////////
    {
        auto doc = CommonData{};
        doc.UserName = "Иванов И.И.";
        doc.Cashier = "Петров П.П.";
        doc.Address = "ул.Тверская, д.1";

        auto response = service->__07__SendDocuments(doc.to_tlv_list());
        std::cout << "COMMAND: 07h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
    }
/////////////////////////////////////////////////////////////////////////////
// 14 - Закрыть смену
/////////////////////////////////////////////////////////////////////////////
    {
        auto response = service->__14__ApproveCloseShift();
        std::cout << "COMMAND: 14h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
        std::cout << response->ShiftNum << std::endl;
        std::cout << std::boolalpha << response->FiscDocNumber << std::endl;
        std::cout << response->FiscSign << std::endl;
    }
/////////////////////////////////////////////////////////////////////////////
// ОТПРВКА В ОФД
/////////////////////////////////////////////////////////////////////////////
    if (!ofd_client->SendDocuments("KKT-772-233-445-566")) {
        std::cout << "SEND TO OFD FAILED" << std::endl;
    }
/////////////////////////////////////////////////////////////////////////////
// 04 - Начать закрытие фискального режима
/////////////////////////////////////////////////////////////////////////////
    {
        auto response = service->__04__StartCloseFiscalisation();
        std::cout << "COMMAND: 04h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
    }
/////////////////////////////////////////////////////////////////////////////
// 07 - Передать необходимые документы
/////////////////////////////////////////////////////////////////////////////
    {
        auto doc = CommonData{};
        doc.Cashier = "Петров П.П.";
        doc.Address = "ул.Тверская, д.1";

        auto response = service->__07__SendDocuments(doc.to_tlv_list());
        std::cout << "COMMAND: 07h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
    }
/////////////////////////////////////////////////////////////////////////////
// 05 - Закрыть фискальный режим ФН
///////////////////////////////////////////////////////////////////////////// 
    {
        auto req = CloseFiscalisationRequest{};
        req.DateTime = time(0);
        req.KKTNumber_cp866 = "KKT-772-233-445-566";

        auto response = service->__05__CloseFiscalisation(req);
        std::cout << "COMMAND: 05h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
        std::cout << response->FiscDocNumber << std::endl;
        std::cout << response->FiscSign << std::endl;
    }
/////////////////////////////////////////////////////////////////////////////
// 40 - Найти фискальный документ по номеру
/////////////////////////////////////////////////////////////////////////////
    {
        auto response = service->__40__GetFiscDocument(3);
        std::cout << "COMMAND: 40h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }

        std::cout << int(response->DocumentType) << std::endl;
        std::cout << std::boolalpha << response->GetOFDReceipt << std::endl;

        // у всех ФД
        std::cout << response->Document->DateTime << std::endl;
        std::cout << response->Document->Number << std::endl;
        std::cout << response->Document->FiscSign << std::endl;

        // в зависимости от типа документа:
        if (CheckDocument* doc = dynamic_cast<CheckDocument*>(response->Document)) {
            // чек
            std::cout << "CHECK DOCUMENT" << std::endl;
            std::cout << int(doc->OpType) << std::endl;
            std::cout << doc->Sum << std::endl;
        }
        if (RegistrationDocument* doc = dynamic_cast<RegistrationDocument*>(response->Document)) {
            // фискализация
            std::cout << "REGISTRATION DOCUMENT" << std::endl;
            std::cout << doc->Inn << std::endl;
            std::cout << doc->KTTNumber << std::endl;
            std::cout << int(doc->NCode) << std::endl;
            std::cout << int(doc->WMode) << std::endl;
        }
        if (ShiftDocument* doc = dynamic_cast<ShiftDocument*>(response->Document)) {
            // открытие/закрытие смены
            std::cout << "SHIFT DOCUMENT" << std::endl;
            std::cout << doc->ShiftNum << std::endl;
        }
        if (CloseFiscDocumnet* doc = dynamic_cast<CloseFiscDocumnet*>(response->Document)) {
            // закрытие фиск. режима
            std::cout << "CLOSE FISCAL MODE DOCUMENT" << std::endl;
            std::cout << doc->Inn << std::endl;
            std::cout << doc->KTTNumber << std::endl;
        }
    }
/////////////////////////////////////////////////////////////////////////////
// 42 - Запрос количества ФД, на которые нет квитанции
/////////////////////////////////////////////////////////////////////////////
    {
        auto response = service->__42__GetFDAmmountWithoutOFD();
        std::cout << "COMMAND: 42h" << std::endl;
        if (response->ErrorMsg != "") {
            std::cout << "ERROR: " << response->ErrorMsg << std::endl;
        }
        std::cout << response->Ammount << std::endl;
    }
/////////////////////////////////////////////////////////////////////////////
    system("pause");
}