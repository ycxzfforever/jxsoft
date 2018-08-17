#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include <QObject>
#include<QTimer>
#include <QtSql/QSqlDatabase>
#include "serial.h"

typedef struct Globar{
//    QTimer *timer;
//    uchar   sendflag;//0：串口未发完数据，1：串口已发完数据
//    qint32  id;
}globar;

extern globar globarvar;

class Communication: public QObject
{
    Q_OBJECT
public:
    Communication(QString COM);
    ~Communication();
    QTimer *timer;
    qint16              GunNum;  //串口对应的枪号
    QByteArray CRC16Calculate(QByteArray data);
    void dataanalysis(QByteArray data);//数据正确时，解析数据
    void recvrealtime51H(QByteArray data);//加气机无数据下载或无记录上传时回复
    void recvrecord52H(QByteArray data);//接收52流水记录
    void sendrecord52H(QByteArray data, bool type);//发送流水确认命令

signals:
    void send_S(QByteArray recvdata);
private slots:
    void sendrollcall();
    void recvdatacheck(QByteArray data);
};


#endif // COMMUNICATION_H
