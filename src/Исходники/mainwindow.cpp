#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTabBar>
#include <QWidget>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    _settingProgW(new SettingProgWindow),
    _settingKKMW(new SettingKKMWindow),
    _loginW(new LoginOperatorOFD),
    _regW(new RegistrationWindow),
    _regData(new RegistrationData),
    _commandW(new CommandWindow),
    _isReg(false),
    _method_calc(0),
    _subject_calc(0),
    _nds(0),
    _quantity(0),
    _count_check(0),
    _posishion_check(0),
    _checkAmount(0),
    _received(0),
    _change(0),
    _KKMAmount(0),
    _cashAmount(0),
    _cashlessAmount(0) {

    ui->setupUi(this);
    initKKMWindow();
    initNewChekWindow();
    initPDB();
    initHDB();
    initInspectWindow();
    initRegistrWindow();
    initCommandWindow();

    ui->new_chek->setEnabled(false);
    ui->inspect->setEnabled(false);
    ui->setting_prog->setEnabled(false);
    ui->other_commands->setEnabled(false);
    ui->open_shift->setEnabled(false);
    ui->close_shift->setEnabled(false);
    ui->info->setEnabled(false);
    ui->buttonFiscalEnd->setEnabled(false);
    ui->buttonFiscal->setEnabled(false);
}

MainWindow::~MainWindow() {
    delete ui;
    delete _settingProgW;
    delete _settingKKMW;
    delete _loginW;
    delete _regW;
    delete _commandW;
    delete _pdb;
    delete _hdb;
    delete _modelP;
    delete _modelH;
    delete _regData;
}

void MainWindow::initPDB() {
    _pdb = new ProductDB();
    _pdb->connectToDataBase();
    _modelP = new QSqlTableModel(this);
    // Инициализируем модель для представления данных
    this->setupModel(TABLE_PRODUCT,
                     QStringList() << trUtf8("ID")
                                   << trUtf8("Штрих-код")
                                   << trUtf8("Название товара")
                                   << trUtf8("Цена"), _modelP);
    this->createUI(ui->productDB, _modelP); // Инициализируем внешний вид таблицы с данными
}

void MainWindow::initHDB() {
    _hdb = new HistoryDB();
    _hdb->connectToDataBase();
    _modelH = new QSqlTableModel(this);
    // Инициализируем модель для представления данных
    this->setupModel(TABLE_HISTORY,
                     QStringList() << trUtf8("Создан")
                                   << trUtf8("№ чека")
                                   << trUtf8("Операция")
                                   << trUtf8("Позиций")
                                   << trUtf8("Сумма нал.")
                                   << trUtf8("Сумма безнал.")
                                   << trUtf8("Получено")
                                   << trUtf8("Сдача")
                                   << trUtf8("№ фиск. док.")
                                   << trUtf8("Фиск. пр."), _modelH);
    this->createUI(ui->historyDB, _modelH); // Инициализируем внешний вид таблицы с данными
}

void MainWindow::initKKMWindow() {
    auto size = ui->tabWidget->width() / ui->tabWidget->count();
    ui->tabWidget->setStyleSheet(ui->tabWidget->styleSheet() +
                                        "QTabBar::tab {"
                                        "width: " + QString::number(size) +
                                        "px; height: 35px}" );
    ui->is_open->setText(STATUS_SHIFT[0]);
    ui->is_redy_kkm->setText(STATUS_KKM[0]);
    ui->is_fiscal->setText(STATUS_FN[0]);
    ui->line_summ->setText(QString::number(_KKMAmount));

    _settingProgW->setAttribute(Qt::WA_DeleteOnClose);
    _settingProgW->setDefaultSetting(_method_calc, _subject_calc, _nds, _quantity);
    _settingProgW->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    _settingProgW->setWindowTitle("Настройка программы");

    _settingKKMW->setAttribute(Qt::WA_DeleteOnClose);
    _settingKKMW->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    _settingKKMW->setWindowTitle("Настройка кассы");

    connect(_settingProgW, SIGNAL(saveSetting(size_t, size_t, size_t, size_t)), this,
            SLOT(recieveProgSetting(size_t, size_t, size_t, size_t)));

    connect(_settingKKMW, SIGNAL(saveSetting(const QString &, const QString &)), this,
            SLOT(initKKMSetting(const QString &, const QString &)));
}

