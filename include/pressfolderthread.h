#ifndef PRESSFOLDERTHREAD
#define PRESSFOLDERTHREAD
#include    "QThread"
class PressFolderThread : public QThread
{
    Q_OBJECT
   protected:
     void run();
};
#endif // PRESSFOLDERTHREAD

