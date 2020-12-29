#ifndef SETTINGPROGWINDOW_H
#define SETTINGPROGWINDOW_H

#include <QWidget>

const QStringList METHOD_OF_CALC = {
    "Полный расчет",
    "Предоплата",
    "Аванс",
    "Частич.расчет"
};

const QStringList SUBJECT_OF_CALC = {
    "Товар",
    "Подакциз.товар",
    "Работа",
    "Услуга",
    "Платеж"
};

const QStringList NDS = {
    "Без НДС",
    "0%",
    "10%",
    "20%"
};

const QStringList QUANTITY_PATTERN = {
    "999999"
};

namespace Ui {
class SettingProgWindow;
}

class SettingProgWindow : public QWidget {
    Q_OBJECT

public:
    explicit SettingProgWindow(QWidget *parent = nullptr);
    ~SettingProgWindow();

    void setDefaultSetting(size_t method_calc, size_t subject_calc,
                           size_t nds, size_t quantity);

signals:
    void saveSetting(size_t method_calc, size_t subject_calc,
                     size_t nds, size_t quantity);

private slots:

    void sendSetting();

    void on_pushButton_cansel_clicked();

private:
    void init();
    Ui::SettingProgWindow *ui;
};

#endif // SETTINGPROGWINDOW_H
