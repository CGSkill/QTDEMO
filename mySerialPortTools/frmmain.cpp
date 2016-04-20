#include "frmmain.h"
#include "ui_frmmain.h"
#include "myhelper.h"
#include <QThread>

frmMain::frmMain(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmMain)
{
    ui->setupUi(this);
    myHelper::FormInCenter(this);
    this->InitForm();

    commond_send.resize(32);
    commond_receive.resize(32);
    //--------------------------------init   tap1------------------------
    init_commond_send_data();
    init_commond02_data();
    QByteArray custom_data;
    custom_data = commond_send;
    custom_data[9] = 0x01;
    custom_data[10] = 0x01;

    custom_data[11] = 0xA5;
    custom_data[12] = 0xA5;

    ui->lineEdit->clear();
    ui->lineEdit->insert(myHelper::ByteArrayToHexStr(custom_data));
    judge_num  =   0;
    //---------------------------------send    CF-------------------------
    connect(this,SIGNAL(send_cf_signal_04(bool)),this,SLOT(send_cf_slots04(bool)));
    connect(this,SIGNAL(send_cf_signal_05(bool,QByteArray)),this,SLOT(send_cf_slots05(bool,QByteArray)));
    //--------------------------------ui setEnable-----------------------
   // this->ui->ID01send_pushButton_4->setEnabled(false);
    this->ui->ID0Asend_pushButton_4->setEnabled(false);
}

frmMain::~frmMain()
{    
    delete ui;
}

//监听回车键
bool frmMain::eventFilter(QObject *obj, QEvent *event)
{
    if (obj==ui->txtSend)
    {
        if (event->type()==QEvent::KeyPress)
        {
            QKeyEvent *keyEvent=static_cast<QKeyEvent *>(event);
            if (keyEvent->key()==Qt::Key_Return || keyEvent->key()==Qt::Key_Enter)
            {
                WriteMyCom();
                return true;
            }
        }
    }

    return QObject::eventFilter(obj,event);
}

void frmMain::InitForm()
{    
    ReceiveCount=0;
    SendCount=0;
    IsShow=true;
    IsAutoClear=false;
    IsHexSend=true;
    IsHexReceive=true;
    IsDebug=false;

    QStringList comList;//串口号
    QStringList baudList;//波特率
    QStringList parityList;//校验位
    QStringList dataBitsList;//数据位
    QStringList stopBitsList;//停止位

#ifdef Q_OS_WIN//如果是windows系统
    comList<<"COM1"<<"COM2"<<"COM3"<<"COM4"<<"COM5"<<"COM6"
          <<"COM7"<<"COM8"<<"COM9"<<"COM10"<<"COM11"<<"COM12"
         <<"COM13"<<"COM14"<<"COM15";
#else//如果是unix或者其他系统
    comList<<"ttyUSB0"<<"ttyUSB1"<<"ttyUSB2"<<"ttyUSB3"<<"ttyUSB4"<<"ttyUSB5"
          <<"ttyS0"<<"ttyS1"<<"ttyS2"<<"ttyS3"<<"ttyS4"<<"ttyS5"<<"ttyS6"
         <<"ttyS7"<<"ttyS8"<<"ttyS9";
#endif

    ui->cboxPortName->addItems(comList);
    ui->cboxPortName->setCurrentIndex(0);

    baudList<<"50"<<"75"<<"100"<<"134"<<"150"<<"200"<<"300"
           <<"600"<<"1200"<<"1800"<<"2400"<<"4800"<<"9600"
          <<"14400"<<"19200"<<"38400"<<"56000"<<"57600"
         <<"76800"<<"115200"<<"128000"<<"256000";

    ui->cboxBaudRate->addItems(baudList);
    ui->cboxBaudRate->setCurrentIndex(12);

    parityList<<"无"<<"奇"<<"偶";

#ifdef Q_OS_WIN//如果是windows系统
    parityList<<"标志";
#endif

    parityList<<"空格";

    ui->cboxParity->addItems(parityList);
    ui->cboxParity->setCurrentIndex(0);

    dataBitsList<<"5"<<"6"<<"7"<<"8";
    ui->cboxDataBit->addItems(dataBitsList);
    ui->cboxDataBit->setCurrentIndex(3);

    stopBitsList<<"1";

#ifdef Q_OS_WIN//如果是windows系统
    stopBitsList<<"1.5";
#endif

    stopBitsList<<"2";

    ui->cboxStopBit->addItems(stopBitsList);
    ui->cboxStopBit->setCurrentIndex(0);

    //读取数据(采用定时器读取数据，不采用事件，方便移植到linux)
    myReadTimer=new QTimer(this);
    myReadTimer->setInterval(300);
    connect(myReadTimer,SIGNAL(timeout()),this,SLOT(ReadMyCom()));

    //发送数据
    mySendTimer=new QTimer(this);
    mySendTimer->setInterval(5000);
    connect(mySendTimer,SIGNAL(timeout()),this,SLOT(WriteMyCom()));

    //保存数据
    mySaveTimer=new QTimer(this);
    mySaveTimer->setInterval(5000);
    connect(mySaveTimer,SIGNAL(timeout()),this,SLOT(SaveMyCom()));

    //显示日期时间
    myTimer=new QTimer(this);
    myTimer->start(1000);
    connect(myTimer,SIGNAL(timeout()),this,SLOT(SetTime()));

    QDate dateNow=QDate::currentDate();
    ui->labDate->setText(QString("日期:%1").arg(dateNow.toString("yyyy年MM月dd日 dddd")));

    for (int i=1;i<=60;i++)
    {
        ui->cboxSend->addItem(QString::number(i)+"秒");
        ui->cboxSave->addItem(QString::number(i)+"秒");
    }

    ui->cboxSave->setCurrentIndex(4);
    ui->cboxSend->setCurrentIndex(4);

    ui->cboxSend->setEnabled(false);
    ui->cboxSave->setEnabled(false);

    this->ChangeEnable(false);
    this->ReadConfigData();//读取发送数据加载到下拉框
    this->ReadSendData();//读取数据转发文件

    ui->txtSend->installEventFilter(this);//安装监听器监听发送数据框回车响应
}

