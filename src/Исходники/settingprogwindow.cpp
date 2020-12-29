#include "settingprogwindow.h"
#include "ui_settingprogwindow.h"

SettingProgWindow::SettingProgWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingProgWindow) {
    ui->setupUi(this);

    init();

    connect(ui->pushButton_ok, SIGNAL(clicked()), this, SLOT(sendSetting()));
}

SettingProgWindow::~SettingProgWindow() {
    delete ui;
}

void SettingProgWindow::init() {
    ui->comboBox_way->addItems(METHOD_OF_CALC);
    ui->comboBox_think->addItems(SUBJECT_OF_CALC);
    ui->comboBox_nds->addItems(NDS);
    ui->comboBox_template->addItems(QUANTITY_PATTERN);
}

void SettingProgWindow::setDefaultSetting(size_t method_calc,
                                          size_t subject_calc,
                                          size_t nds,
                                          size_t quantity) {
    ui->comboBox_way->setCurrentIndex(method_calc);
    ui->comboBox_think->setCurrentIndex(subject_calc);
    ui->comboBox_nds->setCurrentIndex(nds);
    ui->comboBox_template->setCurrentIndex(quantity);
}

void SettingProgWindow::on_pushButton_cansel_clicked() {
    this->hide();
}

void SettingProgWindow::sendSetting() {
    emit this->saveSetting(ui->comboBox_way->currentIndex(),
                           ui->comboBox_think->currentIndex(),
                           ui->comboBox_nds->currentIndex(),
                           ui->comboBox_template->currentIndex());
    this->hide();
}
