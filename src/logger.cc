#include"include/logger.h"
#include<iostream>
#include<time.h>
#include<unistd.h>
#include<fcntl.h>
using namespace std;

// 获取日志的单例
Logger &Logger::GetInstance()
{
    static Logger logger;
    return logger;
}

Logger::Logger()
{
    //启动专门写日志的一个线程
    thread writeLogTask([&](){
        for(;;)
        {
            //获取当天的日期，然后取日志信息，写入相应的日志文件当中
            time_t now=time(nullptr);
            tm*nowtm=localtime(&now);
            char file_name[128];
            sprintf(file_name,"%d-%d-%d-log.txt",nowtm->tm_year+1900,nowtm->tm_mon+1,nowtm->tm_mday);
            FILE* fp=fopen(file_name,"a+");
            if(fp==nullptr)
            {
                cout<<"logger file:"<<file_name<<"open error!"<<endl;
            }

            string msg=m_lckQue.Pop();
            //写入文件当中
            char time_buf[128] = {0};
            sprintf(time_buf, "%d:%d:%d =>[%s] ", 
                    nowtm->tm_hour, 
                    nowtm->tm_min, 
                    nowtm->tm_sec,
                    (m_loglevel == INFO ? "info" : "error"));
            msg.insert(0, time_buf);
            msg.append("\n");

           fputs(msg.c_str(),fp);
           fclose(fp);
        }
    });
    //设置分离线程  守护线程
    writeLogTask.detach();
}

// 设置日志级别
void Logger::SetLogLevel(LogLevel level)
{
    m_loglevel=level;
}
// 写日志  把日志信息写入lockqueue缓冲区当中
void Logger::Log(string msg)
{
    m_lckQue.Push(msg);
}