#include "settingkkmwindow.h"
#include "ui_settingkkmwindow.h"

SettingKKMWindow::SettingKKMWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingKKMWindow) {
    ui->setupUi(this);
    initCB();
}

void SettingKKMWindow::initCB() {
    ui->cb_model->addItem("Модель ККМ");
    ui->cb_link->addItems({"USB", "COM"});
    ui->cb_com->addItems({"COM1", "COM2", "COM3"});
    ui->cb_speed->addItems({"115200"});
    ui->cb_bit->addItems({"8", "9"});
    ui->cb_paraty->addItems({"Нет", "Четно", "Нечетно"});
    ui->cb_stopbit->addItems({"1 бит", "2 бита"});
    connect(ui->pushButton_ok, SIGNAL(clicked()), this, SLOT(sendSetting()));
}

SettingKKMWindow::~SettingKKMWindow() {
    delete ui;
}

void SettingKKMWindow::on_pushButton_cansel_clicked() {
    this->hide();
}

void SettingKKMWindow::sendSetting() {
    if (ui->cashir->text().isEmpty()) {
        QMessageBox::warning(this, tr("Ошибка"),
                                   tr("Введите имя кассира!"),
                                   QMessageBox::Ok);
    } else {
        emit this->saveSetting(ui->cashir->text(), ui->cb_com->currentText());
        this->hide();
    }
}

void SettingKKMWindow::on_cb_link_currentIndexChanged(int index) {
    if (index == 0) {
        ui->cb_com->setEnabled(false);
        ui->cb_speed->setEnabled(false);
        ui->cb_bit->setEnabled(false);
        ui->cb_paraty->setEnabled(false);
        ui->cb_stopbit->setEnabled(false);
    } else {
        ui->cb_com->setEnabled(true);
        ui->cb_speed->setEnabled(true);
        ui->cb_bit->setEnabled(true);
        ui->cb_paraty->setEnabled(true);
        ui->cb_stopbit->setEnabled(true);
    }
}

