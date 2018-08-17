#include "../include/kernel.h"


void PressFolderThread::run()
{
    QProcess *pro = new QProcess;
    //先更改文件夹名字
    QString t1="mv";
    QStringList t2;
    t2.append(LOGFILE_FOLDERNAME);
    t2.append(LOGFILE_FOLDERNAME_BACK);
    pro->execute(t1,t2);

    //再创建log文件夹
    t1.clear();
    t2.clear();
    t1="mkdir";
    t2.append(LOGFILE_FOLDERNAME);
    pro->execute(t1,t2);

    //再压缩文件夹
    t1.clear();
    t2.clear();
    t1="tar";
    t2.append("-czf");
    t2.append(LOGFILE_PRESSFOLDERNAME);
    t2.append(LOGFILE_FOLDERNAME_BACK);
    pro->execute(t1,t2);

    //再删除文件夹下的文件
    t1.clear();
    t2.clear();
    t1="rm";
    t2.append("-rf");
    t2.append(LOGFILE_FOLDERNAME_BACK);
    pro->execute(t1,t2);

    quit();
}
