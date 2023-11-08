//
// Created by 34885 on 2023/7/12.
//

#ifndef REACTORHTTP_CPP_EPOLLDISPATCHER_H
#define REACTORHTTP_CPP_EPOLLDISPATCHER_H


#include "Dispatcher.h"
#include <string>
#include <sys/epoll.h>
class EPollDispatcher:public Dispatcher{
public:
    EPollDispatcher(EventLoop * evLoop);
    ~EPollDispatcher();
    //添加
    int add() override;
    //删除
    int remove() override;
    //修改
    int modify() override;
    //事件监测
    int dispatch(int timeout = 2); //单位：s

private:
    int EpollCtl(int op);
    int epfd;
    struct epoll_event * events;
    const int maxNode = 520;
};


#endif //REACTORHTTP_CPP_EPOLLDISPATCHER_H
