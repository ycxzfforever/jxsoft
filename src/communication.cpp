#include "../include/kernel.h"
globar globarvar;

Communication::Communication(QString COM)
{
    GunNum=ReadSettings(COM,CONF_NAME,"GunNum").toInt();
    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(sendrollcall()));
    timer->start(800);
}
Communication::~Communication()
{
    timer->stop();
}

//modbus crc校验
QByteArray Communication::CRC16Calculate(QByteArray buf)
{
    uint16_t IX, IY, CRC,Len;
    Len=buf.size();
    CRC = 0xFFFF;
    if(Len <= 0)    return 0;
    for(IX = 0; IX < Len; IX++)
    {
        CRC = CRC ^ (uint16_t)(buf[IX]);
        for(IY = 0; IY < 8; IY++)
        {
            if((CRC &1) != 0)
                CRC = (CRC >> 1) ^ 0xA001;
            else
                CRC = CRC >> 1;
        }
    }
    QByteArray r;
    r.append((unsigned char)(CRC>>8));
    r.append((unsigned char)(CRC>>0));
    return r;
}

void Communication::recvdatacheck(QByteArray data)
{
    QByteArray crc,str;

    if((data.at(0)!=0xBB)||(data.at(1)!=0xEE))//帧头
    {
        qDebug("Data error:%02X%02X",data.at(0),data.at(1));
    }
    else
    {
        uint16_t i;
        for(i=0;i<data.size()-6;i++)
        {
            str.append(data[i+2]);
        }
        crc=CRC16Calculate(str);
        if((crc.at(0)==data.at(data.size()-4))&&(crc.at(1)==data.at(data.size()-3)))//crc right
        {
            //jie xi
            dataanalysis(str);//数据正确时，解析数据
        }
        else
        {
            qDebug("==============CRC ERROR==============");
            qDebug("Recv crc:%02X%02X",data.at(data.size()-4),data.at(data.size()-3));
            qDebug("Calc crc:%02X%02X",crc.at(0),crc.at(1));
        }
    }
}

//51轮训
void Communication::sendrollcall()
{
    QByteArray str,tmp;
    QByteArray crc;

    str.append(GunNum);
    str.append(QByteArray::fromHex("00510000"));
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString current_date = current_date_time.toString("yyyyMMddhhmmss");
    //StringToHex(current_date,tmp);
    tmp=hexStringtoByteArray(current_date);
    str.append(tmp);
    str.append(QByteArray::fromHex(("00")));//black ver
    str.append(QByteArray::fromHex(("00")));//add black ver
    str.append(QByteArray::fromHex(("00")));//del black ver
    str.append(QByteArray::fromHex(("00")));//white ver
    str.append(QByteArray::fromHex(("00")));//price ver
    str.append(QByteArray::fromHex(("00")));//key ver
    str[4]=str.size()-5;//data length
    crc=CRC16Calculate(str);
    str.append(crc.at(0));
    str.append(crc.at(1));
    str.prepend(QByteArray::fromHex("AAFF"));//帧头
    str.append(QByteArray::fromHex("CCFF")); //帧尾
    emit send_S(str);
}


//加气机上传数据解析,不包含帧头帧尾和crc,消息体从第5个开始
void Communication::dataanalysis(QByteArray data)
{
    timer->stop();//停止点名
    switch (data.at(2))
    {
    case 0x51://加气机无数据下载或无记录上传时回复
        recvrealtime51H(data);
        break;
    case 0x52://加气机上传流水
        recvrecord52H(data);
        break;
    default:
        break;
    }    
    timer->start();//开始点名
}