void MainWindow::initNewChekWindow() {
    ui->line_sum_check->setText("0.00");
    ui->line_drop_check->setText("0.00");
    ui->line_sdacha_check->setText("0");

    ui->number_check->setText(QString::number(_count_check));

    ui->comboBox_CashAmount->addItems({"Наличные", "Безнал."});
    ui->comboBox_sell->addItems({"Приход (продажа)", "Возврат прихода", "Расход", "Возврат расхода"});

    ui->label_is_pr->setText(METHOD_OF_CALC[_method_calc]);
    ui->label_subject->setText(SUBJECT_OF_CALC[_subject_calc]);
    ui->label_is_nds->setText(NDS[_nds]);

    ui->tableWidget_check->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
}

void MainWindow::initInspectWindow() {
    _loginW->setAttribute(Qt::WA_DeleteOnClose);
    _loginW->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    _loginW->setWindowTitle("Вход инспектора ФНС");

    connect(_loginW, SIGNAL(login(const QString &, const QString &)), this,
            SLOT(checkLoginFNS(const QString &, const QString &)));
}

void MainWindow::checkLoginFNS(const QString &login, const QString &pass) {
    if (login == "admin" && pass == "admin") {
        ui->inspect->setEnabled(true);
        _loginW->hide();
    }
}

void MainWindow::initRegistrWindow() {
    _regW->setAttribute(Qt::WA_DeleteOnClose);
    _regW->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    _regW->setWindowTitle("Регистрация ККМ");

    connect(_regW, SIGNAL(saveData(const RegistrationData *)), this,
            SLOT(registration(const RegistrationData *)));
}

void MainWindow::registration(const RegistrationData *data) {
    _regData->user = data->user;
    _regData->addres = data->addres;
    _regData->placeSettlement = data->placeSettlement;
    _regData->regNumberKKT = data->regNumberKKT;
    _regData->INN = data->INN;
    _regData->siteFNS = data->siteFNS;
    _regData->nc = data->nc;
    _regData->wm = data->wm;
    _regData->operatorOFD = data->operatorOFD;
    _regData->INNoperator = data->INNoperator;
    _regData->IP = data->IP;
    _regData->port = data->port;

    if (ui->is_redy_kkm->text() != STATUS_KKM[0]) {
        auto response = _service->__30__GetFNStatus();
        if (response->ErrorMsg != "") {
            QMessageBox::warning(this, tr("Ошибка"),
                                       tr(response->ErrorMsg.c_str()),
                                       QMessageBox::Ok);
            return;
        } else {
            if (response->PhaseOfLife == FN_LIFE_PHASES.at(3)) {
                ui->is_fiscal->setText(STATUS_FN[2]);
            } else if (response->PhaseOfLife == FN_LIFE_PHASES.at(1)) {
                auto response = _service->__02__StartFiscalisation();
                if (response->ErrorMsg != "") {
                    QMessageBox::warning(this, tr("Ошибка"),
                                               tr(response->ErrorMsg.c_str()),
                                               QMessageBox::Ok);
                    return;
                } else {
                    auto doc = CommonData{};
                    doc.UserName = _regData->user.toStdString();
                    doc.Cashier = _nameCashir.toStdString();
                    doc.Address = _regData->addres.toStdString();
                    doc.InnOFD = _regData->INNoperator.toStdString();

                    auto response = _service->__07__SendDocuments(doc.to_tlv_list());
                    if (response->ErrorMsg != "") {
                        QMessageBox::warning(this, tr("Ошибка"),
                                                   tr(response->ErrorMsg.c_str()),
                                                   QMessageBox::Ok);
                        return;
                    } else {
                        auto req = ApproveFiscalisationRequest{};
                        req.DateTime = time(0);
                        req.Inn_cp866 = _regData->INN.toStdString();
                        req.KKTNumber_cp866 = _regData->regNumberKKT.toStdString();
                        req.NalogCode = _regData->nc;
                        req.WorkMode = _regData->wm;

                        auto response = _service->__03__ApproveFiscalisation(req);
                        if (response->ErrorMsg != "") {
                            QMessageBox::warning(this, tr("Ошибка"),
                                                       tr(response->ErrorMsg.c_str()),
                                                       QMessageBox::Ok);
                            return;
                        }
                        ui->is_fiscal->setText(STATUS_FN[2]);
                        ui->line_summ->setText(QString::number(10000.00));

                        addRowinHistory("", "Фискализация", "", "", "", "", "",
                                        QString::number(response->FiscDocNumber),
                                        QString::number(response->FiscSign));
                        QMessageBox::information(this, tr("Сообщение"),
                                                   tr("ФН фискализирована!"),
                                                   QMessageBox::Ok);
                        ui->setting_prog->setEnabled(true);
                        ui->other_commands->setEnabled(true);
                        ui->open_shift->setEnabled(true);
                        ui->close_shift->setEnabled(true);
                        ui->info->setEnabled(true);
                        ui->buttonFiscalEnd->setEnabled(true);
                    }
                }
            }
        }
    } else {
        QMessageBox::warning(this, tr("Ошибка"),
                                   tr("ККМ не готова!"),
                                   QMessageBox::Ok);
    }
}

