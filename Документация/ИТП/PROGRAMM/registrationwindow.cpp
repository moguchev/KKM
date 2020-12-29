#include "registrationwindow.h"
#include "ui_registrationwindow.h"

RegistrationWindow::RegistrationWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegistrationWindow),
    regData(new RegistrationData) {
    ui->setupUi(this);
    connect(ui->pushButton_reg, SIGNAL(clicked()), this, SLOT(sendData()));
}

RegistrationWindow::~RegistrationWindow() {
    delete ui;
    delete regData;
}

void RegistrationWindow::on_pushButton_cansel_clicked() {
    this->hide();
}

bool RegistrationWindow::checkData() {
    if (!ui->user->text().isEmpty() && !ui->addr->text().isEmpty() &&
        !ui->place->text().isEmpty() && !ui->regNumber->text().isEmpty() &&
        !ui->inn->text().isEmpty() && !ui->sitefns->text().isEmpty() &&
        !ui->nameOperator->text().isEmpty() && !ui->innOperator->text().isEmpty() &&
        !ui->ip->text().isEmpty() && !ui->port->text().isEmpty()) {
        if (ui->NalogCode1->isChecked() || ui->NalogCode2->isChecked() ||
            ui->NalogCode4->isChecked() || ui->NalogCode8->isChecked() ||
            ui->NalogCode16->isChecked() || ui->NalogCode32->isChecked()) {
            if (ui->WorkMode1->isChecked() || ui->WorkMode2->isChecked() ||
                ui->WorkMode4->isChecked() || ui->WorkMode8->isChecked() ||
                ui->WorkMode16->isChecked() || ui->WorkMode32->isChecked()) {
                return true;
            }
        }
    }
    return false;
}

void RegistrationWindow::sendData() {
    if (!checkData()) {
        QMessageBox::warning(this, tr("Ошибка"),
                                   tr("Заполнены не все поля!"),
                                   QMessageBox::Ok);
        return;
    } else {
        regData->user = ui->user->text();
        regData->addres = ui->addr->text();
        regData->placeSettlement = ui->place->text();
        regData->regNumberKKT = ui->regNumber->text();
        regData->INN = ui->inn->text();
        regData->siteFNS = ui->sitefns->text();
        regData->operatorOFD = ui->nameOperator->text();
        regData->INNoperator = ui->innOperator->text();
        regData->IP = ui->ip->text();
        regData->port = ui->port->text();

        if (ui->NalogCode1->isChecked()) regData->nc = NalogCode::COMMON;
        if (ui->NalogCode2->isChecked()) regData->nc = NalogCode::SIMPLE_INCOME;
        if (ui->NalogCode4->isChecked()) regData->nc = NalogCode::SIMPLE_INCOME_MINUS_EXPENSE;
        if (ui->NalogCode8->isChecked()) regData->nc = NalogCode::SINGLE_TAX_ON_IMPUTED_INCOME;
        if (ui->NalogCode16->isChecked()) regData->nc = NalogCode::UNIFIED_AGRICULTURAL_TAX;
        if (ui->NalogCode32->isChecked()) regData->nc = NalogCode::PATENT_TAXATION_SYSTEM;

        if (ui->WorkMode1->isChecked()) regData->wm = WorkMode::ENCRYPTION;
        if (ui->WorkMode2->isChecked()) regData->wm = WorkMode::OFFLINE;
        if (ui->WorkMode4->isChecked()) regData->wm = WorkMode::AUTO;
        if (ui->WorkMode8->isChecked()) regData->wm = WorkMode::SERVICE_APPLICATIONS;
        if (ui->WorkMode16->isChecked()) regData->wm = WorkMode::CHECK;
        if (ui->WorkMode32->isChecked()) regData->wm = WorkMode::INTERNET_COMMERCE;

        emit this->saveData(regData);
        this->hide();
        QMessageBox::information(this, tr("Сообщение"),
                                   tr("Регистрация прошла успешно!"),
                                   QMessageBox::Ok);
    }
}

void RegistrationWindow::on_toolButton_clicked() {
    ui->user->setText("OOO \"Курсовая\"");
    ui->addr->setText("г. Москва");
    ui->place->setText("Там где сделанный курсач");
    ui->regNumber->setText("KKT-772-233-445-566");
    ui->inn->setText("409023738");
    ui->sitefns->setText("www.nalog.ru");
    ui->NalogCode1->setChecked(true);
    ui->WorkMode1->setChecked(true);
    ui->nameOperator->setText("ЗАО \"Панимаю\"");
    ui->innOperator->setText("4029013781");
    ui->ip->setText("127.0.0.1");
    ui->port->setText("4321");
}
