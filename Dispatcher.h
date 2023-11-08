//
// Created by 34885 on 2023/7/6.
//

#ifndef REACTORHTTP_DISPATCHER_H
#define REACTORHTTP_DISPATCHER_H
#include "channel.h"
#include "EventLoop.h"
#include <string>
using namespace std;
class EventLoop;
class Dispatcher{
public:
    Dispatcher(EventLoop * evLoop):evLoop(evLoop){};
    virtual ~Dispatcher(){};
    //添加
    virtual int add(){return 0;};
    //删除
    virtual int remove(){return 0;};
    //修改
    virtual int modify(){return 0;};
    //事件监测
    virtual int dispatch(int timeout = 2){return 0;}; //单位：s
    inline void setChannel(Channel * channel){
        this->channel = channel;
    }
protected:
    std::string name = string();
    Channel * channel;
    EventLoop * evLoop;
};
#endif //REACTORHTTP_DISPATCHER_H
