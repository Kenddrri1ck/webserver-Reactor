//
// Created by 34885 on 2023/7/8.
//
#include "TcpServer.h"
#include "stdlib.h"
#include <arpa/inet.h>
#include <stdio.h>
#include "TcpConnection.h"
#include "Log.h"
#include <iostream>
using namespace std;
TcpServer::TcpServer(unsigned short port,int threadNum){
    this->mainLoop = new EventLoop();  //初始化主线程 所以不用Ex 就叫mainLoop
    this->threadNum = threadNum;
    this->threadPool = new ThreadPool(this->mainLoop,this->threadNum);
    this->port = port;
    this->setListen();
}

void TcpServer::setListen(){

    //1.创建监听的fd
    this->lfd = socket(AF_INET,SOCK_STREAM,0);
    if (this->lfd==-1){
        perror("socket");
        return;
    }
    //2.设置端口复用
    int opt = 1;
    int ret = setsockopt(this->lfd,SOL_SOCKET,SO_REUSEADDR,&opt, sizeof(opt));
    if(ret==-1){
        perror("setsockopt");
        return;
    }
    //3.绑定
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(this->port);
    addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(this->lfd,(struct sockaddr*)&addr, sizeof(addr));
    if(ret == -1){
        perror("bind");
        return;
    }
    //4.设置监听
    ret = listen(this->lfd,128);
    if(ret==-1){
        perror("listen");
        return;
    }

}
int TcpServer::acceptConnection(void * arg){
    TcpServer * server = (TcpServer * )arg;
    //和客户端建立连接
    int cfd = accept(server->getLfd(),NULL,NULL);
    //从线程池取出一个子线程的反应堆实例，处理这个cfd
    EventLoop * evLoop = server->getThreadPool()->takeWorkerEventLoop();
    //将cfd放到TcpConnection中处理
    new TcpConnection(cfd,evLoop);
    return 0;
}
void TcpServer::run(){
    Debug("服务器程序启动了....");
    //启动线程池
    this->threadPool->run();
    //添加检测的任务  即把用于监听的文件描述符添加到主反应堆内，让主反应堆进行监听
    Channel * channel = new Channel(this->lfd,FDEvent::ReadEvent,acceptConnection,NULL,NULL,this);
    this->mainLoop->addTask(channel,ElemType::ADD);
    //启动反应堆
    this->mainLoop->run();
}

