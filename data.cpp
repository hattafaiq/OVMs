#include "data.h"
#include "setting.h"

extern struct d_global global;

data::data(QObject *parent) : QObject(parent)
{
  threadku = new threader();
  Temp = new init_setting_k;
    QFile input("/home/fh/Server_OVM/setting.ini");
    if(input.exists())
    {
       cek_settings(Temp);
    }
    else
    {
       init_setting(Temp);
    }
    count_db = 1;
   // pernah_penuh = 0;
    flagsave=0;
    flagtimestart=0;
    counterCH=0;
    kirimclient = 0;
    spektrum_points = 2.56 * Temp->line_dbSpect;
    //INIT_udp
    socket = new QUdpSocket(this);
    qDebug()<<"status awal "<<socket->state();
    socket->bind(QHostAddress::Any, 5008);
    qDebug()<<"status bind "<<socket->state();
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyReady()));
    qDebug()<<"status connect "<<socket->state();
    //INIT_websocket
    m_pWebSocketServer1 = new QWebSocketServer(QStringLiteral("OVM"),QWebSocketServer::NonSecureMode,this);
    m_pWebSocketServer1->listen(QHostAddress::Any, 2121);
    connect(m_pWebSocketServer1, SIGNAL(newConnection()),this, SLOT(onNewConnection()));
    connect(m_pWebSocketServer1, &QWebSocketServer::closed, this, &data::closed);
    //INIT_database
    dbase->check_db_exist(tmp.dir_DB,count_db);

    set_memory();
    for(int i =0; i<JUM_KANAL;i++)
    {
        cnt_ch[i] = 0;
        cnt_cha[i]=0;
    }
        init_time();
}

data::~data()
{
    m_pWebSocketServer1->close();
    qDeleteAll(Subcribe_wave1.begin(), Subcribe_wave1.end());//paket10
    delete threadku;
    free_memory();
}
void data::free_memory()
{
    int i;
    for (i = 0; i < JUM_KANAL; i++)
    {
        free(data_save[i]);
        free(data_get[i]);
        free(data_prekirim[i]);
    }
}
void data::cek_settings(init_setting_k *Temp)
{
    QString pth = "/home/fh/Server_OVM/setting.ini";
    QSettings settings(pth, QSettings::IniFormat);
    tmp.modulIP1k = settings.value("IP1").toString();
    tmp.modulIP2k = settings.value("IP2").toString();
    Temp->fmax = settings.value("Fmax").toInt() ;
    Temp->timerdbk = settings.value("TimeSaveDB").toInt();
    Temp->timereq = settings.value("TimeReq").toInt();
    Temp->timerclient = settings.value("TimeClien").toInt();
    Temp->line_dbSpect = settings.value("Lines").toInt();
    tmp.dir_DB = settings.value("Dir_DB").toString();
}


void data::init_setting(init_setting_k *Temp)
{
    QString pth = "/home/fh/Server_OVM/setting.ini";
    QSettings settings(pth, QSettings::IniFormat);
    qDebug()<<"tulis";

    memset((char *) Temp, 0, sizeof (struct init_setting_k));
    tmp.dir_DB = QString::fromUtf8("/home/fh/Server_OVM/ovm_dbe");
    tmp.modulIP1k = QString::fromUtf8("192.168.0.101");
    tmp.modulIP2k = QString::fromUtf8("192.168.0.102");

    Temp->line_dbSpect = 800;
    Temp->fmax= 1000;
    Temp->timerdbk =5;
    Temp->timereq = 2;
    Temp->timerclient = 1;
    settings.setValue("TimeClien",Temp->timerclient);
    settings.setValue("IP1",tmp.modulIP1k);
    settings.setValue("IP2",tmp.modulIP2k);
    settings.setValue("Dir_DB",tmp.dir_DB);
    settings.setValue("Lines",Temp->line_dbSpect);
    settings.setValue("Fmax",Temp->fmax);
    settings.setValue("TimeSaveDB",Temp->timerdbk);
    settings.setValue("TimeReq",Temp->timereq);
}

