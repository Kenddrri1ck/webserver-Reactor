//
// Created by 34885 on 2023/7/8.
//

#ifndef REACTORHTTP_TCPSERVER_H
#define REACTORHTTP_TCPSERVER_H
#include "EventLoop.h"
#include "ThreadPool.h"

class TcpServer{
public:
    //初始化
    TcpServer(unsigned short port,int threadNum);
    //启动服务器
    void run();
    //初始化监听
    void setListen();
    int getLfd(){return this->lfd;}
    ThreadPool * getThreadPool(){return this->threadPool;}
    static int acceptConnection(void * arg);
private:
    int threadNum;
    EventLoop * mainLoop;
    ThreadPool * threadPool;
    int lfd;
    unsigned short port;
};


#endif //REACTORHTTP_TCPSERVER_H
