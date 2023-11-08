//
// Created by 34885 on 2023/7/8.
//

#ifndef REACTORHTTP_TCPCONNECTION_H
#define REACTORHTTP_TCPCONNECTION_H
#include "channel.h"
#include "Buffer.h"
#include "EventLoop.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#define MSG_SEND_AUTO
class TcpConnection{
public:
    TcpConnection(int fd,EventLoop * evLoop);
    ~TcpConnection();
    static int processRead(void * arg);
    static int processWrite(void * arg);
    static int  TcpConnectionDestory(void * arg);
    EventLoop * getEventLoop(){
        return this->evLoop;
    }
private:
    EventLoop * evLoop;
    Channel * channel;
    Buffer * readBuf;
    Buffer * writeBuf;
    string name;
    HttpRequest * request;
    HttpResponse * response;
};


#endif //REACTORHTTP_TCPCONNECTION_H
