#include "commandwindow.h"
#include "ui_commandwindow.h"

CommandWindow::CommandWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CommandWindow) {
    ui->setupUi(this);
    initCommands();
    connect(ui->pushButton_request, SIGNAL(clicked()), this, SLOT(sendRequest()));
}

CommandWindow::~CommandWindow() {
    delete ui;
}

void CommandWindow::initCommands() {
    ui->cbCommandFN->addItems({"30h - Запрос статуса ФН",
                               "31h - Запрос номера ФН",
                               "32h - Запрос срока действия ФН",
                               "33h - Запрос версии ФН",
                               "10h - Запрос параметров текущей смены"});
}

void CommandWindow::on_pushButtonClose_clicked() {
    this->hide();
}

void CommandWindow::sendRequest() {
    emit this->request(COMMANDS_FN(ui->cbCommandFN->currentIndex()));
}

void CommandWindow::printInfo(const QString &info) {
    ui->textBrowser->clear();
    ui->textBrowser->setText(info);
}

void CommandWindow::on_pushButton_clear_clicked() {
    ui->textBrowser->clear();
}
