#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QSqlTableModel>
#include <QTableView>
#include <settingprogwindow.h>
#include <settingkkmwindow.h>
#include <loginoperatorofd.h>
#include <registrationwindow.h>
#include <commandwindow.h>
#include <productdb.h>
#include <historydb.h>
#include <memory>
#include <ComReader/utils.h>
#include <ComReader/Hardware.h>
#include <ComReader/TLV.h>

const QStringList STATUS_KKM = {"Не готова", "Готова"};
const QStringList STATUS_FN = {"Недоступна", "Готовность к фискализации", "Фискальный режим", "Фискальный режим закрыт"};
const QStringList STATUS_SHIFT = {"Не открыта", "Открыта", "Закрыта"};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setProgSetting(size_t method_calc, size_t subject_calc,
                        size_t nds, size_t quantity);

signals:
    void sendCommandInfo(const QString &);

private slots:

    void recieveProgSetting(size_t method_calc, size_t subject_calc,
                        size_t nds, size_t quantity);

    bool initKKMSetting(const QString &name, const QString &port);

    void registration(const RegistrationData *data);

    void executeCommand(COMMANDS_FN c);

    void checkLoginFNS(const QString &login, const QString &pass);

    void on_push_exit_clicked();

    void on_comboBox_sell_currentIndexChanged(int index);

    void on_setting_prog_clicked();

    void on_get_check_clicked();

    void on_productDB_doubleClicked(const QModelIndex &index);

    void on_line_drop_check_editingFinished();

    void on_setting_kkm_clicked();

    void on_buttonFiscal_clicked();

    void on_open_shift_clicked();

    void on_close_shift_clicked();

    void on_tabWidget_currentChanged(int index);

    void on_other_commands_clicked();

    void on_buttonFiscalEnd_clicked();

private:
    Ui::MainWindow *ui;
    SettingProgWindow *_settingProgW;
    SettingKKMWindow *_settingKKMW;
    LoginOperatorOFD *_loginW;
    RegistrationWindow *_regW;
    CommandWindow * _commandW;

    // Свзязь с proteus
    std::shared_ptr<Hardware> _service;

    // имя кассира
    QString _nameCashir;
    RegistrationData * _regData;
    bool _isReg;

    // настройки ккм
    QString _port;

    // настройки программы
    size_t _method_calc;
    size_t _subject_calc;
    size_t _nds;
    size_t _quantity;

    size_t _count_check;
    size_t _posishion_check;
    float _KKMAmount;          // сумма кассы
    float _KKMcashAmount;      // сумма кассы
    float _KKMAcashlessAmount; // сумма кассы
    float _cashAmount;         // сумма кассы нал
    float _cashlessAmount;     // сумма кассы безнал
    float _checkAmount;        // сумма чека
    float _received;           // полученная сумма
    float _change;             // сдача

    // БД товаров, истории
    ProductDB *_pdb;
    HistoryDB *_hdb;
    QSqlTableModel  *_modelP;
    QSqlTableModel  *_modelH;
    void initPDB();
    void initHDB();
    void setupModel(const QString &tableName, const QStringList &headers, QSqlTableModel *model);
    void createUI(QTableView *t, QSqlTableModel *model);

    void initKKMWindow();
    void initNewChekWindow();
    void initInspectWindow();
    void initRegistrWindow();
    void initCommandWindow();
    void addRowinCheck(const QString &name, float prise);
    void addRowinHistory(const QString &numberCheck,
                         const QString &operation,
                         const QString &pos,
                         const QString &cashAmount,
                         const QString &cashlessAmount,
                         const QString &received,
                         const QString &change,
                         const QString &fiscDoc,
                         const QString &fiscSign);
};
#endif // MAINWINDOW_H
