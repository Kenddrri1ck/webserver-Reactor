//
// Created by 34885 on 2023/7/12.
//

#ifndef REACTORHTTP_CPP_POLLDISPATCHER_H
#define REACTORHTTP_CPP_POLLDISPATCHER_H
#include "Dispatcher.h"
#include <string>
#include <poll.h>
class PollDispatcher:public Dispatcher{
public:
    PollDispatcher(EventLoop * evLoop);
    ~PollDispatcher();
    //添加
    int add() override;
    //删除
    int remove() override;
    //修改
    int modify() override;
    //事件监测
    int dispatch(int timeout = 2); //单位：s

private:
    int maxfd;
    const int maxNode = 1024;
    struct pollfd *fds;
};
#endif //REACTORHTTP_CPP_POLLDISPATCHER_H
