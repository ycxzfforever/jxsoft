#ifndef SERIAL
#define SERIAL
#define CONF_NAME   "conf.ini"
#include <QtSerialPort/QtSerialPort>
class SerialPort : public QObject
{
		Q_OBJECT

	public:
        SerialPort(QString ComNum);
		~SerialPort();
        QSerialPort         *serialPort;
        QString				PortName;//串口名
        qint16              GunNum;  //串口对应的枪号
		qint64				sendsize;        
	private slots:
        void SendData(QByteArray data);
		void RecvData();
		void handleBytesWritten(qint64 bytes);
        void handleError(QSerialPort::SerialPortError error);

	private:
		QByteArray  readDataBuf;
		QTimer      m_timer;
		QSerialPort::BaudRate BaudRate;
		QSerialPort::DataBits DataBits;
		QSerialPort::Parity Parity;				//N=无校验，O=奇校验，E=偶校验
		QSerialPort::StopBits StopBits;
		QSerialPort::FlowControl FlowControl;	//N=无流控，S=软件流控，H=硬件流控
        void ReadComConf(QString COM);

	signals:
        void ComRecvHandle(QByteArray);
        void datecheck(QByteArray data);
};

#endif // SERIAL

