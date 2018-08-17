#include "../include/kernel.h"

Mainobj::Mainobj(QString COM)
{
    serial = new SerialPort(COM);
    comm = new Communication(COM);
    connect(comm,SIGNAL(send_S(QByteArray)),serial,SLOT(SendData(QByteArray)));
    connect(serial,SIGNAL(datecheck(QByteArray)),comm,SLOT(recvdatacheck(QByteArray)));
}
Mainobj::~Mainobj()
{

}
