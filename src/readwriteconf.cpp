#include "../include/kernel.h"
void WriteSettings(QString Group,QString Confname,QString key,QVariant value)
{
	QSettings settings(Confname, QSettings::IniFormat);
	settings.beginGroup(Group);
	settings.setValue(key, value);
	settings.endGroup();
}

QVariant ReadSettings(QString Group,QString Confname,QString key)
{
	QSettings settings(Confname, QSettings::IniFormat);
	QString str;
	str=Group+"/"+key;
	return settings.value(str);
}
