//
// Created by 34885 on 2023/7/8.
//
#include "WorkerThread.h"
#include <stdio.h>
WorkerThread::WorkerThread(int index){
    this->evLoop = nullptr;
    this->threadID = std::thread::id();
    this->name = "SubThread-"+ std::to_string(index);
    this->m_thread = nullptr;
}
//子线程的回调函数
void WorkerThread::subThreadRunning(){

    this->m_mutex.lock();
    this->evLoop = new EventLoop(this->name);
    this->m_mutex.unlock();
    this->m_cond.notify_one();
    this->evLoop->run();
}

//启动workerthread的函数 相当于主线程
void WorkerThread::run(){
    //创建子线程
    this->m_thread =  new std::thread(&WorkerThread::subThreadRunning,this);

    //阻塞主线程，让当前函数不会直接结束
    std::unique_lock<std::mutex> locker(this->m_mutex);
;
    while(this->evLoop== nullptr){
        this->m_cond.wait(locker); //利用条件变量阻塞
    }
}

WorkerThread::~WorkerThread() {
    if(this->m_thread!= nullptr){
        delete this->m_thread;
    }
}
