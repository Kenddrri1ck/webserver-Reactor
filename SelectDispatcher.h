//
// Created by 34885 on 2023/7/12.
//

#ifndef REACTORHTTP_CPP_SELECTDISPATCHER_H
#define REACTORHTTP_CPP_SELECTDISPATCHER_H



#include "Dispatcher.h"
#include <string>
#include <sys/select.h>
class SelectDispatcher:public Dispatcher{
public:
    SelectDispatcher(EventLoop * evLoop);
    ~SelectDispatcher();
    //添加
    int add() override;
    //删除
    int remove() override;
    //修改
    int modify() override;
    //事件监测
    int dispatch(int timeout = 2); //单位：s

private:
    const int maxNode = 1024;
    fd_set readSet;
    fd_set writeSet;
    void setFdSet();
    void clearFdSet();
};


#endif //REACTORHTTP_CPP_SELECTDISPATCHER_H