void MainWindow::initCommandWindow() {
    _commandW->setAttribute(Qt::WA_DeleteOnClose);
    _commandW->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    _commandW->setWindowTitle("Команды ФН");

    connect(_commandW, SIGNAL(request(COMMANDS_FN)), this, SLOT(executeCommand(COMMANDS_FN)));
    connect(this, SIGNAL(sendCommandInfo(const QString &)), _commandW, SLOT(printInfo(const QString &)));
}

void MainWindow::on_push_exit_clicked() {
    this->close();
}

bool MainWindow::initKKMSetting(const QString &name, const QString &port) {
    if (ui->is_redy_kkm->text() == STATUS_KKM[1]) {
        QMessageBox::information(this, tr("Сообщение"),
                                   tr("Соединение уже установлено!"),
                                   QMessageBox::Ok);
        return true;
    }
    _nameCashir = name;
    ui->nameCashir->setText(_nameCashir);
    ui->label_setSetting->hide();
    _port = port;
    _service = std::make_shared<Hardware>(port.toStdWString(), 100000);
    auto connected = _service->get_connection_status();
    // добавить таймаут
    if (connected) {
        ui->is_redy_kkm->setText(STATUS_KKM[1]);
        ui->is_fiscal->setText(STATUS_FN[1]);
        // проверка фискализирована ли
        auto response = _service->__30__GetFNStatus();
        if (response->ErrorMsg != "") {
            QMessageBox::warning(this, tr("Ошибка"),
                                       tr(response->ErrorMsg.c_str()),
                                       QMessageBox::Ok);
            return false;
        } else {
            if (response->PhaseOfLife == FN_LIFE_PHASES.at(3))
                ui->is_fiscal->setText(STATUS_FN[2]);
            ui->buttonFiscal->setEnabled(true);
            QMessageBox::information(this, tr("Сообщение"),
                                       tr("Соединение установлено!"),
                                       QMessageBox::Ok);
            ui->buttonFiscal->setEnabled(true);
        }
    } else {
        QMessageBox::warning(this, tr("Ошибка"),
                                   tr("Проверьте соединение!"),
                                   QMessageBox::Ok);
        return false;
    }
}

