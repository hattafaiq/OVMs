#include "database.h"
#include "setting.h"
#include <QDate>

struct d_global global;

database::database()
{

}

void database::check_db_exist(QString filename, int count_db)
{
    QFile file(filename + ".ovm");

    if(file.exists()== true)
    {
        //QFileInfo finfo(file.fileName());
        this->load_database(filename+".ovm",count_db++);
    }
    else
    {
        this->create_database(filename + ".ovm",count_db);
        return;
    }
}

void database::create_database(QString filename, int count_db)
{
    QString con_name;
    con_name.sprintf("LOC_DB%d",count_db++);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE",con_name);
    db.setDatabaseName(filename);

    if(!db.open())
    {
        qDebug() << "db gak kebuka";
        return;
    }
    else
    {
        QSqlQuery query(db);
        create_database_awal(&query, SQLITE);
        query.finish();
        global.db = db;

    }
}
void database::load_database(QString filename, int count_db)
{
    QString con_name;
    con_name = QString("LOC_DB%1").arg(count_db);
    QSqlDatabase db =QSqlDatabase::addDatabase("QSQLITE",con_name);
    db.setDatabaseName(filename);

    if(!db.open())
    {
        qDebug()<<"db gak kebukak";
        return;
    }
    else
    {
        global.db = db;
    }

}

void database::create_table_wavefrom_harian(QSqlQuery *q, int driver)
{
    QString query;
    QString HRini = QDate::currentDate().toString("yyyyMMdd");
     qDebug() << HRini.toUtf8().data();
     if (driver==SQLITE)
     {
         //    query.sprintf("select count(name) from sqlite_master where name = 'data_waveform_%s'",hrini.toUtf8().data());
             query = QString("select count(name) from sqlite_master where name = 'data_waveform_%1'").arg(HRini.toUtf8().data());
             int jml=0;
             qDebug("q = %s",query.toUtf8().data());
             q->prepare(query);
             if(!q->exec())
             {
                 qDebug("errorrr");
             }
             else {
                 qDebug("lanjutttt");
                 q->first();
                 jml = q->value(0).toInt();
                 qDebug() << "jumlahnya =  "<< q->value(0).toInt();

             }
             q->clear();
             if (jml < 1){
                 QString aa = QString("create table data_waveform_"+HRini+ "(id integer PRIMARY KEY AUTOINCREMENT, \
                                       waktu DATETIME default(strftime('%Y%m%d%H%M%S', 'now', 'localtime')), \
                                       year DATETIME default(strftime('%Y', 'now', 'localtime')) , \
                                       month DATETIME default(strftime('%m', 'now', 'localtime')), \
                                       day DATETIME default(strftime('%d', 'now', 'localtime')), \
                                       hour DATETIME default(strftime('%H', 'now', 'localtime')), \
                                       minute DATETIME default(strftime('%M', 'now', 'localtime')), \
                                       second DATETIME default(strftime('%S', 'now', 'localtime')), \
                                       ch INTEGER, data BLOB, numsample integer, fmax integer, ref_rpm float)");

                 q->prepare(aa);
                 q->exec();
             }

    }
}

void database::create_database_awal(QSqlQuery *q, int driver)
{
   // create_table_trend_harian(q,driver);
    create_table_wavefrom_harian(q,driver);
}
