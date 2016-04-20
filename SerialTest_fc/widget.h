#ifndef WIDGET_H
#define WIDGET_H
#include "serial_data_thread.h"
#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QDateTime>
#include <QScrollBar>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>
#include <QColor>
#include <QReadWriteLock>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    //------------------------------data(send/receive)---------------------------------------
    QByteArray commond_send;
    QByteArray commond_receive;
    //-------------------------------------serialport--------------------------------------------
    QSerialPort serial;
    bool  serialdetection_bool ;// = true;
    bool  seriaport_open ;
    bool  seriaport_time_add;   //= true
    int     bytes_num;

    //----------------------------------------------------------------------------------------------
    explicit Widget(QWidget *parent = 0);
    ~Widget();
signals:
    void   send_data(QString  read_time,QByteArray read_data);
private slots:
    void receive_serial_data();

    //void handle_data(QByteArray handle_ok_serialdata,QString  time,bool  true_false);

    void  handle_data(QString  read_time,QByteArray read_data);

    void on_serialdetection_button_clicked();

    void on_openserial_button_clicked();

    void on_closeserial_button_clicked();

    void on_tabWidget_currentChanged(int index);

    void on_ID01send_pushButton_clicked();

    void on_ID02send_pushButton_clicked();

    void on_ID04send_pushButton_clicked();

    void on_ID05send_pushButton_clicked();

    void on_ID06send_pushButton_clicked();

    void on_ID07send_pushButton_clicked();

    void on_ID08send_pushButton_clicked();

    void on_ID09send_pushButton_clicked();

    void on_ID0Bsend_pushButton_clicked();

    void on_ID0Csend_pushButton_clicked();

    void on_ID0Dsend_pushButton_clicked();

    void on_ID0Esend_pushButton_clicked();

private:
    Ui::Widget *ui;
   // Serialdata  serial_data;
    QReadWriteLock   lock;
   // serialdatathread data_thread;



public:
    void init_commond_send_data();
    void init_commond_receive_data();

    void  init_commond01_data();
    void  init_commond02_data();
    void  init_commond04_data();
    void  init_commond05_data();
    void  init_commond06_data();
    void  init_commond07_data();
    void  init_commond08_data();
    void  init_commond09_data();
    void  init_commond0A_data();
    void  init_commond0B_data();
    void  init_commond0C_data();
    void  init_commond0D_data();
    void  init_commond0E_data();

    //------------------------------------current command  lineEdit set---------------------------------------------
    void set_ineEdit(QByteArray &commond);
    //------------------------------------receive command  textEdit  set---------------------------------------------
    void set_receive_textEdit(QByteArray &commond);
    //------------------------------------receive command  textEdit  set---------------------------------------------
    void  send_commond();
protected:

};

#endif // WIDGET_H
