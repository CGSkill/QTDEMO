#include "widget.h"
#include "ui_widget.h"
#include "serial_data_thread.h"


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    //---------------------GUI Landscaping------------
    ui->recevie_textEdit->setReadOnly(true);
    ui->lineEdit->setReadOnly(true);

    //-----------------time-------------------------------
    //ui->time_label->setText(QDateTime::currentDateTime().toString("dddd, MMMM dd, yyyy ,hh:mm:ss"));

    //-------------not  send-----------------------
    ui->ID0Asend_pushButton->setEnabled(false);

    //--------------------------------------------------------------------
    commond_send.resize(32);
    commond_receive.resize(1024);
    init_commond_send_data();
    init_commond01_data();

    serialdetection_bool  = true;
    seriaport_open = true;
    seriaport_time_add = true;
    bytes_num  =  0;
    //-----------------------------receive connect---------------------
    connect(&serial,SIGNAL(readyRead()),this,SLOT(receive_serial_data()));
    connect(this,SIGNAL(send_data(QString,QByteArray)),this,SLOT(handle_data(QString,QByteArray)));
    // data_thread.start();
    // connect(&data_thread,SIGNAL(send_data(QByteArray,QString,bool)),this,SLOT(handle_data(QByteArray,QString,bool)));

}

Widget::~Widget()
{
    delete ui;
}

//---------------------------------check  all of   serialport---------------------------------
void Widget::on_serialdetection_button_clicked()
{
    ui->serialdetection_textEdit->clear();
    if(serialdetection_bool){

        foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
        {
            QSerialPort serial;
            serial.setPort(info);
            ui->serialdetection_textEdit->append( "Name : " +info.portName());
            ui->serialdetection_textEdit->append( "Description : " + info.description());
            ui->serialdetection_textEdit->append(  "Manufacturer: " + info.manufacturer());
            ui->serialdetection_textEdit->append( "Serial Number: " + info.serialNumber());
            ui->serialdetection_textEdit->append( "System Location: " + info.systemLocation());
            //            serial.setPort(info);
            if(serial.open(QIODevice::ReadWrite))
                serial.close();
        }

    }else {
        ui->serialdetection_textEdit->clear();
    }
    serialdetection_bool = !serialdetection_bool ;
}

//------------------------------open serialport----------------------------------------------
void Widget::on_openserial_button_clicked()
{
    //serial port   initializtion
    serial.setPortName(ui->serial_lineEdit->text());
    //  serial.setBaudRate(QSerialPort::Baud38400,QSerialPort::AllDirections);
    //    serial.setPortName("/dev/ttyUSB0");
    serial.setBaudRate(QSerialPort::Baud19200,QSerialPort::AllDirections);
    serial.setDataBits(QSerialPort::Data8);
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);
    serial.setFlowControl(QSerialPort::NoFlowControl);
    if(!serial.open(QIODevice::ReadWrite) ){
        QMessageBox::warning(this,tr("警告"),tr("串口没能打开!"));
    }else{
        ui->serial_lineEdit->setEnabled(false);
        seriaport_open = false;
        // data_thread.start();
    }

}

//------------------------------------close   serialport-------------------------------------
void Widget::on_closeserial_button_clicked()
{
    serial.close();
    ui->serial_lineEdit->setEnabled(true);
    seriaport_open = true;
    // data_thread.stop();
}

//-------------------------init  commond  send  and   receive--------------------------
void Widget::init_commond_send_data()
{
    commond_send[0] = 0xEB;
    commond_send[1] = 0x90;
    commond_send[2] = 0x4F;
    for(int i  = 4;i < commond_send.size();i++){
        commond_send[i] = 0x00;
    }

}

void Widget::init_commond_receive_data()
{
    for(int i = 0 ; i < commond_receive.size() ; i++)
        commond_receive[i] = 0x00;
}