void MainWindow::on_buttonFiscal_clicked() {
    if (_isReg) {
        QMessageBox::information(this, tr("Сообщение"),
                                   tr("Регистрация выполнена!"),
                                   QMessageBox::Ok);
        return;
    }
    _isReg = true;
    ui->tabWidget->setEnabled(true);
    ui->label_acceess->hide();
    _regW->show();
}

void MainWindow::on_buttonFiscalEnd_clicked() {
    if (ui->is_fiscal->text() == STATUS_FN[2]) {
        if (ui->is_open->text() == STATUS_SHIFT[1]) {
            QMessageBox::information(this, tr("Сообщение"),
                                       tr("Закройте смену!"),
                                       QMessageBox::Ok);
            return;
        }
        auto response = _service->__04__StartCloseFiscalisation();
        if (response->ErrorMsg != "") {
            QMessageBox::warning(this, tr("Ошибка"),
                                       tr(response->ErrorMsg.c_str()),
                                       QMessageBox::Ok);
            return;
        } else {
            auto doc = CommonData{};
            doc.Cashier = _nameCashir.toStdString();
            doc.Address = _regData->addres.toStdString();
            auto response = _service->__07__SendDocuments(doc.to_tlv_list());
            if (response->ErrorMsg != "") {
                QMessageBox::warning(this, tr("Ошибка"),
                                           tr(response->ErrorMsg.c_str()),
                                           QMessageBox::Ok);
                return;
            } else {
                auto req = CloseFiscalisationRequest{};
                req.DateTime = time(0);
                req.KKTNumber_cp866 = _regData->regNumberKKT.toStdString();

                auto response = _service->__05__CloseFiscalisation(req);
                if (response->ErrorMsg != "") {
                    QMessageBox::warning(this, tr("Ошибка"),
                                               tr(response->ErrorMsg.c_str()),
                                               QMessageBox::Ok);
                    return;
                }
                ui->is_fiscal->setText(STATUS_FN[3]);
                addRowinHistory("", ""
                                    "Закрытие фискального режима", "", "", "", "", "",
                                QString::number(response->FiscDocNumber),
                                QString::number(response->FiscSign));
                QMessageBox::information(this, tr("Сообщение"),
                                           tr("Фискальный режим закрыт!"),
                                           QMessageBox::Ok);
                ui->kkm->setEnabled(false);
                ui->new_chek->setEnabled(false);
                ui->inspect->setEnabled(true);
            }
        }
    } else {
        QMessageBox::warning(this, tr("Ошибка"),
                                   tr("ККМ не фискализирована!"),
                                   QMessageBox::Ok);
    }
}

void MainWindow::on_open_shift_clicked()
{
    if (ui->is_fiscal->text() != STATUS_FN[0]) {
        if (ui->is_open->text() == STATUS_SHIFT[1]) {
            QMessageBox::information(this, tr("Сообщение"),
                                       tr("Смена уже открыта!"),
                                       QMessageBox::Ok);
            return;
        }
        auto req = StartOpeningShiftRequest{};
        req.DateTime = time(0);
        auto response = _service->__11__StartOpeningShift(req);
        if (response->ErrorMsg != "") {
            QMessageBox::warning(this, tr("Ошибка"),
                                       tr(response->ErrorMsg.c_str()),
                                       QMessageBox::Ok);
            return;
        } else {
            auto doc = CommonData{};
            doc.UserName = _regData->user.toStdString();
            doc.Cashier = _nameCashir.toStdString();
            doc.Address = _regData->addres.toStdString();

            auto response = _service->__07__SendDocuments(doc.to_tlv_list());
            if (response->ErrorMsg != "") {
                QMessageBox::warning(this, tr("Ошибка"),
                                           tr(response->ErrorMsg.c_str()),
                                           QMessageBox::Ok);
                return;
            } else {
                auto response = _service->__12__ApproveOpeningShift();
                if (response->ErrorMsg != "") {
                    QMessageBox::warning(this, tr("Ошибка"),
                                               tr(response->ErrorMsg.c_str()),
                                               QMessageBox::Ok);
                    return;
                }
                ui->is_open->setText(STATUS_SHIFT[1]);
                ui->new_chek->setEnabled(true);
                ui->val_sum_nal->setText("0.00");
                ui->val_sum_beznal->setText("0.00");
                ui->shiftNumber->setText(QString::number(response->ShiftNum));
                addRowinHistory("", "Открытие смены", "", "", "", "", "",
                                QString::number(response->FiscDocNumber),
                                QString::number(response->FiscSign));
            }
        }
    } else {
        QMessageBox::warning(this, tr("Ошибка"),
                                   tr("ККМ не фискализирована!"),
                                   QMessageBox::Ok);
    }
}

