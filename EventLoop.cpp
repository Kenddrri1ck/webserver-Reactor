//
// Created by 34885 on 2023/7/6.
//
#include "EventLoop.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include "EpollDispatcher.h"
#include <string>
#include <functional>
#include <iostream>
using namespace std;
 EventLoop::EventLoop():EventLoop(string()) {

}
//写数据
void EventLoop::taskWakeUp(){
    const char * msg = "我是要成为海贼王的男人！！！";
    write(this->socketPair[0],msg, strlen(msg));
}
//读数据
int EventLoop::readLocalMessage(void * arg){
    EventLoop * evLoop = static_cast<EventLoop *>(arg);
    char buf[256];
    read(evLoop->socketPair[1],buf, sizeof(buf));
    return 0;
}

int EventLoop::readMessage() {
    char buf[256];
    read(this->socketPair[1],buf, sizeof(buf));
    return 0;
}

EventLoop::EventLoop( std::string threadName){

    this->isQuit = true;
    this->threadID = std::this_thread::get_id();
    this->threadName  = threadName==std::string()?"MainThread":threadName;
    //如果是epoll
    this->dispatcher = new EPollDispatcher(this);
    //evLoop->dispatcher = &PollDispatcher;
    //evLoop->dispatcher = &SelectDispatcher;
    this->channelMap.clear();
    //socketPair
    int ret = socketpair(AF_UNIX,SOCK_STREAM,0,this->socketPair);
    if(ret==-1){
        perror("socketpair");
        exit(0);
    }
    //指定evLoop->socketPair[0]发送数据，socketPair[1]接收数据
#if 0
    Channel *  channel = new Channel(this->socketPair[1],FDEvent::ReadEvent,readLocalMessage, nullptr, nullptr,this);
    //channel添加到任务队列
#else
    //绑定-bind cpp11可调用对象绑定器
    auto obj = std::bind(&EventLoop::readMessage,this);
    Channel *  channel = new Channel(this->socketPair[1],FDEvent::ReadEvent,obj, nullptr, nullptr,this);
#endif
    addTask(channel,ElemType::ADD);
}

EventLoop::~EventLoop() {}


int EventLoop::run(){

    this->isQuit = false;

    //比较线程id是否正常
    if(this->threadID!=std::this_thread::get_id()){
        return -1;
    }
    //循环进行事件处理
    while(!this->isQuit){
        //超时时长2s  cpp里面就是多态  调用函数指针指向的不同的函数
        // 会调用 epollDispatch pollDispatch 和 selectDispatch中的一种去接收通信中的消息
        this->dispatcher->dispatch(); //dispatch选择类型
        this->processTask(); //处理任务
    }
    return 0;
}

int EventLoop::Active( int fd, int event){
    if(fd<0){
        return -1;
    }
    //取出channel
    Channel *  channel = this->channelMap[fd];
    assert(channel->getFd()==fd);
    if(event&static_cast<int>(FDEvent::ReadEvent) && channel->readCallBack){
        channel->readCallBack(channel->getArg());
    }
    if(event&static_cast<int>(FDEvent::WriteEvent) && channel->writeCallBack){
        channel->writeCallBack(channel->getArg());
    }
    return 0;
}

int EventLoop::addTask( Channel * channel, ElemType type){
    //加锁，保护共享资源
    this->m_mutex.lock();
    //创建新结点
    ChannelElement * node = new ChannelElement;
    node->channel = channel;
    node->type = type;
    this->task_Q.push(node);

    this->m_mutex.unlock();

    //处理结点
    /*
     *细节
     * 1.对于链表结点的添加，可能是当前线程也可能是其他线程（主线程）
     *      1). 修改fd的事件，当前子线程发起，当前子线程处理
     *      2). 添加新的fd，添加任务结点的操作由主线程发起
     * 2.不能让主线程处理任务队列，需要由当前的子线程处理
     */
    if(this->threadID==std::this_thread::get_id()){
        //当前子线程
       this->processTask();
    }else{
        //主线程--告诉子线程处理任务队列中的任务
        //1.子线程正在工作 2.子线程被底层的dispatcher阻塞了：select,poll,epoll
        //如何解除阻塞：往select,poll,epoll中添加一个额外的文件描述符，即可解除阻塞
        this->taskWakeUp();
    }
    return 0;
}

int EventLoop::processTask(){
    //取出链表头结点
    while(!this->task_Q.empty()){
        this->m_mutex.lock();
        ChannelElement * node = this->task_Q.front();
        this->task_Q.pop();
        this->m_mutex.unlock();
        Channel * channel = node->channel;
        if(node->type==ElemType::ADD){
            //添加
            add(channel);
        }else if(node->type == ElemType::DELETE){
            //删除
            remove(channel);
        }else if(node->type == ElemType::MODIFY){
            //修改
            modify(channel);
        }
        delete node;
    }

    return 0;
}

int EventLoop::add(Channel * channel){  //此处的添加指将当前的任务添加到dispatcher的检测集合中去
    int fd = channel->getFd();

    //找到fd对应的数组元素位置，并存储
    if(this->channelMap.find(fd)== this->channelMap.end()){
        this->channelMap.insert(std::make_pair(fd,channel));
        this->dispatcher->setChannel(channel);
        int ret = this->dispatcher->add();  //因为在构造dispatcher时，已经把evLoop传递进去了，所以不需要传递参数
        return ret;
    }
    return -1;
}
int EventLoop::remove(Channel * channel){
    int fd = channel->getFd();
    if(this->channelMap.find(fd)== this->channelMap.end()){
        return -1;
    }
    this->dispatcher->setChannel(channel);
    int ret = this->dispatcher->remove();
    return ret;
}
int EventLoop::modify(Channel * channel){
    int fd = channel->getFd();
    if(this->channelMap.find(fd)== this->channelMap.end()){  //如果为空，则有问题
        return -1;
    }
    this->dispatcher->setChannel(channel);
    int ret = this->dispatcher->modify();
    return ret;
}

int EventLoop::freeChannel(Channel * channel){
    //删除channel 和 fd 的对应关系
    auto it = this->channelMap.find(channel->getFd());
    if(it!=this->channelMap.end()){
        this->channelMap.erase(it);
        //关闭fd
        close(channel->getFd());
        //释放channel
        delete channel;
    }
    return 0;
}