//
// Created by 34885 on 2023/7/8.
//

#ifndef REACTORHTTP_BUFFER_H
#define REACTORHTTP_BUFFER_H
#include <string>
class Buffer{
public:
    //初始化buffer
    Buffer(int size);

    //销毁buffer
    ~Buffer();

    //buffer扩容
    void extendRoom( int size);

    //获取剩余可写的内存容量
    int writeableSize();

    //获取剩余可读的内存容量
    int readableSize();

    //写内存：1.直接写 2.套接字接收数据
    int appendString(const char * data, int size);
    int appendString( const char * data);
    int socketRead( int fd);

    //根据\r\n取出一行  找到其在数据块中的位置 返回该位置
    char * findCRLF();

    //发送数据
    int sendData( int fd);

    char * getData(){return this->data;}

    int getReadPos(){return this->readPos;}

    char * getBegin(){
        return this->data+this->readPos;
    }

    inline int readPosIncrease(int count){
        this->readPos+=count;
        return this->readPos;
    }
private:
    //指向内存的指针
    char * data;
    int capacity;
    int readPos;
    int writePos;
};


#endif //REACTORHTTP_BUFFER_H
