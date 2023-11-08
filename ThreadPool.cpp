//
// Created by 34885 on 2023/7/8.
//
#include "ThreadPool.h"
#include <stdlib.h>
#include "WorkerThread.h"
#include <assert.h>
ThreadPool::ThreadPool( EventLoop * mainLoop,int count){
    this->index = 0;
    this->mainLoop = mainLoop;
    this->isStart = false;
    this->threadNum = count;
    this->workerThreads.clear();
}

void ThreadPool::run(){
    assert(!this->isStart);
    if(this->mainLoop->getThreadID()!=std::this_thread::get_id()){  //如果不是主线程id
        exit(0);
    }
    this->isStart = true;
    if(this->threadNum>0){
        for(int i = 0;i<this->threadNum;i++){
            WorkerThread * tmp = new WorkerThread(i);
            tmp->run();
            this->workerThreads.push_back(tmp);

        }
    }
}

EventLoop * ThreadPool::takeWorkerEventLoop(){
    assert(this->isStart);
    if(this->mainLoop->getThreadID()!=std::this_thread::get_id()){  //如果不是主线程id
        exit(0);
    }
    //从线程池中找出一个子线程，并取出里面的反应堆实例
    EventLoop * evLoop = this->mainLoop;
    if(this->threadNum>0){
        evLoop = this->workerThreads[this->index]->getEventLoop();
        this->index = (this->index+1)%this->threadNum;
    }
    return evLoop;
}