void MainWindow::on_close_shift_clicked()
{
    if (ui->is_fiscal->text() != STATUS_FN[0]) {
        if (ui->is_open->text() == STATUS_SHIFT[0]) {
            QMessageBox::information(this, tr("Сообщение"),
                                       tr("Смена не открыта!"),
                                       QMessageBox::Ok);
            return;
        }
        auto req = StartCloseShiftRequest{};
        req.DateTime = time(0);

        auto response = _service->__13__StartCloseShift(req);
        if (response->ErrorMsg != "") {
            QMessageBox::warning(this, tr("Ошибка"),
                                       tr(response->ErrorMsg.c_str()),
                                       QMessageBox::Ok);
            return;
        } else {
            auto doc = CommonData{};
            doc.UserName = _regData->user.toStdString();
            doc.Cashier = _nameCashir.toStdString();
            doc.Address = _regData->addres.toStdString();

            auto response = _service->__07__SendDocuments(doc.to_tlv_list());
            if (response->ErrorMsg != "") {
                QMessageBox::information(this, tr("Сообщение"),
                                           tr("Смена не открыта!"),
                                           QMessageBox::Ok);
                return;
            } else {
                auto response = _service->__14__ApproveCloseShift();
                if (response->ErrorMsg != "") {
                    QMessageBox::information(this, tr("Сообщение"),
                                               tr("Смена не открыта!"),
                                               QMessageBox::Ok);
                    return;
                }
                ui->is_open->setText(STATUS_SHIFT[2]);
                ui->new_chek->setEnabled(false);
                ui->shiftNumber->setText(QString::number(response->ShiftNum));
                ui->number_check->setText(QString::number(0)); // обновляем
                ui->val_sum_nal->setText(QString::number(_KKMAmount));
                ui->val_sum_beznal->setText(QString::number(_KKMAcashlessAmount));
                addRowinHistory("", "Закрытие смены", "",
                                QString::number(_KKMcashAmount, 'f', 2),
                                QString::number(_KKMAcashlessAmount, 'f', 2), "", "",
                                QString::number(response->FiscDocNumber),
                                QString::number(response->FiscSign));
                _KKMcashAmount = 0;
                _KKMAcashlessAmount = 0;
            }
        }
    } else {
        QMessageBox::warning(this, tr("Ошибка"),
                                   tr("ККМ не фискализирована!"),
                                   QMessageBox::Ok);
    }
}

