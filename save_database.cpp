#include "save_database.h"
#include <QSqlError>
#include <QDate>
#include "setting.h"

save_database::save_database()
{
}

void save_database::save_database_waveform_harian(QSqlDatabase db,QByteArray bb1[],int num, int fmax, int ref_rpm,int safe_to_save_ch[])
{

QSqlQuery q(db);
QString HRini= QDate::currentDate().toString("yyyyMMdd");
QString cek = QString("select count(name) from sqlite_master where name = 'data_waveform_"+HRini+"'");

q.prepare(cek);
if(!q.exec())qDebug("error sqlite ");
q.first();
int cek_table = q.value(0).toUInt();
q.clear();
    if(cek_table == 0)
    {
       QString aa = QString("create table data_waveform_"+HRini+ "(id integer PRIMARY KEY AUTOINCREMENT, \
                            waktu DATETIME default(strftime('%Y%m%d%H%M%S', 'now', 'localtime')), \
                            year DATETIME default(strftime('%Y', 'now', 'localtime')) , \
                            month DATETIME default(strftime('%m', 'now', 'localtime')), \
                            day DATETIME default(strftime('%d', 'now', 'localtime')), \
                            hour DATETIME default(strftime('%H', 'now', 'localtime')), \
                            minute DATETIME default(strftime('%M', 'now', 'localtime')), \
                            second DATETIME default(strftime('%S', 'now', 'localtime')), \
                            ch INTEGER, data BLOB, numsample integer, fmax integer, ref_rpm integer)");
        q.prepare(aa);
        q.exec();
    }
q.clear();
    if(db.transaction())
    {
        //int idx_list_data = 0;
        for(int i=0;i<JUM_KANAL;i++)
        {
            if(safe_to_save_ch[i] == 0)
                {
                continue;
                }
            q.prepare(QString("insert into data_waveform_%1 (ch,data,numsample,fmax,ref_rpm) values (:ch, :data, :num, :fmax, :ref_rpm)").arg(HRini));
            q.bindValue(":ch",i+1);
            q.bindValue(":data",bb1[i]);
           // q.bindValue(":data",balist.at(idx_list_data));
           // idx_list_data++;
            q.bindValue(":num",num);
            q.bindValue(":fmax",fmax);
            q.bindValue(":ref_rpm",ref_rpm);

            if(q.exec())
                {
                    //qDebug()<<"===> sukses save data ke "<< i+1;
                }
            else
                {
                    qDebug()<<"===> gagal save ke "<< i+1 << q.lastError().text();
                }
//            q.clear();
            if(!db.commit())
               {
                  //  qDebug() << "Failed to commit";
                    db.rollback();
               }

       }
       }
}
