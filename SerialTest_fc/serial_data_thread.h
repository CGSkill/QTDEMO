#ifndef SERIAL_DATA_THREAD
#define SERIAL_DATA_THREAD
#include <QString>
#include <QThread>
#include  <QQueue>
#include <QByteArray>
#include <QReadWriteLock>
#include <QDebug>

typedef  struct  serialdata {
    QString   time ;
    QByteArray  data;
}Serialdata;

static QQueue<serialdata> serialdata_queue;

class serialdatathread : public  QThread
{
    Q_OBJECT
public:
    serialdatathread(){
        stopped  =  false;
        judge =   true;
    }
    ~serialdatathread(){}
     void   stop();
     void  run();

private:
     QByteArray   handle_ok_serialdata;
     QReadWriteLock    lock;
     Serialdata    data_receive1;
     Serialdata    data_receive2;
     QString   time;
     volatile bool stopped;
     bool    judge;

  signals:
    void  send_data(QByteArray handle_ok_serialdata,QString  time,bool  true_false);
};






#endif // SERIAL_DATA_THREAD

