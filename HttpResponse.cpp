//
// Created by 34885 on 2023/7/9.
//
#include "HttpResponse.h"
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include "TcpConnection.h"
#define ResponseHeaderSize 16
HttpResponse::HttpResponse(){
    this->headers.clear();
    this->statusCode = HttpStatusCode::Unknown;
    this->fileName = string();
    this->sendDataFunc = nullptr;
}

HttpResponse::~HttpResponse(){

}

void HttpResponse::addHeader(string key,string value){
    if(key.empty()||value.empty()){
        return;
    }
    this->headers.insert(make_pair(key,value));
}

void HttpResponse::prepareMsg( Buffer * sendBuf, int socket){
    //状态行
    char tmp[1024] = {0};
    sprintf(tmp,"HTTP/1.1 %d %s\r\n",this->statusCode,this->m_info[(int)this->statusCode].data());
    sendBuf->appendString(tmp);
    //响应头

    for(auto it = this->headers.begin();it!=this->headers.end();it++){
        sprintf(tmp,"%s: %s\r\n",it->first.data(),it->second.data());
        sendBuf->appendString(tmp);
    }
    //空行
    sendBuf->appendString("\r\n");
#ifndef MSG_SEND_AUTO
    sendBuf->sendData(socket);
#endif
    //回复的数据
    this->sendDataFunc(this->fileName,sendBuf,socket);
}

