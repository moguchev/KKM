#ifndef PRODUCTDB_H
#define PRODUCTDB_H

#include <QObject>
#include <QSql>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDatabase>
#include <QFile>
#include <QDate>
#include <QDebug>

/* Директивы имен таблицы, полей таблицы и базы данных */
#define DATABASE_HOSTNAME   "ProdDB"
#define DATABASE_NAME       "product.db"

#define TABLE_PRODUCT           "TableProduct"
#define TABLE_ID                "ID"
#define TABLE_BARCODE           "Barcode"
#define TABLE_NAME              "Name"
#define TABLE_PRICE             "Prise"

class ProductDB : public QObject
{
    Q_OBJECT
public:
    explicit ProductDB(QObject *parent = 0);
    ~ProductDB() = default;
    /* Методы для непосредственной работы с классом
     * Подключение к базе данных и вставка записей в таблицу
     * */
    void connectToDataBase();
    bool inserIntoTable(const QVariantList &data);

private:
    // Сам объект базы данных, с которым будет производиться работа
    QSqlDatabase    db;

private:
    /* Внутренние методы для работы с базой данных
     * */
    bool openDataBase();
    bool restoreDataBase();
    void closeDataBase();
    bool createTable();
};

#endif // PRODUCTDB_H
