#include "../include/kernel.h"
QT_USE_NAMESPACE

SerialPort::SerialPort(QString ComNum)
{
    ReadComConf(ComNum);
	serialPort = new QSerialPort;
	serialPort->setPortName(PortName);
	if (!serialPort->open(QIODevice::ReadWrite))
    {
		qCritical() << QObject::tr("Failed to open port %1, error: %2").arg(PortName).arg(serialPort->errorString());
	}
	else
	{
		serialPort->setBaudRate(BaudRate);
		serialPort->setParity(Parity);
		serialPort->setDataBits(DataBits);
		serialPort->setStopBits(StopBits);
		serialPort->setFlowControl(FlowControl);
		qDebug()<<QObject::tr("Open %1 success").arg(PortName);
	}
    readDataBuf.clear();    
    connect(serialPort, SIGNAL(readyRead()),this, SLOT(RecvData()));
    connect(serialPort, SIGNAL(bytesWritten(qint64)),this, SLOT(handleBytesWritten(qint64)));
    connect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)),this,SLOT(handleError(QSerialPort::SerialPortError)));
}

SerialPort::~SerialPort()
{
     serialPort->close();
}

void SerialPort::RecvData()
{
    unsigned char len;
    readDataBuf.append(serialPort->readAll());
    if(readDataBuf.size()>6)
    {
        len=readDataBuf.at(6)+11;//消息体长度+帧头帧尾和crc
    }
    if(len<11) return;
    if(readDataBuf.size()<len) return;
    qDebug()<<QObject::tr("%1 Recv<<<<").arg(PortName)+ByteArrayToString(readDataBuf,'.');
    emit datecheck(readDataBuf);
    len=0;
    readDataBuf.clear();
}

void SerialPort::handleError(QSerialPort::SerialPortError serialPortError)
{
	if (serialPortError == QSerialPort::ReadError)
	{
		qCritical() << QObject::tr("An I/O error occurred while reading the data from port %1, error: %2").arg(serialPort->portName()).arg(serialPort->errorString());
	}
}

void SerialPort::SendData(QByteArray data)
{
    qDebug()<<QObject::tr("%1 Send>>>>").arg(PortName)+ByteArrayToString(data,'.');
    qint64 bytesWritten = serialPort->write((data));
    sendsize=data.size();
	if (bytesWritten == -1)
	{
		qCritical() << QObject::tr("Failed to write the data to port %1, error: %2").arg(serialPort->portName()).arg(serialPort->errorString());
	}
    else if (bytesWritten != data.size())
	{
		qCritical() << QObject::tr("Failed to write all the data to port %1, error: %2").arg(serialPort->portName()).arg(serialPort->errorString());
	}    
}

void SerialPort::handleBytesWritten(qint64 bytes)
{
	if (bytes != sendsize)
	{
        qCritical() << QObject::tr("Data fail to sent port %1 send=%2,all=%3").arg(serialPort->portName()).arg(sendsize).arg(bytes) ;
	}
}


void SerialPort::ReadComConf(QString COM)
{
	int var;
	QByteArray ba;
    PortName=ReadSettings(COM,CONF_NAME,"PortName").toString();
    GunNum=ReadSettings(COM,CONF_NAME,"GunNum").toInt();
    if(PortName==NULL)
    {
        qDebug("=========PortName ERROR=========");
        return;
    }
    var=ReadSettings(COM,CONF_NAME,"BaudRate").toInt();
	switch (var) {
		case 9600:
			BaudRate=QSerialPort::Baud9600;
		break;
		case 19200:
			BaudRate=QSerialPort::Baud19200;
		break;
		case 38400:
			BaudRate=QSerialPort::Baud38400;
		break;
		case 57600:
			BaudRate=QSerialPort::Baud57600;
		break;
		case 115200:
			BaudRate=QSerialPort::Baud115200;
		break;
		default:
			BaudRate=QSerialPort::UnknownBaud;
		break;
	}

    var=ReadSettings(COM,CONF_NAME,"DataBits").toInt();
	switch (var) {
		case 5:
			DataBits=QSerialPort::Data5;
		break;
		case 6:
			DataBits=QSerialPort::Data6;
		break;
		case 7:
			DataBits=QSerialPort::Data7;
		break;
		case 8:
			DataBits=QSerialPort::Data8;
		break;
		default:
			DataBits=QSerialPort::UnknownDataBits;
		break;
	}
    ba=ReadSettings(COM,CONF_NAME,"Parity").toString().toLatin1();
	switch (ba.at(0)) {
		case 'n':
		case 'N':
			Parity=QSerialPort::NoParity;
		break;
		case 'o':
		case 'O':
			Parity=QSerialPort::OddParity;
		break;
		case 'e':
		case 'E':
			Parity=QSerialPort::EvenParity;
		break;
		default:
			Parity=QSerialPort::UnknownParity;
		break;
	}

    ba=ReadSettings(COM,CONF_NAME,"FlowControl").toString().toLatin1();
	switch (ba.at(0)) {
		case 'n':
		case 'N':
			FlowControl=QSerialPort::NoFlowControl;
		break;
		case 's':
		case 'S':
			FlowControl=QSerialPort::SoftwareControl;
		break;
		case 'h':
		case 'H':
			FlowControl=QSerialPort::HardwareControl;
		break;
		default:
			FlowControl=QSerialPort::UnknownFlowControl;
		break;
	}
}

