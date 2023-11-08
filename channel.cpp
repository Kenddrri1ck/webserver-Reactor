//
// Created by 34885 on 2023/7/6.
//
#include "channel.h"
#include <malloc.h>
 Channel::Channel(int fd,FDEvent events, handleFunc readCallBack, handleFunc writeCallBack,handleFunc DestoryCallBack, void * arg){

     this->fd = fd;
     this->events = static_cast<int>(events);
     this->readCallBack = readCallBack;
     this->writeCallBack = writeCallBack;
     this->DestoryCallBack = DestoryCallBack;
     this->arg = arg;

}

void Channel::writeEventEnable(bool flag){
    if(flag){
        this->events |= static_cast<int>(FDEvent::WriteEvent);  //0x04二进制为100 按位或可以把events里面的第三个标志位置为1
    }else{
        this->events  = static_cast<int>(FDEvent::WriteEvent);    //对writeEvent 取反 0000100 编程1111011 再按位与 把这一位置0
    }
}

bool Channel::isWriteEventEnable(){
    return this->events & static_cast<int>(FDEvent::WriteEvent);
}

int Channel::getFd(){
    return this->fd;
}
int Channel::getEvent(){
    return this->events;
}
void * Channel::getArg(){
    return this->arg;
}
