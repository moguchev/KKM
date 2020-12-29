#include "loginoperatorofd.h"
#include "ui_loginoperatorofd.h"

LoginOperatorOFD::LoginOperatorOFD(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LoginOperatorOFD),
    counter(0) {
    ui->setupUi(this);
    connect(ui->pushButton_ok, SIGNAL(clicked()), this, SLOT(checklogin()));
}

LoginOperatorOFD::~LoginOperatorOFD() {
    delete ui;
}

void LoginOperatorOFD::clearData() {
    ui->login->clear();
    ui->password->clear();
    ui->labelMess->clear();
}

void LoginOperatorOFD::checklogin() {
    counter++;
    if (counter > 2) {
        counter = 0;
        this->hide();
    } else {
        if (ui->login->text() != "admin" || ui->password->text() != "admin")
            ui->labelMess->setText("Неверный логин или пароль");
        emit this->login(ui->login->text(), ui->password->text());
    }
}

void LoginOperatorOFD::on_pushButton_close_clicked() {
    this->hide();
}
