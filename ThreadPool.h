//
// Created by 34885 on 2023/7/8.
//

#ifndef REACTORHTTP_THREADPOOL_H
#define REACTORHTTP_THREADPOOL_H
#include "EventLoop.h"
#include "WorkerThread.h"
#include <vector>
class ThreadPool{
public:
    //线程池初始化
    ThreadPool(EventLoop * mainLoop,int count);
    ~ThreadPool();
    //启动线程池
    void run();
    //取出线程池中某个子线程的反应堆实例
    EventLoop * takeWorkerEventLoop();
private:
    EventLoop * mainLoop;
    bool isStart;
    int threadNum;
    std::vector<WorkerThread*> workerThreads;
    int index;
};

#endif //REACTORHTTP_THREADPOOL_H