//---------------------------------------set receive_textEdit ----------------------------------
void Widget::receive_serial_data()
{
    QString        read_time   =  QDateTime::currentDateTime().toString("hh:mm:ss:zzz");
    QByteArray  read_data  =  serial.readAll();
    emit  send_data(read_time,read_data);
    read_data.clear();
}

void Widget::handle_data(QString read_time, QByteArray read_data)
{
    QString  _read_time  = read_time;
    QByteArray  _read_data = read_data;
    if((32==_read_data.count()) &&   (0xffffffeb== _read_data.at(0))  && ( 0xffffff90 ==_read_data[1]  )){
        ui->recevie_textEdit->setTextColor(QColor("black"));
    }else {
        ui->recevie_textEdit->setTextColor(QColor("red"));
    }
    ui->recevie_textEdit->insertPlainText("["+_read_time+"] ");
    for(int i = 0; i<_read_data.size();i++){
        unsigned  char   tmp  = _read_data.at(i);
        QString str = QString::number(tmp&0xff,16);
        if(tmp<=15)
            str .insert(0,"0");
        ui->recevie_textEdit->insertPlainText(str+" ");
    }
    ui->recevie_textEdit->insertPlainText("\n");
    _read_data.clear();
    ui->recevie_textEdit->moveCursor(QTextCursor::End);  //gun ping dao zui hou
}
/**
 * @brief Widget::handle_data
 * @param handle_ok_serialdata
 * @param time
 * @param true_false

void Widget::handle_data(QByteArray handle_ok_serialdata, QString time, bool true_false)
{
    if(true_false){
        ui->recevie_textEdit->setTextColor(QColor("black"));
    }else {
        ui->recevie_textEdit->setTextColor(QColor("red"));
    }

    ui->recevie_textEdit->insertPlainText("["+time+"] ");
    int i = 0;
    for(; i<handle_ok_serialdata.size();i++){
        unsigned  char   tmp  = handle_ok_serialdata.at(i);
        QString str = QString::number(tmp&0xff,16);
        if(tmp<=15)
            str .insert(0,"0");
        ui->recevie_textEdit->insertPlainText(str+" ");
        if(i == 32)
            ui->recevie_textEdit->insertPlainText("\n");
    }
    if(i !=32)
        ui->recevie_textEdit->insertPlainText("\n");
}
 */




//----------------------tabWidget  change -----------------------------------------------
void Widget::on_tabWidget_currentChanged(int index)
{
    qDebug()<<index;
    switch (index) {
    case 0:
        init_commond_send_data();
        init_commond01_data();
        break;
    case 1:
        init_commond_send_data();
        init_commond02_data();
        break;
    case 2:
        break;
    case 3:
        init_commond_send_data();
        init_commond04_data();
        break;
    case 4:
        init_commond_send_data();
        init_commond05_data();
        break;
    case 5:
        init_commond_send_data();
        init_commond06_data();
        break;
    case 6:
        init_commond_send_data();
        init_commond07_data();
        break;
    case 7:
        init_commond_send_data();
        init_commond08_data();
        break;
    case 8:
        init_commond_send_data();
        init_commond09_data();
        break;
    case 9:
        init_commond_send_data();
        init_commond0A_data();
        break;
    case 10:
        init_commond_send_data();
        init_commond0B_data();
        break;
    case 11:
        init_commond_send_data();
        init_commond0C_data();
        break;
    case 12:
        init_commond_send_data();
        init_commond0D_data();
        break;
    case 13:
        init_commond_send_data();
        init_commond0E_data();
        break;
    default:
        break;
    }
}

void Widget::init_commond01_data()
{
    commond_send[4] = 0x01;
    commond_send[5] = 0x01;
    commond_send[6] = 0x01;
    commond_send[7] = 0x04;
    commond_send[8] = 0x04;
    //ui->targetID_comboBox->currentIndex();

}

void Widget::init_commond02_data()
{
    commond_send[4] = 0x02;
    commond_send[5] = 0x02;
    commond_send[6] = 0x02;
    commond_send[7] = 0x00;
    commond_send[8] = 0x00;
}

