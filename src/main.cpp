#include"../include/kernel.h"

void creatdatabase()
{
    if(QFile::exists("jxsoft.db"))  return;//数据库已经存在

    QSqlDatabase database= QSqlDatabase::addDatabase("QSQLITE");
    database.setDatabaseName("jxsoft.db");
    if(database.open())
    {
        qDebug()<<"Database Opened Success";
        QSqlQuery sql_query;
        //创建流水交易记录表
        QString create_recordsql = "create table record ("
                "id         integer primary key autoincrement,"
                "recordid   int,"
                "gunnum     int,"
                "classnum   int,"
                "employeenum varchar,"
                "transtype  varchar,"
                "cardnum    varchar,"
                "cardtype   varchar,"
                "carnum     nvarchar,"
                "stationnum varchar,"
                "starttime  varchar,"
                "endtime    varchar,"
                "price  float,"
                "gas    float,"
                "money  float,"
                "realmoney  float,"
                "oldyue float,"
                "newyue float,"
                "totalrechargetimes int,"
                "totalrechargemoney float,"
                "cardtotaltimes int,"
                "cardtotalgas   float,"
                "cardtotalmoney float,"
                "guntotalgas    float,"
                "guntotalmoney  float,"
                "beforpressure  float,"
                "afterpressure  float,"
                "density        float,"
                "dangliang      float,"
                "devicetype     varchar,"
                "unit           varchar,"
                "stopreasion    char,"
                "greyid         int,"
                "classrecordid  int,"
                "startmode      varchar,"
                "startserialnum varchar,"
                "posnum     int)";
        sql_query.prepare(create_recordsql);
        if(!sql_query.exec())
        {
            qCritical()<<sql_query.lastError();
        }
        else
        {
            qDebug()<<"流水记录表创建成功";
        }

        //创建实时数据表
        sql_query.clear();
        QString create_realtimesql = "create table realtime ("
                "gunnum     int,"
                "devicetype varchar,"
                "status     varchar,"
                "price      float,"
                "gas        float,"
                "money      float,"
                "classnum   int,"
                "employeenum varchar,"
                "cardnum    varchar,"
                "carnum     nvarchar,"
                "pressure   float,"
                "inflow       float,"
                "intemperature float,"
                "inmass       float,"
                "indensity    float,"
                "ingain       float,"
                "outflow       float,"
                "outtemperature float,"
                "outmass       float,"
                "outdensity    float,"
                "outgain       float,"
                "alarm         int)";
        sql_query.prepare(create_realtimesql);
        if(!sql_query.exec())
        {
            qCritical()<<sql_query.lastError();
        }
        else
        {
            qDebug()<<"实时数据表创建成功";
        }

        QString insert_sql;
        for(int i=1;i<=10;i++)//实时数据表中先插入枪号，后期根据枪号更新实时数据
        {
            insert_sql=QString("insert into realtime (gunnum) values(%1)").arg(i);
            sql_query.clear();
            sql_query.prepare(insert_sql);
            if(!sql_query.exec())
            {
                qCritical()<<sql_query.lastError();
            }
        }
    }
    database.close();
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);       
    qInstallMessageHandler(MessageOutput);
    //QFile::remove("jxsoft.db");
    creatdatabase();

    QThread m1thread,m2thread;

//    Mainobj *mainobj1=new Mainobj("COM1");//COM1对应一个枪
//    mainobj1->moveToThread(&m1thread);
//    m1thread.start();

    Mainobj *mainobj2=new Mainobj("COM2");//COM2对应一个枪
    mainobj2->moveToThread(&m2thread);
    m2thread.start();

    return a.exec();
}
