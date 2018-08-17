#ifndef MAINOBJ_H
#define MAINOBJ_H
#include "communication.h"
#include"serial.h"
#include <QObject>

class Mainobj:public QObject
{
    Q_OBJECT
public:
    Communication *comm;
    SerialPort *serial;
    Mainobj(QString COM);
    ~Mainobj();
};

#endif // MAINOBJ_H