void Widget::init_commond04_data()
{
    commond_send[4] = 0x04;
    commond_send[5] = 0x04;
    commond_send[6] = 0x04;
    commond_send[7] = 0x02;
    commond_send[8] = 0x02;

}

void Widget::init_commond05_data()
{
    commond_send[4] = 0x05;
    commond_send[5] = 0x05;
    commond_send[6] = 0x05;
    commond_send[7] = 0x07;
    commond_send[8] = 0x07;

}

void Widget::init_commond06_data()
{
    commond_send[4] = 0x06;
    commond_send[5] = 0x06;
    commond_send[6] = 0x06;
    commond_send[7] = 0x02;
    commond_send[8] = 0x02;
}

void Widget::init_commond07_data()
{
    commond_send[4] = 0x07;
    commond_send[5] = 0x07;
    commond_send[6] = 0x07;
    commond_send[7] = 0x02;
    commond_send[8] = 0x02;
}

void Widget::init_commond08_data()
{
    commond_send[4] = 0x08;
    commond_send[5] = 0x08;
    commond_send[6] = 0x08;
    commond_send[7] = 0x02;
    commond_send[8] = 0x02;
}

void Widget::init_commond09_data()
{
    commond_send[4] = 0x09;
    commond_send[5] = 0x09;
    commond_send[6] = 0x09;
    commond_send[7] = 0x0A;
    commond_send[8] = 0x0A;
}

void Widget::init_commond0A_data()
{
    commond_send[4] = 0x0A;
    commond_send[5] = 0x0A;
    commond_send[6] = 0x0A;
    commond_send[7] = 0x02;
    commond_send[8] = 0x02;
}

void Widget::init_commond0B_data()
{
    commond_send[4] = 0x0B;
    commond_send[5] = 0x0B;
    commond_send[6] = 0x0B;
    commond_send[7] = 0x07;
    commond_send[8] = 0x07;
}

void Widget::init_commond0C_data()
{
    commond_send[4] = 0x0C;
    commond_send[5] = 0x0C;
    commond_send[6] = 0x0C;
    commond_send[7] = 0x0C;
    commond_send[8] = 0x0C;
}

void Widget::init_commond0D_data()
{
    commond_send[4] = 0x0D;
    commond_send[5] = 0x0D;
    commond_send[6] = 0x0D;
    commond_send[7] = 0x02;
    commond_send[8] = 0x02;
}

void Widget::init_commond0E_data()
{
    commond_send[4] = 0x0E;
    commond_send[5] = 0x0E;
    commond_send[6] = 0x0E;
    commond_send[7] = 0x04;
    commond_send[8] = 0x04;
}

//-------------------------------------------------------------------------------------------------
void Widget::set_ineEdit(QByteArray &commond)
{
    ui->lineEdit->clear();
    ui->lineEdit->insert("["+ QDateTime::currentDateTime().toString("hh:mm:ss:zzz")+"] ");
    for(int i = 0 ; i < commond.size();i++){
        unsigned  char   tmp  = commond.at(i);
        QString str = QString::number(tmp&0xff,16);
        if(tmp<=15)
            str .insert(0,"0");
        ui->lineEdit->insert(str+" ");
    }
}

void Widget::set_receive_textEdit(QByteArray &commond)
{
    ui->recevie_textEdit->insertPlainText("["+ QDateTime::currentDateTime().toString("hh:mm:ss:zzz")+"] ");
    for(int i = 0 ; i < 32;i++){
        unsigned  char   tmp  = commond.at(i);
        QString str = QString::number(tmp&0xff,16);
        if(tmp<=15)
            str .insert(0,"0");
        ui->recevie_textEdit->insertPlainText(str+" ");
    }
    ui->recevie_textEdit->insertPlainText("\n");
}