void frmMain::ReadSendData()
{
    QString fileName="SendData.txt";
    QFile file(fileName);
    if (!file.exists()){return;}//如果文件不存在则返回

    file.open(QFile::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    QString line;

    do { line=in.readLine();if (line!="") { SendDataList.append(line); }}
    while(!line.isNull());

    file.close();
}

void frmMain::ChangeEnable(bool b)
{
    ui->cboxBaudRate->setEnabled(!b);
    ui->cboxDataBit->setEnabled(!b);
    ui->cboxParity->setEnabled(!b);
    ui->cboxPortName->setEnabled(!b);
    ui->cboxStopBit->setEnabled(!b);

    ui->gbox2->setEnabled(b);
    ui->gbox3->setEnabled(b);
    //ui->gbox5->setEnabled(b);
    ui->gbox6->setEnabled(b);
    ui->btnOpen->setEnabled(true);
}

void frmMain::SetTime()
{
    QTime timeNow=QTime::currentTime();
    ui->labTime->setText(QString("时间:%1").arg(timeNow.toString()));
}

void frmMain::ReadMyCom()
{
    //这个判断尤为重要,否则的话直接延时再接收数据,空闲时和会出现高内存占用
    if (myCom->bytesAvailable()<=0){return;}

    myHelper::Sleep(20);//延时100毫秒保证接收到的是一条完整的数据,而不是脱节的
    QByteArray buffer=myCom->readAll();
    QString        read_time   =  QDateTime::currentDateTime().toString("hh:mm:ss:zzz");
    //--------------------------------lin  shi --------------------
    QString tempDataHex=myHelper::ByteArrayToHexStr(buffer);
    if (IsShow)
    {
        if (IsHexReceive)
        {
            //----------------------------Add receive judgment function------------------------
            handle_data( read_time, buffer);
            /**
             * @brief tempDataHex
            QString tempDataHex=myHelper::ByteArrayToHexStr(buffer);
            ui->txtDataHex->append(QString("%1%2")
                                   .arg("["+read_time+"] ")
                                   .arg(tempDataHex));
               */
            if (IsDebug)//2013-8-6增加接收数据后转发数据，模拟设备
            {
                foreach(QString tempData,SendDataList)
                {
                    QStringList temp=tempData.split(';');
                    if (tempDataHex==temp[0])
                    {
                        //这里没有跳出循环，有可能一条数据会对应多条数据需要转发
                        myCom->write(myHelper::HexStrToByteArray(temp[1]));
                    }
                }
            }
        }
        else
        {
            QString tempDataNormal=QString(buffer);
            //            ui->txtDataHex->append(QString("接收:%1 时间:%2")
            //                                   .arg(tempDataNormal)
            //                                   .arg(QTime::currentTime().toString("HH:mm:ss")));
            ui->txtDataHex->append(QString("%1%2")
                                   .arg("["+read_time+"] ")
                                   .arg(tempDataNormal));

            if (IsDebug)//2013-8-6增加接收数据后转发数据，模拟设备
            {
                foreach(QString tempData,SendDataList)
                {
                    QStringList temp=tempData.split(';');
                    if (tempDataNormal==temp[0])
                    {
                        //这里没有跳出循环，有可能一条数据会对应多条数据需要转发
                        myCom->write(temp[1].toAscii());
                    }
                }
            }
        }

        ReceiveCount=ReceiveCount+buffer.size();
        ui->labReceive->setText(QString("接收:%1 字节").arg(ReceiveCount));
    }
}

void frmMain::WriteMyCom()
{
    QString str=ui->txtSend->currentText();
    if (str==""){ui->txtSend->setFocus();return;}//发送数据为空
    if (!myCom->isOpen()) { return; }//串口没有打开

    QByteArray outData=str.toAscii();
    int size=outData.size();

    if (IsHexSend)//转化为16进制发送
    {
        outData=myHelper::HexStrToByteArray(str);
        size=outData.size();
        myCom->write(outData);
    }
    else
    {
        size=outData.size();
        myCom->write(outData);
    }

    //    ui->txtDataHex->append(QString("发送:%1 时间:%2")
    //                           .arg(str)
    //                           .arg(QTime::currentTime().toString("HH:mm:ss")));
    QString        read_time   =  QDateTime::currentDateTime().toString("hh:mm:ss:zzz");
    ui->txtDataHex->append(QString("%1%2")
                           .arg("["+read_time+"] ")
                           .arg(str));

    SendCount=SendCount+size;
    ui->labSend->setText(QString("发送:%1 字节").arg(SendCount));

    if (IsAutoClear)
    {
        ui->txtSend->setCurrentIndex(-1);
        ui->txtSend->setFocus();
    }
}
//------------------------------------------send commond_send-------------------------------------------------------
void frmMain::WriteMyCom(QByteArray& commond_send)
{
    if (!myCom->isOpen()) { return; }//串口没有打开

    ui->txtDataHex->append("发送:");
    mutex.lock();
    for(int i = 0;i<3 ;i++){
        commond_send[3]  = (commond_send[3]+0x01)&0xff;
        commond_send[31] = 0x00;
        for(int n = 0;n < commond_send.size()-1;n++){
            commond_send[31] = commond_send[31]+commond_send[n];
        }
        commond_send[31] = commond_send[31]&0xff;

        int size=commond_send.size();
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        qDebug()<<commond_send.toHex();
        myCom->write(commond_send);
        ui->txtDataHex->append(QString("%1%2")
                               .arg("["+QDateTime::currentDateTime().toString("hh:mm:ss:zzz")+"] ")
                               .arg(myHelper::ByteArrayToHexStr(commond_send)));
        SendCount=SendCount+size;
        ui->labSend->setText(QString("发送:%1 字节").arg(SendCount));
        myHelper::Sleep(20);//延时20毫秒
    }
    mutex.unlock();
    ui->txtDataHex->append(" ");
    if (IsAutoClear)
    {
        ui->txtSend->setCurrentIndex(-1);
        ui->txtSend->setFocus();
    }
}

void frmMain::WriteMyCom_custom()
{
    if(!myCom->isOpen()){return;}
    QString   tmpstr  =  ui->lineEdit->text();
    //------------------------------------------------------------
    qDebug()<< tmpstr;
    QByteArray  send_data ;
    send_data.append(myHelper::HexStrToByteArray(tmpstr));
    //-------------------------------------------------------------
    qDebug()<<send_data.toHex();
    send_data[3] = commond_send[3];
    ui->txtDataHex->append("自定义命令发送:");
    for(int i = 0 ; i <3 ;i++){
        //-------------------判断自定义命令是不是为空----------------
        if(NULL ==  tmpstr)
            break;
        send_data[3] = send_data[3]+0x01;
        if(32 == send_data.size()) // for  sum
        {
            send_data[31] = 0x00;
            for(int n = 0;n < send_data.size()-1;n++){
                send_data[31] = send_data[31]+send_data[n];
            }
            send_data[31] = send_data[31]&0xff;
        }else{// not   count
        }
        int size=send_data.size();
        myCom->write(send_data);
        ui->txtDataHex->append(QString("%1%2")
                               .arg("["+QDateTime::currentDateTime().toString("hh:mm:ss:zzz")+"] ")
                               .arg(myHelper::ByteArrayToHexStr(send_data)));
        SendCount=SendCount+size;
        ui->labSend->setText(QString("发送:%1 字节").arg(SendCount));
        myHelper::Sleep(20);//延时20毫秒
    }
    commond_send[3] = send_data[3];
    ui->txtDataHex->append(" ");
    send_data.clear();
    if (IsAutoClear)
    {
        ui->txtSend->setCurrentIndex(-1);
        ui->txtSend->setFocus();
    }

}

void frmMain::SaveMyCom()
{
    QString tempData=ui->txtDataHex->toPlainText();
    if (tempData==""){return;}//如果没有内容则不保存

    QDateTime now=QDateTime::currentDateTime();
    QString name=now.toString("yyyyMMddHHmmss");
    QString fileName=name+".txt";

    QFile file(fileName);
    file.open(QFile::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out<<tempData;
    file.close();
}

void frmMain::on_btnOpen_clicked()
{
    if (ui->btnOpen->text()=="打开串口")
    {
        QString portName=ui->cboxPortName->currentText();

#ifdef Q_OS_WIN//如果是windows系统
        myCom = new QextSerialPort(portName);
#else
        myCom = new QextSerialPort("/dev/" + portName);
#endif        

        if (myCom->open(QIODevice::ReadWrite))
        {
            //清空缓冲区
            myCom->flush();
            //设置波特率
            myCom->setBaudRate((BaudRateType)ui->cboxBaudRate->currentText().toInt());
            //设置数据位
            myCom->setDataBits((DataBitsType)ui->cboxDataBit->currentText().toInt());
            //设置校验位
            myCom->setParity((ParityType)ui->cboxParity->currentIndex());
            //设置停止位
            myCom->setStopBits((StopBitsType)ui->cboxStopBit->currentIndex());
            myCom->setFlowControl(FLOW_OFF);
            myCom->setTimeout(10);

            this->ChangeEnable(true);
            ui->btnOpen->setText("关闭串口");
            ui->labIsOpen->setText("串口状态：打开");
            this->myReadTimer->start();
        }
    }
    else
    {
        myCom->close();
        this->ChangeEnable(false);
        ui->btnOpen->setText("打开串口");
        ui->labIsOpen->setText("串口状态：关闭");
        this->myReadTimer->stop();

        //这样的话保证每次关闭串口后,自动发送和自动保存定时器不会空转
        ui->ckIsAutoSend->setChecked(false);
        ui->ckIsAutoSave->setChecked(false);
    }
}

void frmMain::on_ckHexSend_stateChanged(int arg1)
{
    IsHexSend=(arg1==0?false:true);
}

void frmMain::on_ckHexReceive_stateChanged(int arg1)
{
    IsHexReceive=(arg1==0?false:true);
}

void frmMain::on_ckIsAutoSend_stateChanged(int arg1)
{    
    bool IsAutoSend=(arg1==0?false:true);
    if (IsAutoSend)
    {
        this->mySendTimer->start();
    }
    else
    {
        this->mySendTimer->stop();
    }
    ui->cboxSend->setEnabled(IsAutoSend);
}

void frmMain::on_ckIsAutoSave_stateChanged(int arg1)
{
    bool IsAutoSave=(arg1==0?false:true);
    if (IsAutoSave)
    {
        this->mySaveTimer->start();
    }
    else
    {
        this->mySaveTimer->stop();
    }
    ui->cboxSave->setEnabled(IsAutoSave);
}

void frmMain::on_ckIsAutoClear_stateChanged(int arg1)
{
    IsAutoClear=(arg1==0?false:true);
}

void frmMain::on_ckIsDebug_stateChanged(int arg1)
{
    IsDebug=(arg1==0?false:true);
}

void frmMain::on_btnClearSend_clicked()
{
    SendCount=0;
    ui->labSend->setText("发送:0 字节");
}

void frmMain::on_btnClearReceive_clicked()
{
    ReceiveCount=0;
    ui->labReceive->setText("接收:0 字节");
}

void frmMain::on_cboxSend_currentIndexChanged(int index)
{
    mySendTimer->setInterval((index+1)*1000);
}

void frmMain::on_cboxSave_currentIndexChanged(int index)
{
    mySaveTimer->setInterval((index+1)*1000);
}

void frmMain::on_btnStopShow_clicked()
{
    if (ui->btnStopShow->text()=="停止显示")
    {
        IsShow=false;
        ui->btnStopShow->setText("开始显示");
    }
    else
    {
        IsShow=true;
        ui->btnStopShow->setText("停止显示");
    }
}

void frmMain::on_btnClearAll_clicked()
{    
    ui->txtDataHex->clear();
}

void frmMain::on_btnSend_clicked()
{
    this->WriteMyCom();
}

void frmMain::on_btnSave_clicked()
{
    this->SaveMyCom();
}

void frmMain::ReadConfigData()
{    
    QString fileName="ConfigData.txt";
    QFile file(fileName);
    if (!file.exists()){return;}

    ui->txtSend->clear();
    file.open(QFile::ReadOnly | QIODevice::Text);
    QTextStream in(&file);
    QString line;

    do{ line=in.readLine();if (line!="") {ui->txtSend->addItem(line);}}
    while(!line.isNull());

    file.close();
}

void frmMain::on_btnData_clicked()
{
    QString fileName="ConfigData.txt";
    QFile file(fileName);

    if (!file.exists()){ myHelper::ShowMessageBoxError("数据文件不存在！"); return;}

    if (ui->btnData->text()=="管理数据")
    {
        ui->txtDataHex->setReadOnly(false);
        ui->gbox2->setTitle("管理数据");
        ui->txtDataHex->clear();

        file.open(QFile::ReadOnly | QIODevice::Text);
        QTextStream in(&file);
        ui->txtDataHex->setText(in.readAll());
        file.close();

        ui->btnData->setText("保存数据");
    }
    else
    {
        ui->txtDataHex->setReadOnly(true);
        ui->gbox2->setTitle("接收数据");

        file.open(QFile::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out<<ui->txtDataHex->toPlainText();
        file.close();

        ui->txtDataHex->clear();
        ui->btnData->setText("管理数据");

        this->ReadConfigData();
    }
}
//-------------------------init  commond  send  and   receive--------------------------
void frmMain::init_commond_send_data()
{
    commond_send[0] = 0xEB;
    commond_send[1] = 0x90;
    commond_send[2] = 0x4F;
    for(int i  = 4;i < commond_send.size();i++){
        commond_send[i] = 0x00;
    }

}

void frmMain::init_commond_receive_data()
{
    for(int i = 0 ; i < commond_receive.size() ; i++)
        commond_receive[i] = 0x00;
}

//---------------------------------------set receive_textEdit ----------------------------------s
void frmMain::handle_data(QString read_time, QByteArray read_data)
{
    QString  _read_time  = read_time;
    QByteArray  _read_data = read_data;
    //QByteArray   add_num;
    if((32==_read_data.count()) && (0xffffffeb== _read_data.at(0)) && ( 0xffffff90 ==_read_data[1]  )){
        for(int  i  = 0 ;i <_read_data.size()-1;i++ ){
            check_num[1]=check_num[1]+_read_data[i];
        }
        check_num[1] = check_num[1]&0xff;
        //--------------------校检位与发送计数--------------------------
        if((check_num.at(1) ==_read_data.at(31))&&( 0x01 == _read_data.at(2) - check_num.at(0))){
            //---------------------判断是不是火共品-----------------------
            if( (0x04==_read_data[6])||(0x05==_read_data[6])||(0x04==_read_data[8])||(0x05==_read_data[8])){
                //-------------first   RF--------------
                switch (_read_data[6]&0xff ) {
                case 0x04://预点火
                    ui->txtDataHex->setTextColor("lightGray");
                    // ui->txtDataHex->setTextBackgroundColor("black");
                    qDebug()<<"color_04F:lightGray";
                    switch (_read_data[4]&0xff ) {  //    (0xB6==_read_data[5])&&( _read_data[5]== _read_data[4])                                    (0x04==_read_data[6])&& (0xB6 == _read_data[05])&&(0xB6 == _read_data[4])&&(0xA5==_read_data[8]==_read_data[9])
                    case 0xB6:
                        qDebug()<<"----------------------------------------------";
                        ui->txtDataHex->setTextColor(QColor("blue"));
                        ui->txtDataHex->append("预点火RF(接收数据错误):");
                        judge_num++;
                        if(judge_num == 3){
                            judge_num = 0;
                            send_cf_signal_04(false);
                        }
                        break;
                    case 0xA5://(0x04==_read_data[6])&& (0xB6 == _read_data[5])&&(0xA5 == _read_data[4])&&(0xA5==_read_data[8]==_read_data[9])
                        qDebug()<<"++++++++++++++++++++++++++++++";
                        ui->txtDataHex->setTextColor(QColor("green"));
                        ui->txtDataHex->append("预点火RF(接收数据正确):");
                        judge_num++;
                        if(judge_num == 3){
                            judge_num = 0;
                            send_cf_signal_04(true);
                        }
                        break;
                        //                    default:
                        //                        qDebug()<<"2222222222222222222222";
                        //                        qDebug("%x",_read_data.at(4));
                        //                        break;
                    }
                    break;
                case 0x05://正式点火
                    ui->txtDataHex->setTextColor("lightGray");
                    //ui->txtDataHex->setTextBackgroundColor("black");
                    qDebug()<<"color_05F:lightGray";
                    //if((0xA5 == _read_data[4])&& (0xB6 == _read_data[5])&&(0x05==_read_data[6])&&(0xB5==_read_data[8]==_read_data[9])&&(0xAB==_read_data[10]==_read_data[11]))
                    //  }else if((0xB6 == _read_data[4])&& (0xB6 == _read_data[5])&&(0x05==_read_data[6])&&(0xB5==_read_data[8]==_read_data[9])&&(0xAB==_read_data[10]==_read_data[11]))
                    switch (_read_data[4]&0xff) {
                    case 0xA5:
                        ui->txtDataHex->setTextColor(QColor("green"));
                        ui->txtDataHex->append("正式点火RF(接收数据正确):");
                        judge_num++;
                        if(judge_num == 3){
                            judge_num = 0;
                            send_cf_signal_05(true,read_data);
                        }
                        break;
                    case 0xB6:
                        ui->txtDataHex->setTextColor(QColor("blue"));
                        ui->txtDataHex->append("正式点火RF(接收数据不正确):");
                        judge_num++;
                        if(judge_num == 3){
                            judge_num = 0;
                            send_cf_signal_05(false,read_data);
                        }
                        break;
                    }
                    break;
                default:
                    //ui->txtDataHex->setTextColor(QColor("black"));
                    break;
                }
                //-----------second  RF--------------
                switch (_read_data[8]&0xff ) {
                // if((0xA5==_read_data[4])&&(0x04==_read_data[8])&&(0x01==_read_data[6]==_read_data[7]))
                //else if ((0xB6==_read_data[4])&&(0x04==_read_data[8])&&(0x01==_read_data[6]==_read_data[7]))
                case 0x04:
                    ui->txtDataHex->setTextColor("lightGray");
                    //ui->txtDataHex->setTextBackgroundColor("black");
                    qDebug()<<"color_04S:lightGray";
                    switch (_read_data[4]&0xff) {
                    case 0xA5:
                        ui->txtDataHex->setTextColor(QColor("cyan"));
                        break;
                    case 0xB6:
                        ui->txtDataHex->setTextColor(QColor("magenta"));
                        break;
                    }

                    break;
                    // if((0xA5==_read_data[4])&&(0x05==_read_data[8])&&(0x01==_read_data[6]==_read_data[7]))
                    //else if ((0xB6==_read_data[4])&&(0x05==_read_data[8])&&(0x01==_read_data[6]==_read_data[7]))
                case 0x05:
                    ui->txtDataHex->setTextColor("lightGray");
                    //ui->txtDataHex->setTextBackgroundColor("black");
                    qDebug()<<"color_05S:lightGray";
                    switch (_read_data[4]) {
                    case 0xA5:
                        ui->txtDataHex->setTextColor(QColor("cyan"));
                        break;
                    case 0xB6:
                        ui->txtDataHex->setTextColor(QColor("magenta"));
                        break;
                    }
                    break;
                default:
                    // ui->txtDataHex->setTextColor(QColor("black"));
                    //qDebug()<<"color:black";
                    break;
                }
                //ui->txtDataHex->setTextColor(QColor("black"));
            }else {
                ui->txtDataHex->setTextColor(QColor("black"));
            }
        }else {
            ui->txtDataHex->setTextColor(QColor("red"));
        }
    }else {
        ui->txtDataHex->setTextColor(QColor("red"));
    }
    QString tempDataHex=myHelper::ByteArrayToHexStr(_read_data);
    ui->txtDataHex->append(QString("%1%2")
                           .arg("["+_read_time+"] ")
                           .arg(tempDataHex));
    check_num[0] =  _read_data.at(2)&0xff;
    qDebug("d :%x",check_num.at(0));
    check_num[1] =  0;
    ui->txtDataHex->moveCursor(QTextCursor::End);  //gun ping dao zui hou
    ui->txtDataHex->setTextColor(QColor("black"));
}

void frmMain::send_cf_slots04(bool A5_B6)
{
    // myHelper::Sleep(60);//延时20毫秒
    ui->txtDataHex->append("发送04-CF帧:");
    QByteArray  commond_send_cf  =commond_send;
    for(int j = 0;j<3;j++){
        commond_send_cf[3]  = (commond_send_cf[3]+0x01)&0xff;
        //---------------------------------------------------------------------------
        commond_send_cf[4]=0x01;
        commond_send_cf[5]=0x01;
        commond_send_cf[6]=0x01;

        commond_send_cf[7]=0x04;
        commond_send_cf[8]=0x04;
        commond_send_cf[9]=0x04;
        commond_send_cf[10]=0x04;

        if(A5_B6){
            commond_send_cf[11]=0xA5;
            commond_send_cf[12]=0xA5;
        }else {
            commond_send_cf[11]=0xB6;
            commond_send_cf[12]=0xB6;
        }
        //---------------------------------------------------------------------------
        commond_send_cf[31] = 0x00;
        for(int n = 0;n < commond_send_cf.size()-1;n++){
            commond_send_cf[31] = commond_send_cf[31]+commond_send_cf[n];
        }
        commond_send_cf[31] = commond_send_cf[31]&0xff;
        set_txtDataHex(commond_send_cf);
        myCom->write(commond_send_cf);
        myHelper::Sleep(20);
    }
    ui->txtDataHex->append(" ");
    commond_send[3] =commond_send_cf[3];

}

void frmMain::send_cf_slots05(bool A5_B6,QByteArray channel_num)
{
    //  myHelper::Sleep(60);//延时20毫秒
    ui->txtDataHex->append("发送05-CF帧:");
    QByteArray  commond_send_cf  =commond_send;
    for(int j = 0;j<3;j++){
        commond_send_cf[3]  = (commond_send_cf[3]+0x01)&0xff;
        //---------------------------------------------------------------------------
        commond_send_cf[4]=0x01;
        commond_send_cf[5]=0x01;
        commond_send_cf[6]=0x01;

        commond_send_cf[7]=0x07;
        commond_send_cf[8]=0x07;
        commond_send_cf[9]=0x05;
        commond_send_cf[10]=0x05;

        commond_send_cf[13] = channel_num[12];
        commond_send_cf[14] = channel_num[12];
        commond_send_cf[15] = channel_num[12];

        if(A5_B6){
            commond_send_cf[11]=0xA5;
            commond_send_cf[12]=0xA5;
        }else {
            commond_send_cf[11]=0xB6;
            commond_send_cf[12]=0xB6;
        }
        //---------------------------------------------------------------------------
        commond_send_cf[31] = 0x00;
        for(int n = 0;n < commond_send_cf.size()-1;n++){
            commond_send_cf[31] = commond_send_cf[31]+commond_send_cf[n];
        }
        commond_send_cf[31] = commond_send_cf[31]&0xff;
        set_txtDataHex(commond_send_cf);
        myCom->write(commond_send_cf);
        myHelper::Sleep(20);
    }
    ui->txtDataHex->append(" ");
    commond_send[3] =commond_send_cf[3];

}

//----------------------tabWidget  change -----------------------------------------------
void frmMain::on_tabWidget_currentChanged(int index)
{
    qDebug()<<index;
    QByteArray  custom_data;
    switch (index) {
    case 0:
        init_commond_send_data();
        init_commond02_data();

        custom_data = commond_send;
        ui->lineEdit->clear();
        ui->lineEdit->insert(myHelper::ByteArrayToHexStr(custom_data));
        break;
    case 1:
        init_commond_send_data();
        init_commond04_data();

        custom_data = commond_send;
        custom_data[9] = 0xA5;
        custom_data[10] = 0xA5;
        ui->lineEdit->clear();
        ui->lineEdit->insert(myHelper::ByteArrayToHexStr(custom_data));
        break;
    case 2:
        init_commond_send_data();
        init_commond05_data();

        custom_data = commond_send;
        custom_data[13] = 0x91;
        custom_data[14] = 0x91;
        custom_data[15] = 0x91;

        ui->lineEdit->clear();
        ui->lineEdit->insert(myHelper::ByteArrayToHexStr(custom_data));
        break;
    case 3:
        init_commond_send_data();
        init_commond06_data();

        custom_data = commond_send;
        custom_data[9] = 0xA5;
        custom_data[10] = 0xA5;
        ui->lineEdit->clear();
        ui->lineEdit->insert(myHelper::ByteArrayToHexStr(custom_data));
        break;
    case 4:
        init_commond_send_data();
        init_commond07_data();

        custom_data = commond_send;
        custom_data[9] = 0xA5;
        custom_data[10] = 0xA5;
        ui->lineEdit->clear();
        ui->lineEdit->insert(myHelper::ByteArrayToHexStr(custom_data));
        break;
    case 5:
        init_commond_send_data();
        init_commond08_data();

        custom_data = commond_send;
        custom_data[9] = 0x91;
        custom_data[10] = 0x91;
        ui->lineEdit->clear();
        ui->lineEdit->insert(myHelper::ByteArrayToHexStr(custom_data));
        break;
    case 6:
        init_commond_send_data();
        init_commond09_data();

        custom_data = commond_send;
        custom_data[9] = 0x00;
        custom_data[10] = 0xCA;
        custom_data[11] = 0x9A;
        custom_data[12] = 0x3B;

        custom_data[13] = 0x00;
        custom_data[14] = 0x24;
        custom_data[15] = 0xF4;

        custom_data[16] = 0x00;
        custom_data[17] = 0xCA;
        custom_data[18] = 0x9A;
        custom_data[19] = 0x3B;

        ui->lineEdit->clear();
        ui->lineEdit->insert(myHelper::ByteArrayToHexStr(custom_data));
        break;
    case 7:
        init_commond_send_data();
        init_commond0A_data();/*

        custom_data = commond_send;
        ui->lineEdit->clear();
        ui->lineEdit->insert(myHelper::ByteArrayToHexStr(custom_data));*/
        break;
    case 8:
        init_commond_send_data();
        init_commond0B_data();

        custom_data = commond_send;

        ui->lineEdit->clear();
        ui->lineEdit->insert(myHelper::ByteArrayToHexStr(custom_data));
        break;
    case 9:
        init_commond_send_data();
        init_commond0C_data();

        custom_data = commond_send;
        ui->lineEdit->clear();
        ui->lineEdit->insert(myHelper::ByteArrayToHexStr(custom_data));
        break;
    case 10:
        init_commond_send_data();
        init_commond0D_data();

        custom_data = commond_send;
        custom_data[9] = 0xA5;
        custom_data[10] = 0xA5;
        ui->lineEdit->clear();
        ui->lineEdit->insert(myHelper::ByteArrayToHexStr(custom_data));
        break;
    case 11:
        init_commond_send_data();
        init_commond0E_data();

        custom_data = commond_send;
        custom_data[9] = 0x01;
        custom_data[10] = 0x10;
        ui->lineEdit->clear();
        ui->lineEdit->insert(myHelper::ByteArrayToHexStr(custom_data));
        break;
    default:
        break;
    }
}

void frmMain::init_commond02_data()
{
    commond_send[4] = 0x02;
    commond_send[5] = 0x02;
    commond_send[6] = 0x02;
    commond_send[7] = 0x00;
    commond_send[8] = 0x00;
}

void frmMain::init_commond04_data()
{
    commond_send[4] = 0x04;
    commond_send[5] = 0x04;
    commond_send[6] = 0x04;
    commond_send[7] = 0x02;
    commond_send[8] = 0x02;

}

void frmMain::init_commond05_data()
{
    commond_send[4] = 0x05;
    commond_send[5] = 0x05;
    commond_send[6] = 0x05;
    commond_send[7] = 0x07;
    commond_send[8] = 0x07;

    commond_send[9] = 0xB5;
    commond_send[10] = 0xB5;
    commond_send[11] = 0xA6;
    commond_send[12] = 0xA6;


}

void frmMain::init_commond06_data()
{
    commond_send[4] = 0x06;
    commond_send[5] = 0x06;
    commond_send[6] = 0x06;
    commond_send[7] = 0x02;
    commond_send[8] = 0x02;
}

void frmMain::init_commond07_data()
{
    commond_send[4] = 0x07;
    commond_send[5] = 0x07;
    commond_send[6] = 0x07;
    commond_send[7] = 0x02;
    commond_send[8] = 0x02;
}

void frmMain::init_commond08_data()
{
    commond_send[4] = 0x08;
    commond_send[5] = 0x08;
    commond_send[6] = 0x08;
    commond_send[7] = 0x02;
    commond_send[8] = 0x02;
}

void frmMain::init_commond09_data()
{
    commond_send[4] = 0x09;
    commond_send[5] = 0x09;
    commond_send[6] = 0x09;
    commond_send[7] = 0x0A;
    commond_send[8] = 0x0A;
}

void frmMain::init_commond0A_data()
{
    commond_send[4] = 0x0A;
    commond_send[5] = 0x0A;
    commond_send[6] = 0x0A;
    commond_send[7] = 0x02;
    commond_send[8] = 0x02;
}

void frmMain::init_commond0B_data()
{
    commond_send[4] = 0x0B;
    commond_send[5] = 0x0B;
    commond_send[6] = 0x0B;
    commond_send[7] = 0x07;
    commond_send[8] = 0x07;
}

void frmMain::init_commond0C_data()
{
    commond_send[4] = 0x0C;
    commond_send[5] = 0x0C;
    commond_send[6] = 0x0C;
    commond_send[7] = 0x0C;
    commond_send[8] = 0x0C;
}

void frmMain::init_commond0D_data()
{
    commond_send[4] = 0x0D;
    commond_send[5] = 0x0D;
    commond_send[6] = 0x0D;
    commond_send[7] = 0x02;
    commond_send[8] = 0x02;
}

void frmMain::init_commond0E_data()
{
    commond_send[4] = 0x0E;
    commond_send[5] = 0x0E;
    commond_send[6] = 0x0E;
    commond_send[7] = 0x04;
    commond_send[8] = 0x04;
}

//------------------------------- For custom command display  --------------------------------------------
void frmMain::set_txtDataHex(QByteArray &commond)
{
    QString        read_time   =  QDateTime::currentDateTime().toString("hh:mm:ss:zzz");
    QString tempDataHex=myHelper::ByteArrayToHexStr(commond);
    ui->txtDataHex->append(QString("%1%2")
                           .arg("["+read_time+"] ")
                           .arg(tempDataHex));
}

//-----------------------------commond send-----------------------------------------------------------------

void frmMain::on_ID02send_pushButton_4_clicked()
{
    if (!myCom->isOpen()) { return; }//串口没有打开
    WriteMyCom(commond_send);
}

void frmMain::on_ID04send_pushButton_4_clicked()
{
    if (!myCom->isOpen()) { return; }//串口没有打开
    int num_instruct = ui->instruct04_comboBox_4->currentIndex();
    switch (num_instruct) {
    case 0:
        commond_send[9] = 0xA5;
        commond_send[10] = 0xA5;
        break;
    case 1:
        commond_send[9] = 0xB6;
        commond_send[10] = 0xB6;
        break;
    default:
        break;
    }
    WriteMyCom(commond_send);
}

void frmMain::on_ID05send_pushButton_4_clicked()
{
    if (!myCom->isOpen()) { return; }//串口没有打开
    int num_instruct = ui->instruct05_comboBox_4->currentIndex();
    switch (num_instruct) {
    case 0:
        commond_send[13] = 0x91;
        commond_send[14] = 0x91;
        commond_send[15] = 0x91;
        break;
    case 1:
        commond_send[13] = 0x92;
        commond_send[14] = 0x92;
        commond_send[15] = 0x92;
        break;
    case 2:
        commond_send[13] = 0x93;
        commond_send[14] = 0x93;
        commond_send[15] = 0x93;
        break;
    case 3:
        commond_send[13] = 0x94;
        commond_send[14] = 0x94;
        commond_send[15] = 0x94;
        break;
    case 4:
        commond_send[13] = 0x95;
        commond_send[14] = 0x95;
        commond_send[15] = 0x95;
        break;
    case 5:
        commond_send[13] = 0x96;
        commond_send[14] = 0x96;
        commond_send[15] = 0x96;
        break;
    case 6:
        commond_send[13] = 0x97;
        commond_send[14] = 0x97;
        commond_send[15] = 0x97;
        break;
    case 7:
        commond_send[13] = 0x98;
        commond_send[14] = 0x98;
        commond_send[15] = 0x98;
        break;
    case 8:
        commond_send[13] = 0x99;
        commond_send[14] = 0x99;
        commond_send[15] = 0x99;
        break;
    case 9:
        commond_send[13] = 0x9A;
        commond_send[14] = 0x9A;
        commond_send[15] = 0x9A;
        break;
    case 10:
        commond_send[13] = 0x9B;
        commond_send[14] = 0x9B;
        commond_send[15] = 0x9B;
        break;
    case 11:
        commond_send[13] = 0x9C;
        commond_send[14] = 0x9C;
        commond_send[15] = 0x9C;
        break;
    case 12:
        commond_send[13] = 0x9D;
        commond_send[14] = 0x9D;
        commond_send[15] = 0x9D;
        break;
    case 13:
        commond_send[13] = 0x9E;
        commond_send[14] = 0x9E;
        commond_send[15] = 0x9E;
        break;
    case 14:
        commond_send[13] = 0x9F;
        commond_send[14] = 0x9F;
        commond_send[15] = 0x9F;
        break;
    default:
        break;
    }
    WriteMyCom(commond_send);
}

void frmMain::on_ID06send_pushButton_4_clicked()
{
    if (!myCom->isOpen()) { return; }//串口没有打开
    int num_instruct = ui->instruct06_comboBox_4->currentIndex();
    switch (num_instruct) {
    case 0:
        commond_send[9] = 0xA5;
        commond_send[10] = 0xA5;
        break;
    case 1:
        commond_send[9] = 0xB6;
        commond_send[10] = 0xB6;
        break;
    default:
        break;
    }
    WriteMyCom(commond_send);
}

void frmMain::on_ID07send_pushButton_4_clicked()
{
    if (!myCom->isOpen()) { return; }//串口没有打开
    int num_instruct = ui->instruct07_comboBox_4->currentIndex();
    switch (num_instruct) {
    case 0:
        commond_send[9] = 0xA5;
        commond_send[10] = 0xA5;
        break;
    case 1:
        commond_send[9] = 0xA9;
        commond_send[10] = 0xA9;
        break;
    case 2:
        commond_send[9] = 0xAA;
        commond_send[10] = 0xAA;
        break;
    default:
        break;
    }
    WriteMyCom(commond_send);
}

void frmMain::on_ID08send_pushButton_4_clicked()
{
    if (!myCom->isOpen()) { return; }//串口没有打开
    int num_instruct = ui->instruct08_comboBox_4->currentIndex();
    switch (num_instruct) {
    case 0:
        commond_send[9] = 0x91;
        commond_send[10] = 0x91;
        break;
    case 1:
        commond_send[9] = 0x92;
        commond_send[10] = 0x92;
        break;
    case 2:
        commond_send[9] = 0x93;
        commond_send[10] = 0x93;
        break;
    case 3:
        commond_send[9] = 0x94;
        commond_send[10] = 0x94;
        break;
    case 4:
        commond_send[9] = 0x95;
        commond_send[10] = 0x95;
        break;
    default:
        break;
    }
    WriteMyCom(commond_send);
}

void frmMain::on_ID09send_pushButton_4_clicked()
{
    if (!myCom->isOpen()) { return; }//串口没有打开
    float  P1 = ui->P1_lineEdit_4->text().toFloat();
    float  P2 = ui->P2_lineEdit_4->text().toFloat();
    float  P3 = ui->P3_lineEdit_4->text().toFloat();

    if(P1<0||P1>1000000 || P2<0 ||P2 >16 || P3 < 0||P3>1000000)
        QMessageBox::warning(this,tr("警告"),tr("P1,P2,P3输入数值范围/格式不正确!"));
    else {
        int  tmp_p1=(1000000.000 - P1)/0.001;
        int  tmp_p2=(16.000000-P2)/0.000001;
        int  tmp_p3=(1000000.000-P3)/0.001;

        commond_send[9]    = tmp_p1&0xff;
        commond_send[10]  = tmp_p1>>8&0xff;
        commond_send[11]  = tmp_p1>>16&0xff;
        commond_send[12]  = tmp_p1>>24&0xff;

        commond_send[13] = tmp_p2&0xff;
        commond_send[14] = tmp_p2>>8&0xff;
        commond_send[15] = tmp_p2>>16&0xff;

        commond_send[16]  = tmp_p3&0xff;
        commond_send[17]  = tmp_p3>>8&0xff;
        commond_send[18]  = tmp_p3>>16&0xff;
        commond_send[19]  = tmp_p3>>24&0xff;
        //++++++++++++++++++++++++++++++++++++++++++++++++++
        qDebug()<<commond_send.toHex();
        WriteMyCom(commond_send);
    }
}

void frmMain::on_ID0Bsend_pushButton_4_clicked()
{
    QByteArray   motor_1 ;
    QByteArray   motor_2 ;
    QByteArray   motor_3 ;
    QByteArray   motor_4 ;

    int motor_1_int;
    int motor_2_int;
    int motor_3_int;
    int motor_4_int;

    motor_1.append(ui->motor1_lineEdit_4->text());
    motor_2.append(ui->motor2_lineEdit_4->text());
    motor_3.append(ui->motor3_lineEdit_4->text());
    motor_4.append(ui->motor4_lineEdit_4->text());
    if( (0 == motor_1.count())|| (0 == motor_2.count())|| (0 == motor_3.count())|| (0 == motor_3.count()))
        QMessageBox::warning(this,tr("警告"),tr("输入值不能位空！"));
    else {
        if(motor_1.at(0) == '-')
        {
            motor_1.remove(0,1);
            motor_1_int =  motor_1.toInt();
            commond_send[9] = motor_1_int&0xff;
            commond_send[10] = motor_1_int>>8|0x80;

        }else{
            motor_1_int =  motor_1.toInt();
            commond_send[9] = motor_1_int&0xff;
            commond_send[10] = motor_1_int>>8&0xff;
        }

        if(motor_2.at(0) == '-')
        {
            motor_2.remove(0,1);
            motor_2_int =  motor_2.toInt();
            commond_send[11] = motor_2_int&0xff;
            commond_send[12] = motor_2_int >> 8 |0x80;
        }else{
            motor_2_int =  motor_2.toInt();
            commond_send[11] = motor_2_int&0xff;
            commond_send[12] = motor_2_int >> 8 & 0xff;
        }
        if(motor_3.at(0) == '-')
        {
            motor_3.remove(0,1);
            motor_3_int =  motor_3.toInt();
            commond_send[13] = motor_3_int&0xff;
            commond_send[14] = motor_3_int >> 8|0x80;
        }else{
            motor_3_int =  motor_3.toInt();
            commond_send[13] = motor_3_int&0xff;
            commond_send[14] = motor_3_int >> 8 & 0xff;
        }
        if(motor_4.at(0) == '-')
        {
            motor_4.remove(0,1);
            motor_4_int =  motor_4.toInt();
            commond_send[15] = motor_4_int  | 0x80;
        }else {
            motor_4_int =  motor_4.toInt();
            commond_send[15] = motor_4_int  & 0xff;
        }
        if (!myCom->isOpen()) { return; }//串口没有打开
        WriteMyCom(commond_send);
    }
}

void frmMain::on_ID0Csend_pushButton_4_clicked()
{
    QByteArray   longitude ;
    QByteArray   latitude ;
    QByteArray   hovertime;

    int  longitude_int;
    int  latitude_int;
    int  hovertime_int;

    longitude.append(this->ui->longitude_lineEdit_4->text());
    latitude.append(this->ui->latitude_lineEdit_4->text());
    hovertime.append(this->ui->hover_time_lineEdit_4->text());

    if((0 == longitude.count())||(0 ==  latitude.count())||(0  == hovertime.count()))
        QMessageBox::warning(this,tr("warning "),tr("Input can not be empty!"));
    else {
        if(longitude.at(0) == '-')
        {
            longitude.remove(0,1);
            longitude_int =  longitude.toInt();
            commond_send[11] = longitude_int&0xff;
            commond_send[12] = longitude_int>>8&0xff;
            commond_send[13] = longitude_int>>16&0xff;
            commond_send[14] = longitude_int>>24|0x80;

        }else{
            longitude_int =  longitude.toInt();
            commond_send[11] = longitude_int&0xff;
            commond_send[12] = longitude_int>>8&0xff;
            commond_send[13] = longitude_int>>16&0xff;
            commond_send[14] = longitude_int>>24&0xff;
        }

        if(latitude.at(0) == '-')
        {
            latitude.remove(0,1);
            latitude_int =  latitude.toInt();
            commond_send[15] = latitude_int&0xff;
            commond_send[16] = latitude_int>>8&0xff;
            commond_send[17] = latitude_int>>16&0xff;
            commond_send[18] = latitude_int>>24|0x80;

        }else{
            latitude_int =  latitude.toInt();
            commond_send[15] = latitude_int&0xff;
            commond_send[16] = latitude_int>>8&0xff;
            commond_send[17] = latitude_int>>16&0xff;
            commond_send[18] = latitude_int>>24&0xff;
        }


        hovertime_int =  hovertime.toInt();
        commond_send[19] = hovertime_int&0xff;
        commond_send[20] = hovertime_int>>8&0xff;
        commond_send[21] = hovertime_int>>16&0xff;

        commond_send[9] = 0x01;
        commond_send[10] = 0x01;


        if (!myCom->isOpen()) { return; }//串口没有打开
        if(longitude_int<-180||longitude_int >180||-90> latitude_int ||latitude_int>90||hovertime <0)
            QMessageBox::warning(this,tr("警告"),tr("经度/纬度/选停时间输入值/范围有误!"));
        else
            WriteMyCom(commond_send);
    }
}

void frmMain::on_ID0Dsend_pushButton_4_clicked()
{
    if (!myCom->isOpen()) { return; }//串口没有打开
    int num_instruct = ui->instruct0D_comboBox_4->currentIndex();
    switch (num_instruct) {
    case 0:
        commond_send[9] = 0xA5;
        commond_send[10] = 0xA5;
        break;
    case 1:
        commond_send[9] = 0xB6;
        commond_send[10] = 0xB6;
        break;
    default:
        break;
    }
    WriteMyCom(commond_send);

}

void frmMain::on_ID0Esend_pushButton_4_clicked()
{
    if (!myCom->isOpen()) { return; }//串口没有打开

    int num_instruct = ui->instruct0E_comboBox_4->currentIndex();
    switch (num_instruct) {
    case 0:
        commond_send[9] = 0x01;
        commond_send[10] = 0x10;
        break;
    case 1:
        commond_send[9] = 0x02;
        commond_send[10] = 0x20;
        break;
    case 2:
        commond_send[9] = 0x03;
        commond_send[10] = 0x30;
        break;
    case 3:
        commond_send[9] = 0x04;
        commond_send[10] = 0x40;
        break;
    case 4:
        commond_send[9] = 0x05;
        commond_send[10] = 0x50;
        break;
    case 5:
        commond_send[9] = 0x06;
        commond_send[10] = 0x60;
        break;
    case 6:
        commond_send[9] = 0x07;
        commond_send[10] = 0x70;
        break;
    default:
        break;
    }
    WriteMyCom(commond_send);

}

//-------------------------------自定义命令发送-------------------
/**
 * @brief frmMain::on_custom_pushButton_clicked
 * 1.间隔20毫秒发送三次
 * 2.帧循环计数加一
 * 3.自动计算校验和
 */
void frmMain::on_custom_pushButton_clicked()
{
    this->WriteMyCom_custom();
}