void Communication::recvrealtime51H(QByteArray data)
{
    static QMutex  mutex;
    mutex.lock();
    //消息体从第5个开始
    unsigned int i=5,j=0,k=0;
    int tmp=0;
    QByteArray tmpba;
    QString tmpstr;
    union32 u32;
    union16 u16;
    unionfloat ufl;
    QSqlDatabase database;
    if(QSqlDatabase::contains("qt_sql_default_connection"))
      database = QSqlDatabase::database("qt_sql_default_connection");
    else
      database = QSqlDatabase::addDatabase("QSQLITE");

    database.setDatabaseName("jxsoft.db");

    if(database.open())
    {
        qDebug()<<"Record:Database Opened";
        QSqlQuery sql_query;
        sql_query.prepare("update realtime set devicetype=:devicetype,status=:status,"
                          "price=:price,gas=:gas,money=:money,classnum=:classnum,"
                          "employeenum=:employeenum,cardnum=:cardnum,carnum=:carnum,"
                          "pressure=:pressure,"
                          "inflow=:inflow,intemperature=:intemperature,inmass=:inmass,"
                          "indensity=:indensity,ingain=:ingain,outflow=:outflow,"
                          "outtemperature=:outtemperature,outmass=:outmass,"
                          "outdensity=:outdensity,outgain=:outgain,"
                          "alarm=:alarm where gunnum=:gunnum");

        sql_query.bindValue(":gunnum",GunNum);

        tmp=data.at(i++);
        if(tmp==0)
            sql_query.bindValue(":devicetype","LNG");
        else if(tmp==1)
            sql_query.bindValue(":devicetype","CNG");
        else if(tmp==2)
            sql_query.bindValue(":devicetype","加气柱");
        else if(tmp==3)
            sql_query.bindValue(":devicetype","卸气柱");

        tmp=data.at(i++);
        if(tmp==0xA0)
            sql_query.bindValue(":status","下班");
        else if(tmp==0xA1)
            sql_query.bindValue(":status","空闲");
        else if(tmp==0xA2)
            sql_query.bindValue(":status","小循环");
        else if(tmp==0xA3)
            sql_query.bindValue(":status","大循环");
        else if(tmp==0xA4)
            sql_query.bindValue(":status","加气中");
        else if(tmp==0xA5)
            sql_query.bindValue(":status","待扣费");
        else if(tmp==0xA6)
            sql_query.bindValue(":status","关闭");
        else if(tmp==0xA7)
            sql_query.bindValue(":status","对方加液");
        else if(tmp==0xF1)
            sql_query.bindValue(":status","设备故障");

        for(j=0;j<2;j++,i++)//单价
        {
            u16.buf[j]=data.at(i);
        }
        sql_query.bindValue(":price",u16.data/100.0);

        for(j=0;j<4;j++,i++)//气量
        {
            u32.buf[j]=data.at(i);
        }
        sql_query.bindValue(":gas",u32.data/100.0);

        for(j=0;j<4;j++,i++)//金额
        {
            u32.buf[j]=data.at(i);
        }
        sql_query.bindValue(":money",u32.data/100.0);

        sql_query.bindValue(":classnum", data.at(i++));//班号

        tmpba.resize(8);
        for(j=0;j<8;j++,i++)//员工号
        {
            tmpba[j]=data.at(i);
        }
        tmpstr=ByteArrayToString(tmpba,1,NULL);
        if(tmpstr!=NULL)
            sql_query.bindValue(":employeenum",tmpstr);
        else
            sql_query.bindValue(":employeenum","0");

        tmpba.resize(8);
        for(j=0;j<8;j++,i++)//用户卡号
        {
            tmpba[j]=data.at(i);
        }
        tmpstr=ByteArrayToString(tmpba,1,NULL);
        if(tmpstr!=NULL)
            sql_query.bindValue(":cardnum",tmpstr);
        else
            sql_query.bindValue(":cardnum","0");

        tmpba.resize(9);
        tmpstr.clear();
        for(j=0;j<9;j++,i++)//车牌号
        {
            if(data.at(i)!=0)   break;
            else k++;   //前面0去掉
        }
        if(j<9)
        {
            for(j=0;j<9-k;j++,i++)
            {
                tmpba[j]=data.at(i);
            }
            QTextCodec *codec = QTextCodec::codecForName("GB2312");
            tmpstr=codec->toUnicode(tmpba);
        }
        if(tmpstr!=NULL)
            sql_query.bindValue(":carnum",tmpstr);
        else
            sql_query.bindValue(":carnum","0");

        for(j=0;j<2;j++,i++)//压力
        {
            u16.buf[j]=data.at(i);
        }
        sql_query.bindValue(":pressure",u16.data/100.0);

        for(j=0;j<4;j++,i++)//进气流速
        {
            ufl.buf[j]=data.at(i);
        }
        sql_query.bindValue(":inflow",ufl.data);

        for(j=0;j<4;j++,i++)//进气温度
        {
            ufl.buf[j]=data.at(i);
        }
        sql_query.bindValue(":intemperature",ufl.data);

        for(j=0;j<4;j++,i++)//进气质量
        {
            ufl.buf[j]=data.at(i);
        }
        sql_query.bindValue(":inmass",ufl.data);

        for(j=0;j<4;j++,i++)//进气密度
        {
            ufl.buf[j]=data.at(i);
        }
        sql_query.bindValue(":indensity",ufl.data);

        for(j=0;j<4;j++,i++)//进气增益
        {
            ufl.buf[j]=data.at(i);
        }
        sql_query.bindValue(":ingain",ufl.data);

        for(j=0;j<4;j++,i++)//回气流速
        {
            ufl.buf[j]=data.at(i);
        }
        sql_query.bindValue(":outflow",ufl.data);

        for(j=0;j<4;j++,i++)//回气温度
        {
            ufl.buf[j]=data.at(i);
        }
        sql_query.bindValue(":outtemperature",ufl.data);

        for(j=0;j<4;j++,i++)//回气质量
        {
            ufl.buf[j]=data.at(i);
        }
        sql_query.bindValue(":outmass",ufl.data);

        for(j=0;j<4;j++,i++)//回气密度
        {
            ufl.buf[j]=data.at(i);
        }
        sql_query.bindValue(":outdensity",ufl.data);

        for(j=0;j<4;j++,i++)//回气增益
        {
            ufl.buf[j]=data.at(i);
        }
        sql_query.bindValue(":outgain",ufl.data);

        for(j=0;j<4;j++,i++)//系统报警
        {
            u32.buf[j]=data.at(i);
        }
        tmp=u32.data;
        sql_query.bindValue(":alarm",tmp);

        if(!sql_query.exec())
        {
            qCritical()<<sql_query.lastError();
        }
    }
    database.close();
    mutex.unlock();
}

