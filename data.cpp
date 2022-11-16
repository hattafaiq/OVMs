#include "data.h"
#include "setting.h"

extern struct d_global global;

data::data(QObject *parent) : QObject(parent)
{
  threadku = new threader();
  //spektrum_points = 6144;
  panjang_buffer_waveform= 10 * 10240 ;
  panjang_buffer_spektrum = MAX_FFT_POINT;
  Temp = new init_setting_k;
    QFile input("/home/fh/OVMseries/setting.ini");
    //QFile input("/home/fh/runServer/setup/setting.ini");
    if(input.exists())
    {
       cek_settings(Temp);
    }
    else
    {
       init_setting(Temp);
    }
    count_db = 1;
    pernah_penuh = 0;
    flagsave=0;
    flagtimestart=0;
    counterCH1=0;
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
    //delete Temp;
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
       // free(x1[i]);
    }
}
void data::cek_settings(init_setting_k *Temp)
{
    QString pth = "/home/fh/OVMseries/setting.ini";
   // QString pth = "/home/fh/runServer/setting.ini";
    QSettings settings(pth, QSettings::IniFormat);
    tmp.dir_ini = settings.value("Dir_ini").toString();
    tmp.modulIP1k = settings.value("IP1").toString();
    tmp.modulIP2k = settings.value("IP2").toString();
    Temp->fmax = settings.value("Fmax").toInt() ;
    Temp->timerdbk = settings.value("TimeSaveDB").toInt();
    Temp->timereq = settings.value("TimeReq").toInt();
    Temp->timerclient = settings.value("TimeClien").toInt();
    //Temp->sps = settings.value("SPS").toInt();
    Temp->line_dbSpect = settings.value("Lines").toInt();
    //Temp->jum_kanal = settings.value("JUM_KANAL").toInt();
    tmp.dir_DB = settings.value("Dir_DB").toString();
}

void data::flagdatabase()
{
    if(flagsave==1)
        {
            start_database();
            flagsave=0;
        }
    else
    {
        //qDebug()<<"data not save to database";
    }
}

void data::flagclient()
{
    flagtimestart=1;
    if(flagtimestart==1)
    {
        datamanagement();
        flagtimestart=0;
    }
    else
    {
       // qDebug()<<"data not send to client";
    }
}

void data::init_setting(init_setting_k *Temp)
{
    QString pth = "/home/fh/OVMseries/setting.ini";
    //QString pth = "/home/fh/runServer/setting.ini";
    QSettings settings(pth, QSettings::IniFormat);
    qDebug()<<"tulis";

    memset((char *) Temp, 0, sizeof (struct init_setting_k));
    tmp.dir_DB = QString::fromUtf8("/home/fh/OVMseries/ovm_dbe");
      //tmp.dir_DB = QString::fromUtf8("/home/fh/runServer/ovm_dbe");
    tmp.modulIP1k = QString::fromUtf8("192.168.0.101");
    tmp.modulIP2k = QString::fromUtf8("192.168.0.102");

    Temp->line_dbSpect = 800;
    Temp->fmax= 1000;
    Temp->timerdbk =5;
    //Temp->sps = 2560;
    Temp->timereq = 2;
    Temp->timerclient = 1;
    //Temp->jum_kanal = 8;
    //Temp->jum_kanal = settings.value("JUM_KANAL").toInt();
    //settings.setValue("JUM_KANAL",Temp->jum_kanal);
    settings.setValue("TimeClien",Temp->timerclient);
    settings.setValue("Dir_ini", tmp.dir_ini);
    settings.setValue("IP1",tmp.modulIP1k);
    settings.setValue("IP2",tmp.modulIP2k);
    settings.setValue("Dir_DB",tmp.dir_DB);
    settings.setValue("Lines",Temp->line_dbSpect);
    settings.setValue("Fmax",Temp->fmax);
    settings.setValue("TimeSaveDB",Temp->timerdbk);
    settings.setValue("TimeReq",Temp->timereq);
   // settings.setValue("SPS",Temp->sps);
}

