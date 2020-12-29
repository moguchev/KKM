#ifndef HISTORYDB_H
#define HISTORYDB_H

#include <QObject>
#include <QSql>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QFile>
#include <QDate>
#include <QDebug>

/* Директивы имен таблицы, полей таблицы и базы данных */
#define DATABASE_HOSTNAME   "HistoryDB"
#define DATABASE_NAME       "history.db"

#define TABLE_HISTORY           "HistoryDB"
#define TABLE_CREATE            "TimeCreate"
#define TABLE_CHECK_NUMBER      "CheckNumber"
#define TABLE_OPERATION         "Operation"
#define TABLE_POSITIONS         "Positions"
#define TABLE_CASH_AMOUNT       "CashAmount"
#define TABLE_CASHLESS_AMOUNT   "CashlessAmount"
#define TABLE_RECEIVED          "Received"
#define TABLE_CHANGE            "Change"
#define TABLE_FISC_DOC          "FiscDoc"
#define TABLE_FISC_SIGN         "FiscSign"

class HistoryDB : public QObject
{
    Q_OBJECT
public:
    explicit HistoryDB(QObject *parent = 0);
    ~HistoryDB() = default;
    /* Методы для непосредственной работы с классом
     * Подключение к базе данных и вставка записей в таблицу
     * */
    void connectToDataBase();
    bool inserIntoTable(const QVariantList &data);

private:
    // Сам объект базы данных, с которым будет производиться работа
    QSqlDatabase db;

private:
    /* Внутренние методы для работы с базой данных
     * */
    bool openDataBase();
    bool restoreDataBase();
    void closeDataBase();
    bool createTable();
};

#endif // HISTORYDB_H
