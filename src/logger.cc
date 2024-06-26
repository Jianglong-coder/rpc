#include "logger.h"
#include <time.h>
#include <thread>
#include <iostream>
Logger &Logger::GetInstance() // 获取日志的单例
{
    static Logger ins;
    return ins;
}
Logger::Logger()
{
    std::thread writeLogTask([this]()
                             {
        for (;;)
        {
            //获取当前的日期 然后去日志文件 写入响应的日志文件中 a+
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);

            char file_name[128];
            sprintf(file_name, "%d-%d-%d-log.txt", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday);

            FILE *pf = fopen(file_name, "a+");
            if (pf == nullptr)
            {
                std::cout << "logger file :" << file_name << "oper error!" << std::endl;
                exit(EXIT_FAILURE); 
            }

            std::string msg = m_lckQue.Pop();

            char time_buf[128] = {0};
            sprintf(time_buf, "%d:%d:%d =>[%s]", 
            nowtm->tm_hour, 
            nowtm->tm_min, 
            nowtm->tm_sec,
            (m_loglevel == INFO ? "info" : "error"));
            msg.insert(0, time_buf);
            msg.append("\n");
            fputs(msg.c_str(), pf);
            fclose(pf);
        } });
    writeLogTask.detach();
}
void Logger::SetLogLevel(LogLevel level) // 设置日志级别
{
    m_loglevel = level;
}
void Logger::Log(std::string msg) // 写日志  把日志信息写入lockqueue缓冲区当中  此函数是给外部调用的
{
    m_lckQue.Push(msg);
}
