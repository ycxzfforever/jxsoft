#include "../include/kernel.h"

/********************************************************************
* 函数名:DirFileListXml
* 说明:
* 功能:对目录下的文件排序
* 输入:	xmlName：文件类型（后缀名）
*			Path：路径
* 返回值:QStringList
* 创建人:Yang Chao Xu
* 创建时间:2015-10-15
*********************************************************************/
QStringList DirFileListXml(QString xmlName,QString Path)
{
    QStringList fileList;
    QDir dir(Path);
    if (!dir.exists()) return fileList;
    dir.setFilter(QDir::Dirs|QDir::Files);
    dir.setSorting(QDir::DirsFirst|QDir::Time |QDir::Reversed);//排序方式 修改时间从小到大,文件夹在前
    QFileInfoList list = dir.entryInfoList();
    int i=0,filecont=0;
    do{
        QFileInfo fileInfo = list.at(i);
        if(fileInfo.fileName() == "." || fileInfo.fileName()== "..")
        {
            qDebug()<<"filedir="<<fileInfo.fileName();
            i++;
            continue;
        }
        bool bisDir=fileInfo.isDir();
        if(!bisDir)
        {
            QString currentFileName=fileInfo.fileName();
            bool Reght=currentFileName.endsWith(xmlName, Qt::CaseInsensitive);
            if(Reght)
            {
                fileList<<currentFileName;
                qDebug()<<"filelist sort="<<currentFileName;
                filecont++;
            }
        }
        i++;
    }while(i<list.size());
    return fileList;
}

/********************************************************************
* 函数名:DirFileListXml
* 说明:
* 功能:删除目录下面最旧的文件
* 输入:	NameSuffix：文件类型（后缀名）
*			Path：路径
* 返回值:NULL
* 创建人:Yang Chao Xu
* 创建时间:2015-10-15
*********************************************************************/
void DeleteOldFile(QString NameSuffix,QString Path)
{
    QDir dir(Path);
    if (!dir.exists()) 	return;
    dir.setFilter(QDir::Dirs|QDir::Files);
    dir.setSorting(QDir::DirsFirst|QDir::Time |QDir::Reversed);//排序方式 修改时间从小到大,文件夹在前
    QFileInfoList list = dir.entryInfoList();
    int i=0;
    do{
        QFileInfo fileInfo = list.at(i);
        bool bisDir=fileInfo.isDir();
        if((fileInfo.fileName() != ".") &&(fileInfo.fileName()!= "..") &&(!bisDir))
        {
            QString currentFileName=fileInfo.fileName();
            bool Reght=currentFileName.endsWith(NameSuffix, Qt::CaseInsensitive);
            if(Reght)
            {
                dir.remove(currentFileName);
                break;
            }
        }
        i++;
    }while(i<list.size());
}

/********************************************************************
* 函数名:MessageOutput
* 说明:
* 功能:重定向打印信息到日志文件中
* 输入:
* 返回值:NULL
* 创建人:Yang Chao Xu
* 创建时间:2015-10-14
*********************************************************************/
void MessageOutput(QtMsgType type , const QMessageLogContext &, const QString &msg)
{
    static QMutex mutex;
    mutex.lock();
    QString log,logname;
    switch(type)
    {
    case QtDebugMsg:
        log = QString("Debug: ");
        logname="DebugLog";
        break;
    case QtWarningMsg:
        log = QString("Warning: ");
        logname="WarningLog";
        break;
    case QtCriticalMsg:
        log = QString("Critical Error: ");
        logname="CErrorLog";
        break;
    case QtFatalMsg:
        log = QString("Fatal Error: ");
        logname="FErrorLog";
        abort();
        break;
    default:
        log = QString("Unknow Msg Type : ");
        logname="UnknowLog";
        break;
    };

    //文件夹不存在则创建//
    QDir *tmp= new QDir;
    if(!(tmp->exists(LOGFILE_FOLDERNAME)))
        tmp->mkdir(LOGFILE_FOLDERNAME);
    delete(tmp);
    //判断log文件夹的大小是否大于最大值
    if(GetFolderSize(LOGFILE_FOLDERNAME)>LOGFILE_MAXSIZE)
    {
        if(GetFileSize(LOGFILE_PRESSFOLDERNAME)>LOGFILE_PRESSMAXSIZE)
        {
            QProcess *pro=new QProcess;
            QString t1="rm";
            QStringList t2;
            t2.append("-f");
            t2.append(LOGFILE_PRESSFOLDERNAME);
            pro->start(t1,t2);
        }

        PressFolder();
    }
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyy-MM-dd hh:mm:ss.zzz ddd ");
    log +=current_date ;
#ifdef	Debug
    log +=QString("File:")+ QString(context.file) ;
    log += QString(" Line:") + QString::number(context.line) ;
#endif
    log +=QString(" %1\n").arg(msg);
    logname+=current_date_time.toString("yyyyMMdd")+"."+LOGFILE_SUFFIXNAME;
    logname.prepend("/");
    logname.prepend(LOGFILE_FOLDERNAME);
    QFile logFile(logname);
    logFile.open(QIODevice::WriteOnly | QIODevice::Append);
    logFile.write(log.toStdString().c_str());
    logFile.flush();
    mutex.unlock();
}

/********************************************************************
* 函数名:GetFileSize
* 说明:
* 功能:
* 输入:FileName:文件名
* 返回值:文件大小
* 创建人:Yang Chao Xu
* 创建时间:2015-10-15
*********************************************************************/
qint64 GetFileSize(QString FileName)
{
    QFile file(FileName);
    return file.size();
}

