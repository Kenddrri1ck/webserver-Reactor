//
// Created by 34885 on 2023/7/6.
//

#ifndef REACTORHTTP_EVENTLOOP_H
#define REACTORHTTP_EVENTLOOP_H
#include "Dispatcher.h"
#include "channel.h"
#include <thread>
#include <queue>
#include <map>
#include <mutex>
#include <condition_variable>
#include <string>
//处理该节点中channel中的方式
using namespace std;
enum class ElemType:char{
    ADD,DELETE,MODIFY
};

//定义任务队列的结点
struct ChannelElement{
    ElemType type;  //如何处理该结点中的channel
    Channel * channel;
};
class Dispatcher;
class EventLoop{
public:
    EventLoop();
    EventLoop(std::string threadName);
    ~EventLoop();
    inline std::thread::id getThreadID(){return this->threadID;}
//启动
    int run();

//处理激活的文件描述符fd的函数
    int Active(int fd, int event);

//向任务队列中添加任务
    int addTask(Channel * channel, ElemType type);

//处理任务队列中的任务
    int processTask();

//处理dispatcher中的结点
    int add( Channel * channel);
    int remove( Channel * channel);
    int modify( Channel * channel);

//释放channel
    int freeChannel(Channel * channel);

    static int readLocalMessage(void * arg);

    int readMessage();
    void taskWakeUp();
private:
    bool isQuit;
    //父类指针指向子类的实例对象
    Dispatcher * dispatcher;
    queue<ChannelElement*> task_Q;
    //map
    map<int,Channel*> channelMap;
    //线程id,name,mutex
    thread::id threadID;
    string threadName;
    mutex  m_mutex;
    int socketPair[2]; //存储本地通信的fd 通过socketpair初始化
};


#endif //REACTORHTTP_EVENTLOOP_H
