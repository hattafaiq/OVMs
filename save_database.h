#ifndef SAVE_DATABASE_H
#define SAVE_DATABASE_H
#include <QtCore>
#include <QDebug>
#include <QString>
#include <QSqlDatabase>
#include <QFile>
#include <QSqlQuery>
#include <QSqlError>
#include <stdio.h>

class save_database
{
public:
    save_database();
    void save_database_waveform_harian(QSqlDatabase db,QByteArray bb1[],int num, int fmax, int ref_rpm, int safe_to_save_ch[]);
    //void save_database_trend_harian();
};

#endif // SAVE_DATABASE_H