void data::set_memory()
{
    int i;
    for (i=0; i<JUM_KANAL; i++)
    {
        data_save[i] = (float *) malloc((((2560/BESAR_PAKET_F)*JUM_KANAL)*BESAR_PAKET*Temp->timerdbk*10) * sizeof(float));
        memset( (char *) data_save[i], 0, ((((2560/BESAR_PAKET_F)*JUM_KANAL)*BESAR_PAKET*Temp->timerdbk*10)*sizeof(float)));
        data_get[i] = (float *) malloc(MAX_FFT_POINT * sizeof(float));
        memset( (char *) data_get[i], 0, MAX_FFT_POINT * sizeof(float));
        //data_prekirim[i] = (float *) malloc((((2560/BESAR_PAKET_F)*JUM_KANAL)*BESAR_PAKET*Temp->timerdbk*10)* sizeof(float));
        //memset( (char *) data_prekirim[i], 0, (((2560/BESAR_PAKET_F)*JUM_KANAL)*BESAR_PAKET*Temp->timerdbk*10) * sizeof(float));

        data_prekirim[i] = (float *) malloc(((2560*8)+(2560*4)) * sizeof(float));
        memset( (char *) data_prekirim[i], 0, ((2560*8)+(2560*4)) * sizeof(float));
    }
}


void data::init_time()
{
    timer = new QTimer(this);
    QObject::connect(timer,SIGNAL(timeout()),this, SLOT(refresh_plot()));
    timer->start(Temp->timereq*1100);
    timera = new QTimer(this);
    QObject::connect(timera,SIGNAL(timeout()),this, SLOT(start_database()));
    timera->start(5000);
    TMclient = new QTimer(this);
    QObject::connect(TMclient,SIGNAL(timeout()),this, SLOT(datamanagement()));
    TMclient->start(1000);

   // QObject::connect(this,SIGNAL(trig_client()),this, SLOT(datamanagement()));
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
    int xsps;
    int i;
    int datakebutuhan;

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
         int datasyarat= 2560;
         //int datayangdiharapkan=2048;
         penuh = (cnt_ch[i_kanal]%datasyarat)+1;
         penuh2 = (cnt_cha[i_kanal]%datasyarat)+1;

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
        for (i=0; i<PAKET_BUFF; i++)
        {
          cnt_ch[i_kanal]++;
          data_save[i_kanal][cnt_ch[i_kanal]] = p_data[i];
          cnt_cha[i_kanal]++;
          data_prekirim[i_kanal][cnt_cha[i_kanal]] = p_data[i];
        //  kris.x1[((req%10)*256+i)]=(req%10)*256+i;
          //qDebug()<<kri.x1[i];
        }

    }// while
}//void


void data::datamanagement()
{
    qDebug()<<QDateTime::currentMSecsSinceEpoch()<<"kemas";
    memcpy(kri.k1, &data_prekirim[0][1], 2560 * (sizeof(float)));
    memcpy(kri.k2, &data_prekirim[1][1], 2560 * (sizeof(float)));
    memcpy(kri.k3, &data_prekirim[2][1], 2560 * (sizeof(float)));
    memcpy(kri.k4, &data_prekirim[3][1], 2560 * (sizeof(float)));
    memcpy(kri.k5, &data_prekirim[4][1], 2560 * (sizeof(float)));
    memcpy(kri.k6, &data_prekirim[5][1], 2560 * (sizeof(float)));
    memcpy(kri.k7, &data_prekirim[6][1], 2560 * (sizeof(float)));
    memcpy(kri.k8, &data_prekirim[7][1], 2560 * (sizeof(float)));

    QByteArray datagrama = QByteArray(static_cast<char*>((void*)&kri), sizeof(kri));

    sendDataClient1(datagrama);
    qDebug()<<QDateTime::currentMSecsSinceEpoch()<<"kirim";
    for(int i =0; i<JUM_KANAL; i++)//8
    {
        cnt_cha[i] =0;
       // cnt_cha[i] =0;
    }
}

void data::start_database()
{
    for(int i =0; i<JUM_KANAL; i++)//8
    {
       // qDebug()<<cnt_cha[i];
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
           // cnt_cha[i] =0;
        }
       qDebug()<<"data save ";
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

    QByteArray ba1;
    QByteArray bal1;
    QByteArray unsub;
    QString unsub_wave1 ="unsub";
    QString wave1 ="wave1";
    QString wave2 ="wave2";
    ba1 += wave1;
    bal1 += wave2;
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
