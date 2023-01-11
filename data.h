#ifndef DATA_H
#define DATA_H

#include <QUdpSocket>
#include <QTimer>
#include <QObject>
#include <QList>
#include <QByteArray>
#include <QWebSocket>
#include <QTimer>
#include <QUdpSocket>
#include <QDate>
#include <QString>
#include "QWebSocketServer"
#include "setting.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSqlDatabase>
#include <threader.h>
#include "database.h"
#include <QVector>

class data : public QObject
{
    Q_OBJECT
public:
    QByteArray simpanan;
    explicit data(QObject *parent = nullptr);
    void req_UDP();
    virtual ~data();
    unsigned short spsX;
    struct kirim_fmax1000 fmax_1000;
    struct kirim_fmax4000 fmax_4000;
    struct kirims kris;
    QString pesan_topik;
    int xsps;
    void set_memory();
    void free_memory();
    void init_setting();
    void cek_settings();

Q_SIGNALS:
    void closed();

public slots:
    void readyReady(); //(QByteArray datagram);//data tidak mau masuk
    void init_time();
    void refresh_plot();
    void start_database();


private slots:
    void onNewConnection();
    void processMessage(QByteArray message);
    void socketDisconnected();
    void showTime();
    void sendDataClient1(QByteArray isipesan);
    void data_kirim();


private:
    //
    float last_data_check1;
    float last_data_check2;
    QVector <float> vec;
    int flagsave;
    int flagtimestart;

    int kirimclient;
    int penuh;
    int penuh2;
    unsigned int panjang_buffer_waveform;
    unsigned int panjang_buffer_spektrum;
    int period_simpan;
    int spektrum_points;
    int paket_dikirim;
    float *data_save[JUM_KANAL];
    float *data_get[JUM_KANAL];
    float *data_prekirim[JUM_KANAL];
    int x1[set_up];
    int cnt_ch[JUM_KANAL];
    int cnt_cha[JUM_KANAL];

    int counterCH;
    ///
    QTimer *timer;
    QTimer *timera;
    QTimer *TMclient;

    //websocket
    QWebSocketServer *m_pWebSocketServer1;
    QList<QWebSocket *> Subcribe_wave1;
    QWebSocket *C_NewCon;

    QWebSocket *pClient1;
    QWebSocket *pClientkirim;

    QString *datas;
    QTimer *jam;
    QTimer *timers;
    QDate date;
    QString dateTimeText;
    QString time_text;
    //
    //parsing UDP
    QByteArray datagram;
    quint16 senderPort;
    QHostAddress sendera;
    QUdpSocket *socket;
    QUdpSocket *cek_koneksi;
    int tim_count;
    // inisial data
    double *data_y_voltage[JUM_KANAL];
    threader *threadku;
    database *dbase;
    int count_db;
    //----------------------------------------------//
    int syarat_data;
    int paket_diharapkan;
    int datasyarat;

    int modul_1_penuh;
    int modul_2_penuh;


};
#endif // DATA_H
