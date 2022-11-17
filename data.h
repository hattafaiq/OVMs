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
    struct kirim kri;
    struct kirims kris;
    void set_memory();
    void free_memory();
    void init_setting(init_setting_k *Temp);
    void cek_settings(init_setting_k *Temp);
    //struct init_setting_k *Temp;
    QVector<float> y_bisa[8][3840];

Q_SIGNALS:
    void closed();
    void trig_client();
    void kirim();

public slots:
    void readyReady(); //(QByteArray datagram);//data tidak mau masuk
    void init_time();
    void refresh_plot();
    void datamanagement();
    void start_database();
    void flagdatabase();
    void flagclient();



private slots:
    void onNewConnection();
    void processMessage(QByteArray message);
    void socketDisconnected();
    void showTime();
    void sendDataClient1(QByteArray isipesan);
   // void sendDataClient2(QString isipesan2);


private:
    //
    QVector <float> vec;
    struct init_setting_k tmp, *Temp;
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
    //float *data_prekirim[8];
    float *data_prekirim[JUM_KANAL];
    int x1[set_up];
    int cnt_ch[JUM_KANAL];
    int cnt_cha[JUM_KANAL];

    int counterCH1;
    ///
    //int pernah_penuh;
    //timer
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
    int tim_count;
    // inisial data
    double *data_y_voltage[JUM_KANAL];
    threader *threadku;
    database *dbase;
    int count_db;
    //----------------------------------------------//
    int pernah_penuh[JUM_KANAL];
    int paket_datang[JUM_KANAL];
    int counter_paket[JUM_KANAL];

    int penuh_vel1[JUM_KANAL];
    int paket_vel1[JUM_KANAL];
    int counter_vel1[JUM_KANAL];

    int penuh_hpf[JUM_KANAL];
    int paket_hpf[JUM_KANAL];
    int counter_hpf[JUM_KANAL];

    int syarat_data;
    int paket_diharapkan;
    int datasyarat;

    int modul_1_penuh;
    int modul_2_penuh;


};
#endif // DATA_H