void MainWindow::on_get_check_clicked() // формирование чека
{
    if (_checkAmount > _received || _checkAmount == 0) {
        int ret = QMessageBox::warning(this, tr("Ошибка"),
                                       tr("Недостаточно средств.\n"
                                          "Повторите попытку!"),
                                       QMessageBox::Ok);
        return;
    }

    auto req = StartCheckRequest{};
    req.DateTime = time(0);

    auto response = _service->__15__StartCheck(req);
    if (response->ErrorMsg != "") {
        QMessageBox::warning(this, tr("Ошибка"),
                                   tr(response->ErrorMsg.c_str()),
                                   QMessageBox::Ok);
        return;
    } else {
        auto doc = CommonData{};
        doc.UserName = _regData->user.toStdString();
        doc.Cashier = _nameCashir.toStdString();
        doc.Address = _regData->addres.toStdString();
        doc.InnOFD = _regData->INNoperator.toStdString();

        auto response = _service->__07__SendDocuments(doc.to_tlv_list());
        if (response->ErrorMsg != "") {
            QMessageBox::warning(this, tr("Ошибка"),
                                       tr(response->ErrorMsg.c_str()),
                                       QMessageBox::Ok);
            return;
        } else {
            auto req = CreateCheckRequest();
            req.DateTime = time(0);
            req.OperationType = OperationType(ui->comboBox_sell->currentIndex());
            req.Total = _checkAmount;
            auto response = _service->__16__CreateCheck(req);
            if (response->ErrorMsg != "") {
                QMessageBox::warning(this, tr("Ошибка"),
                                           tr(response->ErrorMsg.c_str()),
                                           QMessageBox::Ok);
                return;
            }

            if (ui->comboBox_sell->currentIndex() == 0 || ui->comboBox_sell->currentIndex() == 2)
                _KKMAmount += _checkAmount; // обновляем сумму всей кассы
            else
                _KKMAmount -= _checkAmount; // обновляем сумму всей кассы

            ui->line_summ->setText(QString::number(_KKMAmount)); // обновляем сумму кассы

            ui->number_check->setText(QString::number(response->CheckNum)); // обновляем

            ui->tableWidget_check->setRowCount(0); // очистка таблицы

            if (ui->comboBox_CashAmount->currentIndex() == 0) {
                _cashAmount = _checkAmount;
                _KKMcashAmount += _checkAmount;
            } else {
                _cashlessAmount = _checkAmount;
                _KKMAcashlessAmount += _checkAmount;
            }

            // добавляем в историю
            addRowinHistory(QString::number(response->CheckNum),
                            ui->comboBox_sell->currentText(),
                            QString::number(_posishion_check),
                            QString::number(_cashAmount, 'f', 2),
                            QString::number(_cashlessAmount, 'f', 2),
                            QString::number(_received, 'f', 2),
                            QString::number(_change, 'f', 2),
                            QString::number(response->FiscDocNumber),
                            QString::number(response->FiscSign));

            // чистим переменные
            ui->line_sum_check->setText("0.00");
            ui->line_drop_check->setText("0.00");
            ui->line_sdacha_check->setText("0");
            _posishion_check = 0;
            _checkAmount = 0;
            _cashAmount= 0;
            _cashlessAmount= 0;
            _received = 0;
            _change = 0;
        }
    }
}

