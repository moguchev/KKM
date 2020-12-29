#include "historydb.h"

HistoryDB::HistoryDB(QObject *parent) : QObject(parent)
{

}

/* Методы для подключения к базе данных
 * */
void HistoryDB::connectToDataBase()
{
    if(!QFile("C:/Users/Sergei/Documents/KKM/" DATABASE_NAME).exists()){
        this->restoreDataBase();
    } else {
        this->openDataBase();
    }
}

/* Методы восстановления базы данных
 * */
bool HistoryDB::restoreDataBase()
{
    if(this->openDataBase()){
        if(!this->createTable()){
            return false;
        } else {
            return true;
        }
    } else {
        qDebug() << "Не удалось восстановить базу данных";
        return false;
    }
    return false;
}

/* Метод для открытия базы данных
 * */
bool HistoryDB::openDataBase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setHostName(DATABASE_HOSTNAME);
    db.setDatabaseName("C:/Users/Sergei/Documents/KKM/" DATABASE_NAME);
    if(db.open()){
        return true;
    } else {
        return false;
    }
}

/* Методы закрытия базы данных
 * */
void HistoryDB::closeDataBase()
{
    db.close();
}

/* Метод для создания таблицы в базе данных
 * */
bool HistoryDB::createTable()
{    
    QSqlQuery query;
    QString temp = "CREATE TABLE " TABLE_HISTORY " ("
            TABLE_CREATE          " VARCHAR(255)    NOT NULL,"
            TABLE_CHECK_NUMBER    " VARCHAR(255)    NOT NULL,"
            TABLE_OPERATION       " VARCHAR(255)    NOT NULL,"
            TABLE_POSITIONS       " VARCHAR(255)    NOT NULL,"
            TABLE_CASH_AMOUNT     " VARCHAR(255)    NOT NULL,"
            TABLE_CASHLESS_AMOUNT " VARCHAR(255)    NOT NULL,"
            TABLE_RECEIVED        " VARCHAR(255)    NOT NULL,"
            TABLE_CHANGE          " VARCHAR(255)    NOT NULL,"
            TABLE_FISC_DOC        " VARCHAR(255)    NOT NULL,"
            TABLE_FISC_SIGN       " VARCHAR(255)    NOT NULL"
        " )";
    if (!query.exec(temp)) {
        qDebug() <<temp;
        qDebug() << "DataBase: error of create " << TABLE_HISTORY;
        qDebug() << query.lastError().text();
        return false;
    } else {
        return true;
    }
    return false;
}

/* Метод для вставки записи в базу данных
 * */
bool HistoryDB::inserIntoTable(const QVariantList &data)
{
    QSqlQuery query;
    query.prepare("INSERT INTO " TABLE_HISTORY " ( " TABLE_CREATE ", "
                                             TABLE_CHECK_NUMBER ", "
                                             TABLE_OPERATION ", "
                                             TABLE_POSITIONS ", "
                                             TABLE_CASH_AMOUNT ", "
                                             TABLE_CASHLESS_AMOUNT ", "
                                             TABLE_RECEIVED ", "
                                             TABLE_CHANGE ", "
                                             TABLE_FISC_DOC ", "
                                             TABLE_FISC_SIGN " ) "
                  "VALUES (:TimeCreate, :CheckNumber, :Operation, :Positions, "
                  ":CashAmount, :CashlessAmount, :Received, :Change, :FiscDoc, :FiscSign )");
    query.bindValue(":TimeCreate",     data[0].toString());
    query.bindValue(":CheckNumber",    data[1].toString());
    query.bindValue(":Operation",      data[2].toString());
    query.bindValue(":Positions",      data[3].toString());
    query.bindValue(":CashAmount",     data[4].toString());
    query.bindValue(":CashlessAmount", data[5].toString());
    query.bindValue(":Received",       data[6].toString());
    query.bindValue(":Change",         data[7].toString());
    query.bindValue(":FiscDoc",        data[8].toString());
    query.bindValue(":FiscSign",       data[9].toString());

    if(!query.exec()){
        qDebug() << "error insert into " << TABLE_HISTORY;
        qDebug() << query.lastError().text();
        return false;
    } else {
        return true;
    }
    return false;
}
