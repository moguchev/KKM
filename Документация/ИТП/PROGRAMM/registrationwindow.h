#ifndef REGISTRATIONWINDOW_H
#define REGISTRATIONWINDOW_H

#include <QWidget>
#include <QMessageBox>
#include <ComReader/Hardware.h>

struct RegistrationData {
    QString user;
    QString addres;
    QString placeSettlement;
    QString regNumberKKT;
    QString INN;
    QString siteFNS;
    NalogCode nc;
    WorkMode wm;
    QString operatorOFD;
    QString INNoperator;
    QString IP;
    QString port;
}; // RegistrationData

namespace Ui {
class RegistrationWindow;
}

class RegistrationWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RegistrationWindow(QWidget *parent = nullptr);
    ~RegistrationWindow();


signals:
    void saveData(const RegistrationData *data);

private slots:
    void sendData();

    void on_pushButton_cansel_clicked();

    void on_toolButton_clicked();

private:
    Ui::RegistrationWindow *ui;
    RegistrationData *regData;

    bool checkData();
};

#endif // REGISTRATIONWINDOW_H