void MainWindow::executeCommand(COMMANDS_FN command) {
    QString message;
    switch (command) {
    case COMMANDS_FN::_30h: {
        auto response = _service->__30__GetFNStatus();
        message += "Команда: 30h - Запрос статуса ФН\n";
        if (response->ErrorMsg != "") {
            message += response->ErrorMsg.c_str();
            return;
        }
        message += QString("%1%2%3").arg("Состояние фазы жизни: ").arg(response->PhaseOfLife.c_str()).arg("\n");
        message += QString("%1%2%3").arg("Текущий документ: ").arg(response->CurrentDocument.c_str()).arg("\n");
        QString documentData = response->DocumentDataRecived ? "данные документа получены" : "нет данных документа";
        message += QString("%1%2%3").arg("Данные документ: ").arg(documentData).arg("\n");
        QString shiftData = response->ShiftIsOpen ? "смена открыта" : "смена закрыта";
        message += QString("%1%2%3").arg("Состояние смены: ").arg(shiftData).arg("\n");
        message += QString("%1%2%3").arg("Флаги предупреждения: ").arg(response->Warnings.c_str()).arg("\n");
        message += QString("%1%2%3").arg("Дата и время: ").arg(response->DateTime.c_str()).arg("\n");
        message += QString("%1%2%3").arg("Номер ФН: ").arg(response->Number_cp866.c_str()).arg("\n");
        message += QString("%1%2%3").arg("Номер последнего ФД: ").arg(QString::number(response->LastFDNumber)).arg("\n");
        emit this->sendCommandInfo(message);
        break;
    }
    case COMMANDS_FN::_31h: {
        auto response = _service->__31__GetFNNumber();
        message += "Команда: 31h - Запрос номера ФН\n";
        if (response->ErrorMsg != "") {
            message += response->ErrorMsg.c_str();
            return;
        }
        message += QString("%1%2%3").arg("Номер ФН: ").arg(response->Number_cp866.c_str()).arg("\n");
        emit this->sendCommandInfo(message);
        break;
    }
    case COMMANDS_FN::_32h: {
        auto response = _service->__32__GetFNEndDate();
        message += "Команда: 32h - Запрос срока действия ФН\n";
        if (response->ErrorMsg != "") {
            message += response->ErrorMsg.c_str();
            return;
        }
        message += QString("%1%2%3").arg("Срок действия ФН: ").arg(response->Date.c_str()).arg("\n");
        emit this->sendCommandInfo(message);
        break;
    }
    case COMMANDS_FN::_33h: {
        auto response = _service->__33__GetFNVersion();
        message += "Команда: 33h - Запрос версии ФН\n";
        if (response->ErrorMsg != "") {
            message += response->ErrorMsg.c_str();
            return;
        }
        message += QString("%1%2%3").arg("Версия программного обеспечения ФН: ").arg(response->VersionSoftWare_crc866.c_str()).arg("\n");
        message += QString("%1%2%3").arg("Тип программного обеспечения ФН: ").arg(response->TypeSoftWare.c_str()).arg("\n");
        emit this->sendCommandInfo(message);
        break;
    }
    case COMMANDS_FN::_10h: {
        auto response = _service->__10__GetShiftStatus();
        message += "Команда: 10h - Запрос параметров текущей смены\n";
        if (response->ErrorMsg != "") {
            message += response->ErrorMsg.c_str();
            return;
        }
        message += QString("%1%2%3").arg("Номер смены: ").arg(QString::number(response->ShiftNum)).arg("\n");
        QString shiftData = response->ShiftOpen ? "смена открыта" : "смена закрыта";
        message += QString("%1%2%3").arg("Состояние смены: ").arg(shiftData).arg("\n");
        message += QString("%1%2%3").arg("Номер чека: ").arg(QString::number(response->CheckAmmount)).arg("\n");
        emit this->sendCommandInfo(message);
        break;
    }
    }
}


void MainWindow::on_comboBox_sell_currentIndexChanged(int index) {
    if (index == 0 || index == 3)
        ui->label_pol_vid->setText("Получено");
    else if (index == 1 || index == 2)
        ui->label_pol_vid->setText("Выдано");
}

void MainWindow::on_setting_prog_clicked() {
    _settingProgW->show();
}

void MainWindow::on_setting_kkm_clicked() {
    _settingKKMW->show();
}

void MainWindow::on_other_commands_clicked() {
    _commandW->show();
}

void MainWindow::recieveProgSetting(size_t method_calc, size_t subject_calc,
                    size_t nds, size_t quantity) {
    _method_calc = method_calc;
    _subject_calc = subject_calc;
    _nds = nds;
    _quantity = quantity;
    ui->label_is_pr->setText(METHOD_OF_CALC[_method_calc]);
    ui->label_subject->setText(SUBJECT_OF_CALC[_subject_calc]);
    ui->label_is_nds->setText(NDS[_nds]);
}

