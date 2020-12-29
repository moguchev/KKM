#ifndef SETTINGKKMWINDOW_H
#define SETTINGKKMWINDOW_H

#include <QWidget>
#include <QMessageBox>

namespace Ui {
class SettingKKMWindow;
}

class SettingKKMWindow : public QWidget {
    Q_OBJECT

public:
    explicit SettingKKMWindow(QWidget *parent = nullptr);
    ~SettingKKMWindow();

signals:
    void saveSetting(const QString &name, const QString &port);

private slots:
    void sendSetting();

    void on_pushButton_cansel_clicked();

    void on_cb_link_currentIndexChanged(int index);

private:
    Ui::SettingKKMWindow *ui;

    void initCB();
};

#endif // SETTINGKKMWINDOW_H
