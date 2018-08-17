#ifndef LOG
#define LOG
#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <iostream>
#include <QString>
#include <QtDebug>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <qdir.h>
#include <qmutex.h>
#include <QProcess>
#include <QDateTime>
#define		LOGFILE_SUFFIXNAME					"log"				//日志文件后缀名
#define		LOGFILE_MAXSIZE						1024*1024*10		//40M
#define		LOGFILE_PRESSMAXSIZE				1024*1024*10		//压缩后的日志文件最大10M
#define		LOGFILE_FOLDERNAME					"log"				//日志文件文件夹名称
#define		LOGFILE_FOLDERNAME_BACK 			"log_back"          //压缩日志文件时备份文件夹名称
#define		LOGFILE_PRESSFOLDERNAME             "log.tar.gz"

typedef union{
    unsigned char buf[4];
    unsigned long data;
}union32;
typedef union{
    unsigned char buf[2];
    unsigned short data;
}union16;
typedef union{
    unsigned char buf[4];
    float data;
}unionfloat;

void MessageOutput(QtMsgType type , const QMessageLogContext &, const QString &msg);
QStringList DirFileListXml(QString xmlName,QString Path);
void DeleteOldFile(QString NameSuffix,QString Path);
void formatString(QString &org, int n, const QChar &ch);
qint64 GetFileSize(QString FileName);
qint64 GetFolderSize(const QString &path);
void PressFolder();
QString ByteArrayToString(QByteArray ByteArray,QChar ch);
QString ByteArrayToString(QByteArray ByteArray, char type, char *);
QByteArray hexStringtoByteArray(QString hex);
char ConvertHexChar(char ch);
void StringToHex(QString str, QByteArray &senddata);
unsigned char DectoHex(unsigned char dec);
#endif // LOG


