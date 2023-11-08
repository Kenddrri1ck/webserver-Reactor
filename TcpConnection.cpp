//
// Created by 34885 on 2023/7/8.
//

#include "TcpConnection.h"
#include <stdlib.h>
#include <stdio.h>
#include "HttpRequest.h"
#include "Log.h"
int TcpConnection::processRead(void * arg){
    TcpConnection * conn = (TcpConnection *)arg;
    //接收数据
    int count = conn->readBuf->socketRead(conn->channel->getFd());
    Debug("接收到的Http请求：%s",conn->readBuf->getData()+conn->readBuf->getReadPos());
    if(count>0){
        //接收到了http请求，解析http请求
#ifdef MSG_SEND_AUTO  //.h文件中的MSG_SEND_AUTO被注释，则在这里的代码块相当于被注释，则未给channel添加写事件，则processWrite不被调用
        conn->channel->writeEventEnable(true);
        conn->evLoop->addTask(conn->channel,ElemType::MODIFY);
#endif
        Debug("1111");
        bool flag = conn->request->parseHttpRequest(conn->readBuf,conn->response,conn->writeBuf,conn->channel->getFd());
        Debug("readbuf: %s",conn->readBuf->getData());
        Debug("writebuf: %s",conn->writeBuf->getData());
        if(!flag){
            //解析失败 回复一个简单的HTML
            char * errMsg = "Http/1.1 400 Bad Request\r\n\r\n";
            conn->writeBuf->appendString(errMsg);
        }
    }else{
#ifdef MSG_SEND_AUTO
        // 断开连接
        conn->evLoop->addTask(conn->channel,ElemType::DELETE);
#endif
    }
    //全部写入到writeBuf再发送的方法，则这里不能断开连接，否则无法发送数据
#ifndef MSG_SEND_AUTO
    conn->evLoop->addTask(conn->channel,ElemType::DELETE);
#endif
    return 0;
}
int TcpConnection::processWrite(void * arg){
    Debug("开始发送数据了（基于写事件发送）...");
    TcpConnection * conn = (TcpConnection *)arg;
    //发送数据的函数
    int count = conn->writeBuf->sendData(conn->channel->getFd());
    if(count>0){
        //判断数据是否完全被发出去
        if(conn->writeBuf->readableSize()==0){
            //数据发送完毕
            //1.不再检测写事件 --修改channel中保存的事件
            conn->channel->writeEventEnable(false);
            //2.修改dispatcher检测的集合 -- 添加任务结点
            conn->evLoop->addTask(conn->channel,ElemType::MODIFY);
            //3.删除这个结点
            conn->evLoop->addTask(conn->channel,ElemType::DELETE);
        }
    }
    return 0;
}

int  TcpConnection::TcpConnectionDestory(void * arg){
    TcpConnection * conn = (TcpConnection*)arg;
    if(conn!= nullptr){
        delete conn;
    }
    return 0;
}

TcpConnection::TcpConnection(int fd, EventLoop * evLoop){

    this->evLoop = evLoop;
    this->readBuf = new Buffer(10240);
    this->writeBuf = new Buffer(10240);
    this->name = "Connection-"+ to_string(fd);

    this->channel =new  Channel(fd,FDEvent::ReadEvent,processRead,processWrite,TcpConnectionDestory,this);
    this->evLoop->addTask(this->channel,ElemType::ADD);
    this->request = new HttpRequest();
    this->response = new HttpResponse();
    Debug("和客户端建立连接， threadID: %ld, connName: %s ",evLoop->getThreadID(),  this->name.data());
}

TcpConnection::~TcpConnection() {
    if(this->readBuf&& this->readBuf->readableSize()==0
       &&this->writeBuf&& this->writeBuf->writeableSize()==0){
        this->evLoop->freeChannel(this->channel);
        delete this->readBuf;
        delete this->writeBuf;
        delete this->request;
        delete this->response;
//            bufferDestory(conn->readBuf);
//            bufferDestory(conn->writeBuf);
//            httpRequestDestory(conn->request);
//            httpResponseDestory(conn->response);
    }
    Debug("连接断开，释放资源 connName: %s",this->name.data());
}

