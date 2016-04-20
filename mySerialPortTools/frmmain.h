#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QWidget>
#include <QTimer>
#include <QKeyEvent>
#include <QColor>
#include <QWidget>

#include <QMessageBox>
#include <QDateTime>
#include <QScrollBar>
#include <QThread>
#include <QDebug>
#include <QColor>
#include <QReadWriteLock>
#include <QTime>
#include <QCoreApplication>
#include <QLineEdit>
#include <QTableWidget>
#include  <QVector>
#include <QMutex>
#include "qextserialport.h"

namespace Ui {
class frmMain;
}

class frmMain : public QWidget
{
    Q_OBJECT

public:
    explicit frmMain(QWidget *parent = 0);
    ~frmMain();

protected:    
    bool eventFilter(QObject *obj, QEvent *event);//监听事件

private:
    Ui::frmMain *ui;

    QextSerialPort *myCom;
    QTimer *myTimer;

    QTimer *myReadTimer;//定时读取串口数据
    QTimer *mySendTimer;//定时发送串口数据
    QTimer *mySaveTimer;//定时保存串口数据

    int SendCount;//发送数据计数
    int ReceiveCount;//接收数据计数
    bool IsShow;//是否显示数据
    bool IsDebug;//是否启用调试,接收到数据后模拟发送数据    
    bool IsAutoClear;//是否自动清空
    bool IsHexSend;//是否16进制数据发送
    bool IsHexReceive;//是否16进制数据接收    

    QStringList SendDataList;//转发数据链表

    QByteArray   check_num;
    int   num_3to2;
    QVector<QByteArray> data_3to2;

public:
    //------------------------------data(send/receive)---------------------------------------
    QByteArray commond_send;
    QByteArray commond_receive;
    //-------------------------------------serialport--------------------------------------------
    //QSerialPort serial;
    bool  serialdetection_bool ;// = true;
    bool  seriaport_open ;
    bool  seriaport_time_add;   //= true
    int     bytes_num;

    //----------------------------------------------------------------------------------------------
    int     judge_num;
    QMutex   mutex;



private:
    void InitForm();//初始化界面以及其他处理
    void ChangeEnable(bool b);//改变状态

private slots:
    void on_cboxSave_currentIndexChanged(int index);
    void on_cboxSend_currentIndexChanged(int index);
    void on_btnData_clicked();
    void on_btnSend_clicked();
    void on_btnSave_clicked();
    void on_btnClearAll_clicked();
    void on_btnStopShow_clicked();
    void on_btnClearReceive_clicked();
    void on_btnClearSend_clicked();
    void on_ckHexSend_stateChanged(int arg1);
    void on_ckHexReceive_stateChanged(int arg1);
    void on_ckIsAutoSave_stateChanged(int arg1);
    void on_ckIsAutoSend_stateChanged(int arg1);
    void on_ckIsAutoClear_stateChanged(int arg1);
    void on_ckIsDebug_stateChanged(int arg1);
    void on_btnOpen_clicked();

    void SetTime();//动态显示时间
    void ReadMyCom();//读取串口数据
    void WriteMyCom();//写串口数据
    void WriteMyCom(QByteArray &commond_send);//
    void WriteMyCom_custom();//get  lineEdit' s   txt  with  handled  waitefor  send
    void SaveMyCom();//保存串口数据    

    void ReadConfigData();//读取配置文件数据
    void ReadSendData();//读取转发文件数据
     void  handle_data(QString  read_time,QByteArray read_data);
     void on_custom_pushButton_clicked(); //自定义命令发送

signals:
    void   send_cf_signal_04(bool A5_B6);
    void   send_cf_signal_05(bool A5_B6,QByteArray channel_num);

public slots:

    void  send_cf_slots04(bool  A5_B6);

    void  send_cf_slots05(bool  A5_B6,QByteArray channel_num);

    void on_tabWidget_currentChanged(int index);

    void on_ID02send_pushButton_4_clicked();

    void on_ID04send_pushButton_4_clicked();

    void on_ID05send_pushButton_4_clicked();

    void on_ID06send_pushButton_4_clicked();

    void on_ID07send_pushButton_4_clicked();

    void on_ID08send_pushButton_4_clicked();

    void on_ID09send_pushButton_4_clicked();

    void on_ID0Bsend_pushButton_4_clicked();

    void on_ID0Csend_pushButton_4_clicked();

    void on_ID0Dsend_pushButton_4_clicked();

    void on_ID0Esend_pushButton_4_clicked();

public:
    void init_commond_send_data();
    void init_commond_receive_data();

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
    void set_txtDataHex(QByteArray &commond);
};

#endif // FRMMAIN_H
