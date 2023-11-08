//
// Created by 34885 on 2023/7/8.
//
#include "Buffer.h"
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <sys/uio.h>
#include <string.h>
#include <arpa/inet.h>
#include "Log.h"
Buffer::Buffer(int size){
    //因为后续要扩容，要使用relloac,所以这里使用malloc
    this->data = (char*)malloc(size);
    this->capacity = size;
    this->readPos = this->writePos = 0;
    bzero(this->data,this->capacity);
}

Buffer::~Buffer(){
    if(this->data!= nullptr){
        free(this->data);
    }

}

//获取剩余可写的内存容量
int Buffer::writeableSize(){
    return this->capacity-this->writePos;
}
//获取剩余可读的内存容量
int Buffer::readableSize(){
    return this->writePos-this->readPos;
}

void Buffer::extendRoom( int size){
    //1.内存够用-不需要扩容
    if(this->writeableSize()>=size){
        return;
    }
    //2.内存需要合并才够用-不需要扩容
    //剩余可写的内存加已读内存
    else if(this->readPos+ this->writeableSize()>=size){
        //得到未读的内存大小
        int readable = this->readableSize();
        //移动内存
        memcpy(this->data,this->data+this->readPos,readable);
        this->readPos = 0;
        this->writePos = readable;
    }
    //3.内存不够用-需要扩容
    else{
        char * temp = (char *)realloc(this->data,this->capacity+size);
        if(temp== nullptr){
            return; //失败了
        }
        memset(temp+this->capacity,0,size);
        //更新数据
        this->data = temp;
        this->capacity+=size;
    }
}

int Buffer::appendString( const char * data, int size){
    if(data== nullptr||size<=0){
        return -1;
    }
    //扩容
    this->extendRoom(size);
    //数据拷贝
    memcpy(this->data+this->writePos,data,size);
    this->writePos+=size;
    return 0;
}

int Buffer::appendString( const char * data){
    int size = strlen(data);
    int ret = this->appendString(data,size);
    return ret;
}

int Buffer::socketRead( int fd){
    //read recv readv
    struct iovec vec[2];
    //初始化数组元素
    int writeable = this->writeableSize();
    vec[0].iov_base = this->data+this->writePos;
    vec[0].iov_len = writeable;

    char * tmpbuf = (char * )malloc(sizeof(char)*40960);
    vec[1].iov_base = tmpbuf;
    vec[1].iov_len = 40960;

    int result = readv(fd,vec,2);
    if(result==-1){
        return -1;
    }else if(result<=writeable){  //buffer中内存够用，接收的数据都写到buffer里面了
        this->writePos+=result;
    } else{  //buffer中的内存不够用 要扩容
        this->writePos = this->capacity;
        this->appendString(tmpbuf,result-writeable); //扩容，result-writeable是要追加到buffer里面的长度
    }
    free(tmpbuf);
    return result;  //表示一共接收多少字节
}

char * Buffer::findCRLF(){
    //strstr -->大字符串中匹配子字符串（遇到\0结束）
    //memmem -->大数据块里匹配子数据块（数据块大小需要指定）
    //void *memmem(const void *haystack, size_t haystacklen,const void *needle, size_t needlelen);
    char * ptr = (char *)memmem(this->data+this->readPos, this->readableSize(),"\r\n",2);
    return ptr;
}

int Buffer::sendData( int fd){
    //判断有无数据
    int readable = this->readableSize();  //判断有多少未读数据，这些未读的数据就是待发送的数据
    if(readable>0){
        int count = send(fd,this->data+this->readPos,readable,MSG_NOSIGNAL);
        Debug("已发送%d个数据",count);
        Debug("发送数据为: %s",this->data+this->readPos);
        if(count){
            this->readPos+=count;
            usleep(1);
        }
        return count;
    }
    return 0;
}