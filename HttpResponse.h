//
// Created by 34885 on 2023/7/9.
//

#ifndef REACTORHTTP_HTTPRESPONSE_H
#define REACTORHTTP_HTTPRESPONSE_H
#include "Buffer.h"
#include <string>
#include <map>
#include <functional>
using namespace std;
//定义状态码枚举类
enum class HttpStatusCode{
    Unknown,
    OK = 200,
    MovePermanently = 301,
    MovedTemporarily = 302,
    BadRequest = 400,
    NotFound = 404
};
//定义响应的结构体 用数组，不需要存储空间



//定义结构体
class HttpResponse{
public:
    //初始化HttpResponse
    HttpResponse();
    ~HttpResponse();
//添加响应头
    void addHeader(string key,string value);
//组织响应的http数据
    void prepareMsg(Buffer * sendBuf, int socket);
    function<void(const string , Buffer * ,int )> sendDataFunc;
    void setStatusCode(HttpStatusCode code){
        this->statusCode = code;
    }
    void setFileName(string fileName){
        this->fileName = fileName;
    }
private:
    //状态行：状态码，状态描述，http协议的版本
    HttpStatusCode statusCode;

    //响应头-键值对
    map<string,string> headers;
    //文件名
    string fileName;
    //定义状态码和描述的对应关系
    map<int,string> m_info = {
            {200,"OK"},
            {301,"MovePermanently"},
            {302,"MovedTemporarily"},
            {400,"BadRequest"},
            {400,"NotFound"}
    };
};


#endif //REACTORHTTP_HTTPRESPONSE_H
