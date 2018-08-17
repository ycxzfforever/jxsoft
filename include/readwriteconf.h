#ifndef READWRITECONF
#define READWRITECONF
#define		CONF_NAME		"conf.ini"
void WriteSettings(QString Group,QString Confname,QString key,QVariant value);
QVariant ReadSettings(QString Group,QString Confname,QString key);
#endif // READWRITECONF

