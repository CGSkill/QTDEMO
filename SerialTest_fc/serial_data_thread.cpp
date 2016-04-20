#include "serial_data_thread.h"
void serialdatathread::stop()
{
    stopped  =  true;
}

void serialdatathread::run()
{
//   while(1){
//       qDebug()<< serialdata_queue.size();
//       Qthread:sleep(1);
//   }

    /*
    while (!stopped) {

        if(serialdata_queue.isEmpty()){
            QThread::sleep(10);
             qDebug()<<"+++++++++++++++++++++++++++++++++++++++++++++++++";
        }
        //        if(judge){
        //            data_receive1 = serialdata_queue.dequeue();
        //            data_receive2 = serialdata_queue.dequeue();
        //        }
       qDebug()<<"------------------------------------------------------------------------------------";
        while(1){
             qDebug()<<"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@";
            if(serialdata_queue.isEmpty())
                break;
              qDebug()<<"11111111111111111111111111111111111111111111111111111111111111111111111111111111";
            if(judge){
                lock.lockForWrite();
                data_receive1 = serialdata_queue.dequeue();
                qDebug()<<data_receive1.time;
                qDebug()<<data_receive1.data;
                data_receive2 = serialdata_queue.dequeue();
                lock.unlock();
            }

            if(data_receive1.data[0] ==  0xeb && data_receive2.data[0] == 0x90)
            {
                bool  tmp   = true;
                time  =   data_receive1.time;
                handle_ok_serialdata.append(data_receive1.data);
                handle_ok_serialdata.append(data_receive2.data);
                for(int i = 2;i < 32; i++){
                    lock.lockForWrite();
                    data_receive1 =  serialdata_queue.dequeue();
                    lock.unlock();
                    handle_ok_serialdata.append(data_receive1.data);
                    if(handle_ok_serialdata[i] == 0xeb ){
                        lock.lockForWrite();
                        data_receive2 = serialdata_queue.dequeue();
                        lock.unlock();
                        handle_ok_serialdata.remove(i,1);
                        emit  send_data(handle_ok_serialdata,time,false);
                        handle_ok_serialdata.clear();
                        judge = false;
                        tmp   = false;
                        break;
                    }
                }
                if(tmp){
                    emit  send_data(handle_ok_serialdata,time,true);
                    handle_ok_serialdata.clear();
                    judge = true;
                }
            }else {
                time  =   data_receive1.time;
                handle_ok_serialdata.append(data_receive1.data);
                handle_ok_serialdata.append(data_receive2.data);
                int  i = 2;
                while(1){
                    i++;
                    lock.lockForWrite();
                    data_receive1 = serialdata_queue.dequeue();
                    lock.unlock();
                    handle_ok_serialdata.append(data_receive1.data);
                    if(handle_ok_serialdata[i] == 0xeb){
                        lock.lockForWrite();
                        data_receive2 = serialdata_queue.dequeue();
                        lock.unlock();
                        break;
                    }
                }
                handle_ok_serialdata.remove(i,1);
                emit  send_data(handle_ok_serialdata,time,false);
                handle_ok_serialdata.clear();
                judge = false;
            }
        }
    }
   // stopped  = false;
    QThread::sleep(1);
    */
}