void Widget::send_commond()
{
    commond_send[3]  = commond_send[3]+0x01;
    commond_send[31] = 0x00;
    for(int n = 9;n < commond_send.size()-1;n++){
        if(commond_send.at(n) !=0x00)
            commond_send[31] = commond_send[31]+commond_send[n];
    }
    commond_send[31] = commond_send[31]&0xff;
    set_ineEdit(commond_send);
    qDebug()<<serial.write(commond_send);

    //####################################
    for(int j = 0;j<2;j++){
        commond_send[3]  = commond_send[3]+0x01;
        set_ineEdit(commond_send);
        serial.waitForBytesWritten(20);
        qDebug()<<serial.write(commond_send);
    }
}

//-----------------------------commond send-----------------------------------------------------------------
void Widget::on_ID01send_pushButton_clicked()
{
    if(!seriaport_open){
        // commond_send[3]  = commond_send[3]+0x01;
        int num_target  =  ui->targetID_comboBox->currentIndex();
        int num_instruct = ui->instruct01_comboBox->currentIndex();

        switch (num_target) {
        case 0:
            commond_send[9] = 0x01;
            commond_send[10] = 0x01;
            break;
        case 1:
            commond_send[9] = 0x02;
            commond_send[10] = 0x02;
            break;
        case 2:
            commond_send[9] = 0x04;
            commond_send[10] = 0x04;
            break;
        case 3:
            commond_send[9] = 0x05;
            commond_send[10] = 0x05;
            break;
        case 4:
            commond_send[9] = 0x06;
            commond_send[10] = 0x06;
            break;
        case 5:
            commond_send[9] = 0x07;
            commond_send[10] = 0x07;
            break;
        case 6:
            commond_send[9] = 0x08;
            commond_send[10] = 0x08;
            break;
        case 7:
            commond_send[9] = 0x09;
            commond_send[10] = 0x0;
            break;
        case 8:
            commond_send[9] = 0x0A;
            commond_send[10] = 0x0A;
            break;
        case 9:
            commond_send[9] = 0x0B;
            commond_send[10] = 0x0B;
            break;
        case 10:
            commond_send[9] = 0x0C;
            commond_send[10] = 0x0C;
            break;
        case 11:
            commond_send[9] = 0x0D;
            commond_send[10] = 0x0D;
            break;
        case 12:
            commond_send[9] = 0x0E;
            commond_send[10] = 0x0E;
            break;
        case 13:
            commond_send[9] = 0x0F;
            commond_send[10] = 0x0F;
            break;
        case 14:
            commond_send[9] = 0xFF;
            commond_send[10] = 0xFF;
            break;
        default:
            break;
        }
        switch (num_instruct) {
        case 0:
            commond_send[11] = 0xA5;
            commond_send[12] = 0xA5;
            break;
        case 1:
            commond_send[11] = 0xB6;
            commond_send[12] = 0xB6;
            break;
        default:
            break;
        }
        //ui->lineEdit->setText(commond_send.toHex());
        send_commond();
        /*
        commond_send[3]  = commond_send[3]+0x01;
        commond_send[31] = 0x00;
        for(int n = 9;n < commond_send.size()-1;n++){
            if(commond_send.at(n) !=0x00)
                commond_send[31] = commond_send[31]+0x01;
        }
        set_ineEdit(commond_send);
        qDebug()<<serial.write(commond_send);

        //####################################
        for(int j = 0;j<2;j++){
            commond_send[3]  = commond_send[3]+0x01;
            set_ineEdit(commond_send);
            serial.waitForBytesWritten(20);
            qDebug()<<serial.write(commond_send);
        }
        //        serial.waitForBytesWritten(20);
        //        qDebug()<<serial.write(commond_send);
        //        serial.waitForBytesWritten(20);
        //        qDebug()<<serial.write(commond_send);
    */
    }
}

void Widget::on_ID02send_pushButton_clicked()
{
    if(!seriaport_open){
        send_commond();
    }
}