//加气机上传流水
void Communication::recvrecord52H(QByteArray data)
{        
    static QMutex  mutex;
    mutex.lock();
    //消息体从第5个开始
    unsigned int i=5,j=0,k=0;
    int tmp=0;
    QByteArray tmpba;
    QString tmpstr;
    union32 u32;
    union16 u16;
    QSqlDatabase database;
    if(QSqlDatabase::contains("qt_sql_default_connection"))
      database = QSqlDatabase::database("qt_sql_default_connection");
    else
      database = QSqlDatabase::addDatabase("QSQLITE");

    database.setDatabaseName("jxsoft.db");

    if(database.open())
    {
        qDebug()<<"Record:Database Opened";
        QSqlQuery sql_query;

        /*获取数据库最大id号*/
//        sql_query.prepare("select max(id) from record");
//        if(!sql_query.exec())
//        {
//            qDebug()<<sql_query.lastError();
//        }
//        else
//        {
//            while(sql_query.next())
//            {
//                tmp = sql_query.value(0).toInt();
//                qDebug()<<QString("MAX_ID=%1").arg(tmp);
//            }
//        }
        /*获取数据库最大id号 end*/
//        tmp++;

        sql_query.clear();
        sql_query.prepare("INSERT INTO record ("
                          "id,recordid,gunnum,classnum, employeenum,transtype,cardnum,"
                          "cardtype,carnum,stationnum,starttime,endtime,"
                          "price,gas,money,realmoney,oldyue,"
                          "newyue,totalrechargetimes,totalrechargemoney,cardtotaltimes,cardtotalgas,"
                          "cardtotalmoney,guntotalgas,guntotalmoney,beforpressure,afterpressure,"
                          "density,dangliang,devicetype,unit,stopreasion,"
                          "greyid,classrecordid,startmode,startserialnum,posnum)"
                          "VALUES ("
                          ":id,:recordid,:gunnum ,:classnum, :employeenum,:transtype,:cardnum,"
                          ":cardtype,:carnum,:stationnum,:starttime,:endtime,"
                          ":price,:gas,:money,:realmoney,:oldyue,"
                          ":newyue,:totalrechargetimes,:totalrechargemoney,:cardtotaltimes,:cardtotalgas,"
                          ":cardtotalmoney,:guntotalgas,:guntotalmoney,:beforpressure,:afterpressure,"
                          ":density,:dangliang,:devicetype,:unit,:stopreasion,"
                          ":greyid,:classrecordid,:startmode,:startserialnum,:posnum)");

        //sql_query.bindValue(":id", NULL);//id记录号自增

        for(j=0;j<4;j++,i++)
        {
            u32.buf[j]=data.at(i);//流水号
        }
        tmp=u32.data;

        /*获取是否已经存在此条流水*/
        QSqlQuery tmpquary;
        QString str = QString("select count(*) from record where recordid=%1").arg(tmp);
        if(tmpquary.exec(str))
        {
            int t;
            while(tmpquary.next())
            {
                t = tmpquary.value(0).toInt();
            }
            //qDebug()<<QString("流水记录数=%1").arg(t);
            if(t==0)
            {
                sql_query.bindValue(":recordid", tmp);
            }
            else
            {
                qDebug("%d流水记录已存在",tmp);
                sendrecord52H(data,true);
                mutex.unlock();
                return;
            }
        }
        else
        {
            qDebug()<<sql_query.lastError();
        }
        /*获取是否已经存在此条流水 end*/

        sql_query.bindValue(":gunnum",GunNum);
        sql_query.bindValue(":classnum", data.at(i++));
        tmpba.resize(8);
        for(j=0;j<8;j++,i++)//员工号
        {
            tmpba[j]=data.at(i);
        }
        tmpstr=ByteArrayToString(tmpba,1,NULL);
        if(tmpstr!=NULL)
            sql_query.bindValue(":employeenum",tmpstr);
        else
            sql_query.bindValue(":employeenum","0");

        tmp=data.at(i++);
        if(tmp==0)
            sql_query.bindValue(":transtype","正常交易");
        else if((tmp==1)||(tmp==0xFF))
            sql_query.bindValue(":transtype","逃卡交易");
        else if(tmp==2)
            sql_query.bindValue(":transtype","补扣交易");
        else if(tmp==3)
            sql_query.bindValue(":transtype","补充交易");

        tmpba.resize(8);
        for(j=0;j<8;j++,i++)//用户卡号
        {
            tmpba[j]=data.at(i);
        }
        tmpstr=ByteArrayToString(tmpba,1,NULL);
        if(tmpstr!=NULL)
            sql_query.bindValue(":cardnum",tmpstr);
        else
            sql_query.bindValue(":cardnum","0");

        tmp=data.at(i++);
        if(tmp==1)
            sql_query.bindValue(":cardtype","储蓄卡");
        else if(tmp==2)
            sql_query.bindValue(":cardtype","记账卡");
        else
            sql_query.bindValue(":cardtype","无卡");

        tmpba.resize(9);
        tmpstr.clear();
        for(j=0;j<9;j++,i++)//车牌号
        {
            if(data.at(i)!=0)   break;
            else k++;   //前面0去掉
        }
        if(j<9)
        {
            for(j=0;j<9-k;j++,i++)
            {
                tmpba[j]=data.at(i);
            }
            QTextCodec *codec = QTextCodec::codecForName("GB2312");
            tmpstr=codec->toUnicode(tmpba);
        }
        if(tmpstr!=NULL)
            sql_query.bindValue(":carnum",tmpstr);
        else
            sql_query.bindValue(":carnum","0");

        tmpba.resize(4);
        for(j=0;j<4;j++,i++)//站编号
        {
            tmpba[j]=data.at(i);
        }
        tmpstr=ByteArrayToString(tmpba,1,NULL);
        if(tmpstr!=NULL)
            sql_query.bindValue(":stationnum",tmpstr);
        else
            sql_query.bindValue(":stationnum","0");

        tmpba.resize(7);
        for(j=0;j<7;j++,i++)//开始时间
        {
            tmpba[j]=data.at(i);
        }
        tmpstr=ByteArrayToString(tmpba,0,NULL);
        //20161227142418
        tmpstr.insert(4,'-');
        tmpstr.insert(7,'-');
        tmpstr.insert(10,' ');
        tmpstr.insert(13,':');
        tmpstr.insert(16,':');
        sql_query.bindValue(":starttime",tmpstr);

        tmpba.resize(7);
        for(j=0;j<7;j++,i++)//结束时间
        {
            tmpba[j]=data.at(i);
        }
        tmpstr=ByteArrayToString(tmpba,0,NULL);
        tmpstr.insert(4,'-');
        tmpstr.insert(7,'-');
        tmpstr.insert(10,' ');
        tmpstr.insert(13,':');
        tmpstr.insert(16,':');
        sql_query.bindValue(":endtime",tmpstr);

        for(j=0;j<2;j++,i++)//单价
        {
            u16.buf[j]=data.at(i);
        }        
        sql_query.bindValue(":price",u16.data/100.0);

        for(j=0;j<4;j++,i++)//气量
        {
            u32.buf[j]=data.at(i);
        }        
        sql_query.bindValue(":gas",u32.data/100.0);

        for(j=0;j<4;j++,i++)//金额
        {
            u32.buf[j]=data.at(i);
        }        
        sql_query.bindValue(":money",u32.data/100.0);

        for(j=0;j<4;j++,i++)//实际扣款金额
        {
            u32.buf[j]=data.at(i);
        }        
        sql_query.bindValue(":realmoney",u32.data/100.0);

        for(j=0;j<4;j++,i++)//加气前余额
        {
            u32.buf[j]=data.at(i);
        }       
        sql_query.bindValue(":oldyue",u32.data/100.0);

        for(j=0;j<4;j++,i++)//加气后余额
        {
            u32.buf[j]=data.at(i);
        }    
        sql_query.bindValue(":newyue",u32.data/100.0);

        for(j=0;j<4;j++,i++)//累计充值次数
        {
            u32.buf[j]=data.at(i);
        }
        tmp=u32.data;
        sql_query.bindValue(":totalrechargetimes",tmp);

        for(j=0;j<4;j++,i++)//累计充值金额
        {
            u32.buf[j]=data.at(i);
        }        
        sql_query.bindValue(":totalrechargemoney",u32.data/100.0);

        for(j=0;j<4;j++,i++)//累计消费值次数
        {
            u32.buf[j]=data.at(i);
        }        
        tmp=u32.data;
        sql_query.bindValue(":cardtotaltimes",tmp);

        for(j=0;j<4;j++,i++)//累计消费气量
        {
            u32.buf[j]=data.at(i);
        }        
        sql_query.bindValue(":cardtotalgas",u32.data/100.0);

        for(j=0;j<4;j++,i++)//累计消费金额
        {
            u32.buf[j]=data.at(i);
        }        
        sql_query.bindValue(":cardtotalmoney",u32.data/100.0);

        for(j=0;j<4;j++,i++)//枪气量累计
        {
            u32.buf[j]=data.at(i);
        }        
        sql_query.bindValue(":guntotalgas",u32.data/100.0);

        for(j=0;j<4;j++,i++)//枪金额累计
        {
            u32.buf[j]=data.at(i);
        }        
        sql_query.bindValue(":guntotalmoney",u32.data/100.0);

        for(j=0;j<2;j++,i++)//加气前压力
        {
            u16.buf[j]=data.at(i);
        }        
        sql_query.bindValue(":beforpressure",u16.data/100.0);

        for(j=0;j<2;j++,i++)//加气后压力
        {
            u16.buf[j]=data.at(i);
        }        
        sql_query.bindValue(":afterpressure",u16.data/100.0);

        for(j=0;j<4;j++,i++)//密度
        {
            u32.buf[j]=data.at(i);
        }        
        sql_query.bindValue(":density",u32.data/10000.0);

        for(j=0;j<4;j++,i++)//当量
        {
            u32.buf[j]=data.at(i);
        }        
        sql_query.bindValue(":dangliang",u32.data/10000.0);

        tmp=data.at(i++);
        if(tmp==0)
            sql_query.bindValue(":devicetype","LNG");
        else if(tmp==1)
            sql_query.bindValue(":devicetype","CNG");
        else if(tmp==2)
            sql_query.bindValue(":devicetype","加气柱");
        else if(tmp==3)
            sql_query.bindValue(":devicetype","卸气柱");

        tmp=data.at(i++);//计量单位
        if(tmp==0)
            sql_query.bindValue(":unit","千克");
        else if(tmp==1)
            sql_query.bindValue(":unit","标方");

        sql_query.bindValue(":stopreasion",data.at(i++));//停机原因

        for(j=0;j<4;j++,i++)//解灰相关流水号
        {
            u32.buf[j]=data.at(i);
        }
        tmp=u32.data;
        sql_query.bindValue(":greyid",tmp);

        for(j=0;j<4;j++,i++)//班流水号
        {
            u32.buf[j]=data.at(i);
        }
        tmp=u32.data;
        sql_query.bindValue(":classrecordid",tmp);

        tmp=data.at(i++);//启动方式
        if(tmp==0)
            sql_query.bindValue(":startmode","手动启动");
        else if(tmp==1)
            sql_query.bindValue(":startmode","后台启动");
        else if(tmp==2)
            sql_query.bindValue(":startmode","标签启动");

        tmpba.resize(8);
        for(j=0;j<8;j++,i++)//启动序列号
        {
            tmpba[j]=data.at(i);
        }
        tmpstr=ByteArrayToString(tmpba,0,NULL);
        sql_query.bindValue(":startserialnum",tmpstr);

        sql_query.bindValue(":posnum",data.at(i++));//手持机编号

        if(!sql_query.exec())
        {
            qCritical()<<sql_query.lastError();
            sendrecord52H(data,false);
        }
        else
        {
            qDebug()<<"插入记录成功";
            sendrecord52H(data,true);
        }

    }
    database.close();
    mutex.unlock();
}

void Communication::sendrecord52H(QByteArray data,bool type)
{
    uchar i;
    QByteArray str,tmp;
    QByteArray crc;

    str.append(GunNum);
    str.append(QByteArray::fromHex("00520000"));
    if(type==true)//保存是否成功
        str.append(QByteArray::fromHex(("01")));
    else
        str.append(QByteArray::fromHex(("00")));
    str.append(data[18]);//交易类型
    tmp.resize(4);
    for(i=0;i<4;i++)//流水号
    {
        tmp[i]=data[8-i];
    }
    str.append(tmp);
    str[4]=str.size()-5;//data length
    crc=CRC16Calculate(str);
    str.append(crc.at(0));
    str.append(crc.at(1));
    str.prepend(QByteArray::fromHex("AAFF"));//帧头
    str.append(QByteArray::fromHex("CCFF")); //帧尾
    emit send_S(str);
}

