//
// Created by 34885 on 2023/7/6.
//

#include "EpollDispatcher.h"
#include <arpa/inet.h>  //相比于socket.h这个头文件里声明了更多的套接字函数

#include <sys/epoll.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "Log.h"



static int EpollCtl( Channel * channel,  EventLoop * evLoop, int op);


EPollDispatcher::EPollDispatcher(EventLoop *evLoop): Dispatcher(evLoop){
    this->epfd = epoll_create(10); //指定大于0即可
    if(this->epfd==-1){
        perror("epoll_create");
        exit(0);
    }
    this->events  = new epoll_event[this->maxNode];
    this->name = "Epoll";
}

EPollDispatcher::~EPollDispatcher() {
    close(this->epfd);
    delete [] this->events;
}
//添加
int EPollDispatcher::add() {

    int ret = EpollCtl(EPOLL_CTL_ADD);
    if(ret==-1){
        perror("epoll_ctl");
        exit(0);
    }
    return ret;
}
//删除
int EPollDispatcher::remove(){

//    return ret;
    int ret = EpollCtl(EPOLL_CTL_DEL);
    if(ret==-1){
        perror("epoll_ctl");
        exit(0);
    }
    channel->DestoryCallBack(channel->getArg());
    return ret;
}
//修改
int EPollDispatcher::modify(){
    int ret = EpollCtl(EPOLL_CTL_MOD);
    if(ret==-1){
        perror("epoll_ctl");
        exit(0);
    }
    return ret;
}
//事件监测
int EPollDispatcher::dispatch( int timeout){
    int count = epoll_wait(this->epfd,this->events,this->maxNode,timeout*1000); //自己定义的timeout是秒
    for(int i = 0;i<count;i++){
        int events = this->events[i].events;
        int fd = this->events[i].data.fd;
        if(events&EPOLLERR||events&EPOLLHUP){
            //对方断开连接，删除fd
            //EpollRemove(channel,evLoop);
            continue;
        }
        if(events&EPOLLIN){
            this->evLoop->Active(fd,static_cast<int>(FDEvent::ReadEvent));
        }
        if(events&EPOLLOUT){
            this->evLoop->Active(fd,static_cast<int>(FDEvent::WriteEvent));
        }
    }
    return 0;
}


int EPollDispatcher::EpollCtl( int op){

    struct epoll_event  ev;
    int events = 0;
    if(channel->getEvent()&static_cast<int>(FDEvent::ReadEvent)){  //都是按照标识符来操作
        events|=EPOLLIN;
    }
    if(channel->getEvent()&static_cast<int>(FDEvent::ReadEvent)){
        events|=EPOLLOUT;
    }
    ev.events = events;
    ev.data.fd = channel->getFd();
    int ret = epoll_ctl(this->epfd,op,channel->getFd(),&ev);
    return ret;
}