void Widget::on_ID04send_pushButton_clicked()
{
    if(!seriaport_open){
        int num_instruct = ui->instruct04_comboBox->currentIndex();
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
        send_commond();
    }
}

void Widget::on_ID05send_pushButton_clicked()
{
    if(!seriaport_open){
        int num_instruct = ui->instruct05_comboBox->currentIndex();
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
        send_commond();
    }
}

void Widget::on_ID06send_pushButton_clicked()
{
    if(!seriaport_open){
        int num_instruct = ui->instruct06_comboBox->currentIndex();
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
        send_commond();
    }
}

void Widget::on_ID07send_pushButton_clicked()
{
    if(!seriaport_open){
        int num_instruct = ui->instruct07_comboBox->currentIndex();
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
        send_commond();
    }
}

void Widget::on_ID08send_pushButton_clicked()
{
    if(!seriaport_open){
        int num_instruct = ui->instruct08_comboBox->currentIndex();
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
        send_commond();
    }
}

void Widget::on_ID09send_pushButton_clicked()
{
    float  P1 = ui->P1_lineEdit->text().toFloat();
    float  P2 = ui->P2_lineEdit->text().toFloat();
    float  P3 = ui->P3_lineEdit->text().toFloat();

    if(!seriaport_open){
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

            send_commond();
        }
    }
}

void Widget::on_ID0Bsend_pushButton_clicked()
{
    QByteArray   motor_1 ;
    QByteArray   motor_2 ;
    QByteArray   motor_3 ;
    QByteArray   motor_4 ;

    int motor_1_int;
    int motor_2_int;
    int motor_3_int;
    int motor_4_int;

    motor_1.append(ui->motor1_lineEdit->text());
    motor_2.append(ui->motor2_lineEdit->text());
    motor_3.append(ui->motor3_lineEdit->text());
    motor_4.append(ui->motor4_lineEdit->text());

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
    if(!seriaport_open){
        send_commond();
    }

}

void Widget::on_ID0Csend_pushButton_clicked()
{
    QByteArray   longitude ;
    QByteArray   latitude ;
    QByteArray   hovertime;

    int  longitude_int;
    int  latitude_int;
    int  hovertime_int;

    longitude.append(ui->longitude_lineEdit->text());
    latitude.append(ui->latitude_lineEdit->text());
    hovertime.append(ui->hover_time_lineEdit->text());

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
        latitude_int =  longitude.toInt();
        commond_send[15] = latitude_int&0xff;
        commond_send[16] = latitude_int>>8&0xff;
        commond_send[17] = latitude_int>>16&0xff;
        commond_send[18] = latitude_int>>24|0x80;

    }else{
        latitude_int =  longitude.toInt();
        commond_send[15] = latitude_int&0xff;
        commond_send[16] = latitude_int>>8&0xff;
        commond_send[17] = latitude_int>>16&0xff;
        commond_send[18] = latitude_int>>24&0xff;
    }


    hovertime_int =  longitude.toInt();
    commond_send[19] = hovertime_int&0xff;
    commond_send[20] = hovertime_int>>8&0xff;
    commond_send[21] = hovertime_int>>16&0xff;

    commond_send[9] = 0x01;
    commond_send[10] = 0x01;


    if(!seriaport_open){
        if(longitude_int<-180||longitude_int >180||-90> latitude_int ||latitude_int>90||hovertime <0)
            QMessageBox::warning(this,tr("警告"),tr("经度/纬度/选停时间输入值/范围有误!"));
        else
            send_commond();
    }

}

void Widget::on_ID0Dsend_pushButton_clicked()
{
    if(!seriaport_open){
        int num_instruct = ui->instruct0D_comboBox->currentIndex();
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
        send_commond();
    }

}

void Widget::on_ID0Esend_pushButton_clicked()
{
    if(!seriaport_open){
        int num_instruct = ui->instruct0E_comboBox->currentIndex();
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
        send_commond();
    }
}
