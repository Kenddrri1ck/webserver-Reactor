//
// Created by 34885 on 2023/7/6.
//
//当需要检测的文件描述符的数量不多且大多数都是激活的时候，用poll和select，其他时候用epoll
#include "Dispatcher.h"  //这种函数指针加静态函数的方法个人认为相当于多态
#include <arpa/inet.h>  //相比于socket.h这个头文件里声明了更多的套接字函数
#include "SelectDispatcher.h"
#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#define Max 1024




 SelectDispatcher::SelectDispatcher(EventLoop *evLoop):Dispatcher(evLoop){
    FD_ZERO(&this->readSet);
    FD_ZERO(&this->writeSet);
    this->name = "Select";
}

SelectDispatcher::~SelectDispatcher(){

 }

//添加
int SelectDispatcher::add(){

    setFdSet();
    return 0;
}
//删除
int SelectDispatcher::remove(){

    clearFdSet();
    channel->DestoryCallBack(channel->getArg());
    return 0;
}
//修改
int SelectDispatcher::modify(){
    setFdSet();
    clearFdSet();
    return 0;
}
//事件监测
int SelectDispatcher::dispatch(int timeout){

    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    fd_set rdtmp = this->readSet;
    fd_set wrtmp = this->writeSet;
    int count = select(this->maxNode,&rdtmp,&wrtmp,NULL,&tv); //自己定义的timeout是秒
    if(count==-1){
        perror("select");
        exit(0);
    }
    for(int i = 0;i<this->maxNode;i++){
        if(FD_ISSET(i,&rdtmp)){
            //调用对应的读回调函数
            this->evLoop->Active(i,static_cast<int>(FDEvent::ReadEvent));
        }
        if(FD_ISSET(i,&wrtmp)){
            //调用对应的写回调函数
            this->evLoop->Active(i,static_cast<int>(FDEvent::WriteEvent));
        }
    }
    return 0;
}


void SelectDispatcher::setFdSet(){
    if(this->channel->getEvent()&static_cast<int>(FDEvent::ReadEvent)){  //都是按照标识符来操作
        FD_SET(this->channel->getFd(),&this->readSet);
    }
    if(this->channel->getEvent()&static_cast<int>(FDEvent::WriteEvent)){
        FD_SET(this->channel->getFd(),&this->writeSet);
    }

}
void SelectDispatcher::clearFdSet(){
    if(this->channel->getEvent()&static_cast<int>(FDEvent::ReadEvent)){  //都是按照标识符来操作
        FD_CLR(this->channel->getFd(),&this->readSet);
    }
    if(this->channel->getEvent()&static_cast<int>(FDEvent::WriteEvent)){
        FD_CLR(this->channel->getFd(),&this->writeSet);
    }
}