void data::set_memory()
{
    int i;
    for (i=0; i<JUM_KANAL; i++)
    {
        data_save[i] = (float *) malloc(((sps_fmax4000*JUM_KANAL)*5) * sizeof(float));
        memset( (char *) data_save[i], 0, (((sps_fmax4000*JUM_KANAL)*5)*sizeof(float)));
        data_get[i] = (float *) malloc(((sps_fmax4000*JUM_KANAL)*5) * sizeof(float));
        memset( (char *) data_get[i], 0, ((sps_fmax4000*JUM_KANAL)*5) * sizeof(float));
        data_prekirim[i] = (float *) malloc(((sps_fmax4000*JUM_KANAL)*5) * sizeof(float));
        memset( (char *) data_prekirim[i], 0, ((sps_fmax4000*JUM_KANAL)*5) * sizeof(float));
    }
}


void data::init_time()
{ 
    modul_1_penuh=0;
    modul_2_penuh=0;
    timer = new QTimer(this);
    QObject::connect(timer,SIGNAL(timeout()),this, SLOT(refresh_plot()));
    timer->start(Temp->timereq*1100);
    timera = new QTimer(this);
    QObject::connect(timera,SIGNAL(timeout()),this, SLOT(start_database()));
    timera->start(5000);
    TMclient = new QTimer(this);
    QObject::connect(TMclient,SIGNAL(timeout()),this, SLOT(data_kirim()));
    TMclient->start(1000);
    //QObject::connect(this,SIGNAL(kirim()),this,SLOT(datamanagement()));
}

void data::req_UDP()
{
    QByteArray Data;
    Data.append("getdata");
    QHostAddress ip_modul_1, ip_modul_2;
    ip_modul_1.setAddress(tmp.modulIP1k);
    ip_modul_2.setAddress(tmp.modulIP2k);
    socket->writeDatagram(Data,ip_modul_1, 5006);
    socket->writeDatagram(Data,ip_modul_2, 5006);
}

void data::showTime()
{
    QTime time = QTime::currentTime();
    time_text = time.toString("hh:mm:ss:z");
}


