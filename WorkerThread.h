//
// Created by 34885 on 2023/7/8.
//

#ifndef REACTORHTTP_WORKERTHREAD_H
#define REACTORHTTP_WORKERTHREAD_H
#include <pthread.h>
#include "EventLoop.h"
#include <thread>
#include <string>
#include <mutex>
#include <condition_variable>
//定义子线程对应结构体
class WorkerThread{
public:
    inline EventLoop * getEventLoop(){
        return this->evLoop;
    }
    WorkerThread(int index);
    ~WorkerThread();
    //启动线程
    void run();

private:
    std::thread::id threadID;
    std::string name;
    std::mutex m_mutex;
    std::condition_variable m_cond;
    EventLoop * evLoop;
    std::thread * m_thread;

    void subThreadRunning();
};


#endif //REACTORHTTP_WORKERTHREAD_H