/********************************************************************
* 函数名:GetFolderSize
* 说明:
* 功能:类似于linux下面du命令，查看文件夹大小
* 输入:path：文件夹路径
* 返回值:文件夹大小
* 创建人:Yang Chao Xu
* 创建时间:2015-10-14
*********************************************************************/
qint64 GetFolderSize(const QString &path)
{
    QDir dir(path);//QDir类具有存取目录结构和内容的能力
    qint64 size = 0;

    //entryInfoList(Qdir::Files)函数返回文件信息，根据这些信息计算文件的大小
    foreach (QFileInfo fileInfo, dir.entryInfoList(QDir::Files))
        size += fileInfo.size();
    // 判断是否有子目录,如果有，则递归计算
    //entryList(QDir::Dirs | QDir::NoDotAndDotDot 返回所有子目录并过滤掉
    // '.' '..'目录
    //使用QDir::separator() 函数来返回特定平台的目录分割符
    foreach (QString subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        size += GetFolderSize(path + QDir::separator() + subDir);

    //	char unit = 'B';
    //	quint64 curSize = size;
    //	if(curSize > 1024) {
    //			curSize /= 1024;
    //			unit = 'K';
    //			if(curSize > 1024) {
    //					curSize /= 1024;
    //					unit = 'M';
    //					if(curSize > 1024) {
    //							curSize /= 1024;
    //							unit = 'G';
    //					}
    //			}
    //	}
    //	qDebug() << curSize << unit << "\t" << qPrintable(path) << endl;
    return size;
}

/********************************************************************
* 函数名:PressFolder
* 说明:
* 功能:调用windows下的winrar来压缩文件夹
* 输入:FolderName:文件夹名字
* 返回值:NULL
* 创建人:Yang Chao Xu
* 创建时间:2015-10-15
*********************************************************************/
void PressFolder()
{
    PressFolderThread *pft=new PressFolderThread;
    pft->start();
}

//格式化字符串数组，字节之间加相应字符隔开
QString ByteArrayToString(QByteArray ByteArray,QChar ch)
{
    QString str,t;
    char tmp[4];
    for(int i=0;i<ByteArray.size();i++){
        sprintf(tmp,"%02X",(uchar)ByteArray.at(i));
        str.append(tmp);
        if(i!=ByteArray.size()-1)
            str.append(ch);
    }
    t=QObject::tr(" Len=%1").arg(ByteArray.size());
    str.append(t);
    return str;
}

//格式化字符串数组，字节之间不加相应字符隔开，而且去掉前面的0，重载函数
QString ByteArrayToString(QByteArray ByteArray,char type,char *)
{
    QString str;
    char tmp[4];
    int i=0;
    if(type==1)
    {
        for(i=0;i<ByteArray.size();i++){
            if(ByteArray.at(i)!=(0x00))
                break;
        }
    }
    for(;i<ByteArray.size();i++){
        sprintf(tmp,"%02X",(uchar)ByteArray.at(i));
        str.append(tmp);
    }
    return str;
}

/*************************************************
Function: formatString()
Description: 将十六进制字串每字节中间加空格分隔
Calls:
Called By:hexStringtoByteArray()
Input: org->待处理的字串
       n->间隔数默认为2
       ch->分隔标志，在此取空格
Output: NULL
Return: void
Others: NULL
*************************************************/
void formatString(QString &org, int n=2, const QChar &ch=QChar(' '))
{
    int size= org.size();
    int space= qRound(size*1.0/n+0.5)-1;
    if(space<=0)
        return;
    for(int i=0,pos=n;i<space;++i,pos+=(n+1))
    {
        org.insert(pos,ch);
    }
}

//将一个字符串数组转换成BCD字符串,与下面函数StringToHex()一样的功能
QByteArray hexStringtoByteArray(QString hex)
{
    QByteArray ret;
    hex=hex.trimmed();
    formatString(hex,2,' ');
    QStringList sl=hex.split(" ");
    foreach(QString s,sl)
    {
        if(!s.isEmpty())
            ret.append((char)s.toInt(0,16)&0xFF);
    }
    return ret;
}

//将十六进制的字符串转换为对应的字符
char ConvertHexChar(char ch)
{
    if((ch >= '0') && (ch <= '9'))
        return ch-0x30;
    else if((ch >= 'A') && (ch <= 'F'))
        return ch-'A'+10;
    else if((ch >= 'a') && (ch <= 'f'))
        return ch-'a'+10;
    else return (-1);
}

//将一个字符串数组转换成BCD字符串
void StringToHex(QString str, QByteArray &senddata)
{
    int hexdata,lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    senddata.resize(len/2);
    char lstr,hstr;
    for(int i=0; i<len; )
    {
        //char lstr,
        hstr=str[i].toLatin1();
        if(hstr == ' ')
        {
            i++;
            continue;
        }
        i++;
        if(i >= len)
            break;
        lstr = str[i].toLatin1();
        hexdata = ConvertHexChar(hstr);
        lowhexdata = ConvertHexChar(lstr);
        if((hexdata == 16) || (lowhexdata == 16))
            break;
        else
            hexdata = hexdata*16+lowhexdata;
        i++;
        senddata[hexdatalen] = (char)hexdata;
        hexdatalen++;
    }
    senddata.resize(hexdatalen);
}


