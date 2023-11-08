//
// Created by 34885 on 2023/7/6.
//
//当需要检测的文件描述符的数量不多且大多数都是激活的时候，用poll和select，其他时候用epoll
#include "Dispatcher.h"  //这种函数指针加静态函数的方法个人认为相当于多态
#include <arpa/inet.h>  //相比于socket.h这个头文件里声明了更多的套接字函数

#include <poll.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "PollDispatcher.h"

PollDispatcher::PollDispatcher(EventLoop *evLoop): Dispatcher(evLoop){

    this->maxfd = 0;
    this->name = "Poll";
    this->fds = new pollfd[this->maxNode];
    for(int i = 0;i<this->maxNode;i++){
        this->fds[i].fd = -1;
        this->fds[i].events = -1;
        this->fds[i].revents = -1;
    }
}

//添加
int PollDispatcher::add(){

    int events = 0;
    if(channel->getEvent()&static_cast<int>(FDEvent::ReadEvent)){  //都是按照标识符来操作
        events|=POLLIN;
    }
    if(channel->getEvent()&static_cast<int>(FDEvent::WriteEvent)){
        events|=POLLOUT;
    }
    int i = 0;
    for(;i<this->maxNode;i++){
        if(this->fds[i].fd==-1){
            this->fds[i].fd = channel->getEvent();
            this->fds[i].events = events;
            this->maxfd = i> this->maxfd?i:this->maxfd;
            break;
        }
    }
    if(i>=this->maxNode){
        return -1;
    }
    return 0;
}
//删除
int PollDispatcher::remove(){
    int i = 0;
    for(;i<this->maxNode;i++){
        if(this->fds[i].fd==channel->getFd()){
            this->fds[i].fd = -1;
            this->fds[i].events = 0;
            this->fds[i].revents = 0;
            break;
        }
    }
    //通过channel释放TcpConnection对应的资源  必须把外部实例传递给内部channel成员
    channel->DestoryCallBack(channel->getArg());

    if(i>=this->maxNode){
        return -1;
    }
    return 0;
}
//修改
int PollDispatcher::modify(){
    int events = 0;
    if(channel->getEvent()&static_cast<int>(FDEvent::ReadEvent)){  //都是按照标识符来操作
        events|=POLLIN;
    }
    if(channel->getEvent()&static_cast<int>(FDEvent::WriteEvent)){
        events|=POLLOUT;
    }
    int i = 0;
    for(;i<this->maxNode;i++){
        if(this->fds[i].fd==channel->getFd()){
            this->fds[i].events = events;
            break;
        }
    }
    if(i>=this->maxNode){
        return -1;
    }
    return 0;
}
//事件监测
int PollDispatcher::dispatch(int timeout){

    int count = poll(this->fds,this->maxfd+1,timeout*1000); //自己定义的timeout是秒
    if(count==-1){
        perror("poll");
        exit(0);
    }
    for(int i = 0;i<=this->maxfd;i++){
        if(this->fds[i].fd==-1){
            continue;
        }

        if(this->fds[i].revents&POLLIN){
           //读事件触发，调用读回调函数
            this->evLoop->Active(this->fds[i].fd,static_cast<int>(FDEvent::ReadEvent));
        }
        if(this->fds[i].revents&POLLOUT){
            //写事件触发，调用写回调函数
            this->evLoop->Active(this->fds[i].fd,static_cast<int>(FDEvent::WriteEvent));
        }
    }
    return 0;
}
//清除数据（关闭fd或释放内存）
PollDispatcher::~PollDispatcher(){
    delete [] this->fds;
}


