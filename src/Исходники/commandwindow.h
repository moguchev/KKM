#ifndef COMMANDWINDOW_H
#define COMMANDWINDOW_H

#include <QWidget>
#include <ComReader/Hardware.h>
#include <iostream>

enum class COMMANDS_FN {
    _30h,
    _31h,
    _32h,
    _33h,
    _10h
};

namespace Ui {
class CommandWindow;
}

class CommandWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CommandWindow(QWidget *parent = nullptr);
    ~CommandWindow();

signals:
    void request(COMMANDS_FN command);

private slots:
    void sendRequest();

    void printInfo(const QString &info);

    void on_pushButtonClose_clicked();

    void on_pushButton_clear_clicked();

private:
    Ui::CommandWindow *ui;

    void initCommands();
};

#endif // COMMANDWINDOW_H
