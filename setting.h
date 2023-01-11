#ifndef SETTING_H
#define SETTING_H

#include <QSqlQuery>
#include <QSqlDatabase>
#include <QHostAddress>

#define alamat "/OVMSsetting.ini"

//#define SUMBU_FONT  "Helvetica"
#define set_up          3840//5120
#define JUM_MODULE      2
#define JUM_KANAL       8

#define JUM_BAND_AREA   5

#define MAX_LEN_DATA    100000  /* 100 k Sample x 8 byte double */
#define LEN_DUMMY       5000
#define SET_CEK         "daunbiru_maumere_01"

#define BESAR_PAKET     1024 //2048
#define PAKET_10        2560
#define PAKET_BUFF      256
#define BESAR_PAKET_F	(BESAR_PAKET / 4)
#define TIME_FRAME1s    1000
#define TIME_REQ        2200

/* Database Driver */
#define SQLITE          1
#define MYSQL           2
#define POSTGRESQL      3

//KUKU
#define LEN_TEST        2560

#define MAX_FFT_POINT   16540 /* Max line : 6400 --> spektrum_point untuk fft = line * const Nyquist
                                                                               = 6400 * 2.56 = 16284
                                  16284 + 256 (biar ga mepet) */

#define PORT_CILIWUNG_SEND       5006
#define PORT_CILIWUNG_RECV       5007
#define PORT_CILIWUNG_DATA       5008

#define sps_fmax1000 2560
#define sps_fmax4000 10240


struct tt_req2 {
    char head[10];
    unsigned short sps;//KUKU
    char cur_kanal;
    int kanal_enable;       /* jumlah kanal aktif */
    int request_sample;		/* sample diminta */
    unsigned char buf[ BESAR_PAKET ];
};

struct d_global{
    QSqlDatabase db;
};

struct init_setting_k{
    int timerdbk;
    int fmax;
    int line_dbSpect;
    QString dir_ini;
    QString modulIP1k;
    QString modulIP2k;
    QString dir_DB;
    int sps;
    int timereq;
    int timerclient;
    int jum_kanal;
    char cek_set[32];
};

struct filepesan
{
    unsigned char a;
};

struct kirim_fmax1000
{
    float k1[sps_fmax1000];
    float k2[sps_fmax1000];
    float k3[sps_fmax1000];
    float k4[sps_fmax1000];
    float k5[sps_fmax1000];
    float k6[sps_fmax1000];
    float k7[sps_fmax1000];
    float k8[sps_fmax1000];
    int info_sps;
    int modul_aktif;
};

struct kirim_fmax4000
{
    float k1[sps_fmax4000];
    float k2[sps_fmax4000];
    float k3[sps_fmax4000];
    float k4[sps_fmax4000];
    float k5[sps_fmax4000];
    float k6[sps_fmax4000];
    float k7[sps_fmax4000];
    float k8[sps_fmax4000];
    int info_sps;
    int modul_aktif;
};

struct kirims
{
    float k1[JUM_KANAL][2560];
};

#endif // SETTING_H
