//
// Created by 34885 on 2023/7/6.
//

#ifndef REACTORHTTP_CHANNEL_H
#define REACTORHTTP_CHANNEL_H
//定义函数指针
#include <stdbool.h>
#include <functional>
//typedef int(*handleFunc)(void * arg);

//using handleFunc = int(*)(void *);

//定义文件描述符的读写事件的枚举类
enum class FDEvent{  //强类型枚举
    TimeOut = 0x01,
    ReadEvent = 0x02,
    WriteEvent = 0x04
};

class Channel{
public:
    //使用可调用对象包装器对函数进行包装
    //可调用对象包装器打包的是什么？
    //1.函数指针
    //2.可调用对象（可以像函数一样使用）
    //最终得到地址
    using handleFunc = std::function<int(void*)>;
    //回调函数
    handleFunc readCallBack;
    handleFunc writeCallBack;
    handleFunc DestoryCallBack;
    //初始化一个Channel
    Channel(int fd,FDEvent events, handleFunc readCallBack, handleFunc writeCallBack, handleFunc DestoryCallBack,void * arg);
    //修改fd的写事件(检测 or 不检测)  flag为true 说明需要检测 channel的写事件， flag为false不需要检测channel的写事件
    void writeEventEnable( bool flag);
    //判断是否需要检测文件描述符的写事件
    bool isWriteEventEnable();
    //getter
    int getFd();
    int getEvent();
    void * getArg();
private:
    //文件描述符
    int fd;
    //事件
    int events;
    //回调函数的参数
    void * arg;
};


#endif //REACTORHTTP_CHANNEL_H