void data::readyReady()
{
    struct tt_req2 *p_req2;
    float *p_data;
    int i_kanal;
    int req;
    int i;

    while (socket->hasPendingDatagrams())
    {
        datagram.resize(socket->pendingDatagramSize());
        socket->readDatagram(datagram.data(), datagram.size(), &sendera, &senderPort);
        QHostAddress ip_modul_1, ip_modul_2;
        ip_modul_1.setAddress(tmp.modulIP1k);
        ip_modul_2.setAddress(tmp.modulIP2k);
         p_req2 = (struct tt_req2 *) datagram.data();
         p_data = (float *) p_req2->buf;
         req = p_req2->request_sample;
         i_kanal = p_req2->cur_kanal;
         xsps = p_req2->sps;
         fmax_1000.info_sps = p_req2->sps;
         fmax_4000.info_sps = p_req2->sps;
         int no_module = -1;

         if(sendera.toIPv4Address() == ip_modul_1.toIPv4Address())
         {
             no_module = 0;
         }
         else if(sendera.toIPv4Address() == ip_modul_2.toIPv4Address())
         {
             i_kanal = i_kanal+4;
             no_module = 1;      
         }
         ////////////////////////////////////////////////////////////////////////////////////////////////////////////
         for (i=0; i<PAKET_BUFF; i++)
        {
          cnt_ch[i_kanal]++;
          data_save[i_kanal][cnt_ch[i_kanal]] = p_data[i];
          cnt_cha[i_kanal]++;
          data_prekirim[i_kanal][cnt_cha[i_kanal]] = p_data[i];
        }

    }// while
//    if(xsps==sps_fmax1000)
//    {
//        if(cnt_cha[3]==sps_fmax1000)
//        {
//            modul_1_penuh=1;
//            memcpy(fmax_1000.k1, &data_prekirim[0][1], sps_fmax1000 * (sizeof(float)));
//            memcpy(fmax_1000.k2, &data_prekirim[1][1], sps_fmax1000 * (sizeof(float)));
//            memcpy(fmax_1000.k3, &data_prekirim[2][1], sps_fmax1000 * (sizeof(float)));
//            memcpy(fmax_1000.k4, &data_prekirim[3][1], sps_fmax1000 * (sizeof(float)));
//         }
//         else if(cnt_cha[7]==sps_fmax1000)
//         {
//            modul_2_penuh=1;
//            memcpy(fmax_1000.k5, &data_prekirim[4][1], sps_fmax1000 * (sizeof(float)));
//            memcpy(fmax_1000.k6, &data_prekirim[5][1], sps_fmax1000 * (sizeof(float)));
//            memcpy(fmax_1000.k7, &data_prekirim[6][1], sps_fmax1000 * (sizeof(float)));
//            memcpy(fmax_1000.k8, &data_prekirim[7][1], sps_fmax1000 * (sizeof(float)));
//          }
//     }
//     else
//     {
//        if(cnt_cha[3]==sps_fmax4000)
//        {
//            modul_1_penuh=1;
//            memcpy(fmax_4000.k1, &data_prekirim[0][1], sps_fmax4000 * (sizeof(float)));
//            memcpy(fmax_4000.k2, &data_prekirim[1][1], sps_fmax4000 * (sizeof(float)));
//            memcpy(fmax_4000.k3, &data_prekirim[2][1], sps_fmax4000 * (sizeof(float)));
//            memcpy(fmax_4000.k4, &data_prekirim[3][1], sps_fmax4000 * (sizeof(float)));
//            fmax_4000.modul_aktif = 1;
//         }
//         else if(cnt_cha[7]==sps_fmax4000)
//         {
//            modul_2_penuh=1;
//            memcpy(fmax_4000.k5, &data_prekirim[4][1], sps_fmax4000 * (sizeof(float)));
//            memcpy(fmax_4000.k6, &data_prekirim[5][1], sps_fmax4000 * (sizeof(float)));
//            memcpy(fmax_4000.k7, &data_prekirim[6][1], sps_fmax4000 * (sizeof(float)));
//            memcpy(fmax_4000.k8, &data_prekirim[7][1], sps_fmax4000 * (sizeof(float)));
//            fmax_4000.modul_aktif = 2;
//          }
//    }

//    if((modul_1_penuh&&!modul_2_penuh)||(modul_1_penuh&&modul_2_penuh))
//    {
//        qDebug()<<QDateTime::currentMSecsSinceEpoch()<<"kemas";
//        if(xsps==sps_fmax1000)
//        {
//            if(modul_1_penuh){
//                fmax_1000.modul_aktif =1;
//            }
//            if(modul_2_penuh){
//                fmax_1000.modul_aktif =2;
//            }
//            QByteArray datagrama = QByteArray(static_cast<char*>((void*)&fmax_1000), sizeof(fmax_1000));
//            sendDataClient1(datagrama);
//            qDebug()<<QDateTime::currentMSecsSinceEpoch()<<"kirim";
//            for(i =0; i<JUM_KANAL; i++)//8
//            {
//                qDebug()<<" kanal data= "<<cnt_cha[i];
//                cnt_cha[i]=0;
//            }
//            modul_1_penuh=0;
//            modul_2_penuh=0;
//        }
//        else
//        {
//            QByteArray datagrama = QByteArray(static_cast<char*>((void*)&fmax_4000), sizeof(fmax_4000));
//            sendDataClient1(datagrama);
//            qDebug()<<QDateTime::currentMSecsSinceEpoch()<<"kirim";
//            for(i =0; i<JUM_KANAL; i++)//8
//            {
//                qDebug()<<" kanal data= "<<cnt_cha[i];
//                cnt_cha[i]=0;
//            }
//            modul_1_penuh=0;
//            modul_2_penuh=0;
//        }

//    }
}//void

