#ifndef DATABASE_H
#define DATABASE_H

#include <QDebug>
#include <QString>
#include <QSqlDatabase>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <stdio.h>

class database
{
public:
    database();
    void check_db_exist(QString filename, int count_db);
    void create_database(QString filename, int count_db);
    void load_database(QString filename, int count_db);

    void create_database_awal(QSqlQuery *q, int driver);
    void create_table_wavefrom_harian(QSqlQuery *q, int driver);
    //void create_table_trend_harian(QSqlQuery *q, int driver);
};

#endif // DATABASE_H