void MainWindow::setupModel(const QString &tableName, const QStringList &headers, QSqlTableModel *model)
{
    model->setTable(tableName);

    for(int i = 0, j = 0; i < model->columnCount(); i++, j++){
        model->setHeaderData(i,Qt::Horizontal,headers[j]);
    }
    model->setSort(0,Qt::AscendingOrder);
}

void MainWindow::createUI(QTableView *t, QSqlTableModel  *model)
{
    t->setModel(model);     // Устанавливаем модель на TableView
    t->verticalHeader()->setVisible(false);
    // Разрешаем выделение строк
    t->setSelectionBehavior(QAbstractItemView::SelectRows);
    // Устанавливаем режим выделения лишь одно строки в таблице
    t->setSelectionMode(QAbstractItemView::SingleSelection);
    // Устанавливаем размер колонок по содержимому
    t->resizeColumnsToContents();
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->productDB->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->historyDB->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //ui_->productDB->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    model->select(); // Делаем выборку данных из таблицы
}

void MainWindow::addRowinCheck(const QString &name, float prise) {
    int newRow = ui->tableWidget_check->rowCount();
    for (auto i = 0; i < newRow; i++) { // проверяем есть ли данный товар в чеке
        if (ui->tableWidget_check->item(i, 0)->data(Qt::DisplayRole).toString() == name) {
            auto count = ui->tableWidget_check->item(i, 2)->data(Qt::DisplayRole).toInt() + 1;
            auto endPrise = ui->tableWidget_check->item(i, 3)->data(Qt::DisplayRole).toFloat() + prise;
            ui->tableWidget_check->setItem(i, 2, new QTableWidgetItem(QString::number(count)));
            ui->tableWidget_check->setItem(i, 3, new QTableWidgetItem(QString::number(endPrise)));
            return;
        }
    }
    ui->tableWidget_check->insertRow(newRow);
    ui->tableWidget_check->setItem(newRow, 0, new QTableWidgetItem(name));
    ui->tableWidget_check->setItem(newRow, 1, new QTableWidgetItem(QString::number(prise)));
    ui->tableWidget_check->setItem(newRow, 2, new QTableWidgetItem(QString::number(1)));
    ui->tableWidget_check->setItem(newRow, 3, new QTableWidgetItem(QString::number(prise)));
}

void MainWindow::addRowinHistory(const QString &numberCheck,
                                 const QString &operation,
                                 const QString &pos,
                                 const QString &cashAmount,
                                 const QString &cashlessAmount,
                                 const QString &received,
                                 const QString &change,
                                 const QString &fiscDoc,
                                 const QString &fiscSign) {
    QVariantList data;
    data.append(QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss"));
    data.append(numberCheck);
    data.append(operation);
    data.append(pos);
    data.append(cashAmount);
    data.append(cashlessAmount);
    data.append(received);
    data.append(change);
    data.append(fiscDoc);
    data.append(fiscSign);
    _hdb->inserIntoTable(data);
    _modelH->select();
}

void MainWindow::on_productDB_doubleClicked(const QModelIndex &index) {
    auto name = _modelP->data(_modelP->index(index.row(), 2)).toString();
    auto prise = _modelP->data(_modelP->index(index.row(), 3)).toFloat();

    _checkAmount += prise;
    ++_posishion_check; //увеличиваем количество позиций_
    addRowinCheck(name, prise);

    auto newPrise = QString::number(_checkAmount);
    ui->line_sum_check->setText(newPrise);
}

void MainWindow::on_line_drop_check_editingFinished() {
    _received = ui->line_drop_check->text().toFloat();
    if (_checkAmount < _received) {
            _change = _received - _checkAmount; // считаем сдачу
            ui->line_sdacha_check->setText(QString::number(_change));
    }
}

void MainWindow::on_tabWidget_currentChanged(int index) {
    qDebug() << "окно" << index;
    if (index == 3) {
        _loginW->clearData();
        _loginW->show();
    } else {
        ui->inspect->setEnabled(false);
        _loginW->hide();
    }
}