void data::data_kirim()
{
    memcpy(fmax_1000.k1, &data_prekirim[0][1], sps_fmax1000 * (sizeof(float)));
    memcpy(fmax_1000.k2, &data_prekirim[1][1], sps_fmax1000 * (sizeof(float)));
    memcpy(fmax_1000.k3, &data_prekirim[2][1], sps_fmax1000 * (sizeof(float)));
    memcpy(fmax_1000.k4, &data_prekirim[3][1], sps_fmax1000 * (sizeof(float)));
    memcpy(fmax_1000.k5, &data_prekirim[4][1], sps_fmax1000 * (sizeof(float)));
    memcpy(fmax_1000.k6, &data_prekirim[5][1], sps_fmax1000 * (sizeof(float)));
    memcpy(fmax_1000.k7, &data_prekirim[6][1], sps_fmax1000 * (sizeof(float)));
    memcpy(fmax_1000.k8, &data_prekirim[7][1], sps_fmax1000 * (sizeof(float)));
    QByteArray datagrama = QByteArray(static_cast<char*>((void*)&fmax_1000), sizeof(fmax_1000));
    sendDataClient1(datagrama);
    qDebug()<<QDateTime::currentMSecsSinceEpoch()<<"kirim";
    for(int i =0; i<JUM_KANAL; i++)//8
    {
        qDebug()<<" kanal data= "<<cnt_cha[i];
        cnt_cha[i]=0;
    }
}



void data::start_database()
{
    for(int i =0; i<JUM_KANAL; i++)//8
    {
        if(cnt_ch[i] < spektrum_points)
        {
            threadku->safe_to_save_ch[i] = 0;
            continue;
        }
        else
        {
            threadku->safe_to_save_ch[i] = 1;
        }
        memcpy(&data_get[i][0], &data_save[i][cnt_ch[i]-(spektrum_points)], spektrum_points * (sizeof(float)));
        QByteArray array0((char *) &data_get[i][0], spektrum_points * sizeof(float));
        threadku->bb1[i] = array0;
        threadku->ref_rpm = 6000;
        threadku->num = spektrum_points;
        threadku->fmax = Temp->fmax;

        threadku->start();

        array0.clear();
        }
        for(int i =0; i<JUM_KANAL; i++)//8
        {
            cnt_ch[i] =0;
        }
       qDebug()<<"data save ";
//       cek_koneksi = new QUdpSocket(this);
//       qDebug()<<"status udp: "<<cek_koneksi->state();
//       if(cek_koneksi->state() == cek_koneksi->UnconnectedState)
//       {
//            QByteArray info_udp;
//            QString los= "udp_loss";
//            info_udp += los;
//            sendDataClient1(info_udp);
//            qDebug()<<"send to client data loss";
//       }
//       cek_koneksi->deleteLater();
}

void data::refresh_plot()
{
    req_UDP();
    tim_count++;
}

void data::onNewConnection()
{
    C_NewCon = m_pWebSocketServer1->nextPendingConnection();
    connect(C_NewCon, &QWebSocket::binaryMessageReceived, this, &data::processMessage);
    connect(C_NewCon, &QWebSocket::disconnected, this, &data::socketDisconnected);
}

void data::processMessage(QByteArray message)
{
    QWebSocket *C_NewReq = qobject_cast<QWebSocket *>(sender());
       // qDebug()<<message;

    QByteArray ba1;;
    QByteArray unsub;
    QString unsub_wave1 ="unsub";
    if(xsps==2560)
    {
       pesan_topik ="fmax1000";
    }
    else
    {
        pesan_topik ="fmax4000";
    }
    ba1 += pesan_topik;
    unsub += unsub_wave1;

    if((C_NewReq)&&(message==ba1))
    {
        Subcribe_wave1.removeOne(C_NewReq);
        Subcribe_wave1 << C_NewReq;
        qDebug()<<"req wave 1 dari:"<<C_NewReq->peerAddress().toString();
    }
    if((C_NewReq)&&(message==unsub))
    {
        Subcribe_wave1.removeOne(C_NewReq);
        qDebug()<<"unsub scribe dari:"<<C_NewReq->peerAddress().toString();
    }

}

void data::sendDataClient1(QByteArray isipesan)
{
    Q_FOREACH (pClientkirim, Subcribe_wave1)//paket10
    {
        QHostAddress join=pClientkirim->peerAddress();
        QString joinstr=join.toString();
        qDebug() << "kirim paket 1----ke : "<<joinstr;
        pClientkirim->sendBinaryMessage(isipesan);
    }
}


void data::socketDisconnected()
{
    pClient1 = qobject_cast<QWebSocket *>(sender());
    if (pClient1)
    {
        Subcribe_wave1.removeOne(pClient1);//paket10
        pClient1->deleteLater();
        QHostAddress join=pClient1->peerAddress();
        QString loststr=join.toString();
        qDebug()<<"client loss" << loststr;
    }
}
