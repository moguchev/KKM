#include "productdb.h"

ProductDB::ProductDB(QObject *parent) : QObject(parent) {

}


/* Методы для подключения к базе данных
 * */
void ProductDB::connectToDataBase() {
    /* Перед подключением к базе данных производим проверку на её существование.
     * В зависимости от результата производим открытие базы данных или её восстановление
     * */
    if (!QFile("C:/Users/Sergei/Documents/KKM/" DATABASE_NAME).exists()){
        this->restoreDataBase();
    } else {
        this->openDataBase();
    }
}

/* Методы восстановления базы данных
 * */
bool ProductDB::restoreDataBase() {
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
bool ProductDB::openDataBase()
{
    /* База данных открывается по заданному пути
     * и имени базы данных, если она существует
     * */
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
void ProductDB::closeDataBase()
{
    db.close();
}

/* Метод для создания таблицы в базе данных
 * */
bool ProductDB::createTable()
{
    /* В данном случае используется формирование сырого SQL-запроса
     * с последующим его выполнением.
     * */
    QSqlQuery query;
    if(!query.exec( "CREATE TABLE " TABLE_PRODUCT " ("
                            TABLE_ID        " INTEGER         NOT NULL,"
                            TABLE_BARCODE   " VARCHAR(255)    NOT NULL,"
                            TABLE_NAME      " VARCHAR(255)    NOT NULL,"
                            TABLE_PRICE     " FLOAT           NOT NULL"
                        " )"
                    )){
        qDebug() << "DataBase: error of create " << TABLE_PRODUCT;
        qDebug() << query.lastError().text();
        return false;
    } else {
        return true;
    }
    return false;
}

/* Метод для вставки записи в базу данных"
""
""
 * */
bool ProductDB::inserIntoTable(const QVariantList &data)
{
    /* Запрос SQL формируется из QVariantList,
     * в который передаются данные для вставки в таблицу.
     * */
    QSqlQuery query;
    /* В начале SQL запрос формируется с ключами,
     * которые потом связываются методом bindValue
     * для подстановки данных из QVariantList
     * */
    query.prepare("INSERT INTO " TABLE_PRODUCT " ( " TABLE_ID ", "
                                                     TABLE_NAME ", "
                                                     TABLE_BARCODE ", "
                                                     TABLE_PRICE " ) "
                  "VALUES (:ID, :Barcode, :Name, :Prise )");
    query.bindValue(":ID",       data[0].toInt());
    query.bindValue(":Barcode",  data[1].toString());
    query.bindValue(":Name",     data[2].toString());
    query.bindValue(":Prise",    data[3].toFloat());
    // После чего выполняется запросом методом exec()
    if(!query.exec()){
        qDebug() << "error insert into " << TABLE_PRODUCT;
        qDebug() << query.lastError().text();
        return false;
    } else {
        return true;
    }
    return false;
}
