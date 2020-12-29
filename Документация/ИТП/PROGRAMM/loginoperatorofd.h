#ifndef LOGINOPERATOROFD_H
#define LOGINOPERATOROFD_H

#include <QWidget>

namespace Ui {
class LoginOperatorOFD;
}

class LoginOperatorOFD : public QWidget
{
    Q_OBJECT

public:
    explicit LoginOperatorOFD(QWidget *parent = nullptr);
    ~LoginOperatorOFD();

    void clearData();

signals:
    void login(const QString &, const QString &);

private slots:
    void checklogin();

    void on_pushButton_close_clicked();

private:
    Ui::LoginOperatorOFD *ui;
    size_t counter;
};

#endif // LOGINOPERATOROFD_H
