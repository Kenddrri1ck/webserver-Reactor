//
// Created by 34885 on 2023/7/9.
//

#ifndef REACTORHTTP_HTTPREQUEST_H
#define REACTORHTTP_HTTPREQUEST_H
#include "Buffer.h"
#include "HttpResponse.h"
#include <string>
#include <map>
#include <functional>
using namespace std;


//当前的解析状态
enum class HttpProcessState:char{
    ParseReqLine,  //解析请求行
    ParseReqHeaders, //解析请求头
    ParseReqBody,   //解析请求体
    ParseReqDone    //解析完成
};

//定义http请求结构体
class HttpRequest{
public:
    //初始化
    HttpRequest();
    ~HttpRequest();
//重置
    void reset();


//获取处理状态
    enum HttpProcessState getState();

//添加请求头
    void addHeader( const string key, const string value);

//根据key得到请求头value
    string getHeader( const string key);

//解析请求行
    bool parseHttpRequestLine( Buffer * readBuf);

//解析请求头
    bool parseHttpRequestHeader( Buffer * readBuf);

//解析http请求协议
    bool parseHttpRequest( Buffer * readBuf,  HttpResponse * response,  Buffer * sendBuf, int socket);

//处理http请求
    bool processHttpRequest(HttpResponse * response);

    string decodeMsg(string from);

    const string getFileType(const string name);

    static void sendDir(const string dirName, Buffer * sendBuf,int cfd);

    static void sendFile(const string fileName,Buffer * sendBuf,int cfd);

    char * splitRequestLine(const char * start,const char * end,const char * sub, function<void(string)>callback);

    inline void setMethod(const string method){
        this->method = method;
    }

    inline void setUrl(const string url){
        this->url = url;
    }

    inline void setVersion(const string version){
        this->version = version;
    }
    inline void setState(HttpProcessState state){
        this->curState = state;
    }
    int hexToDec(char c);
private:
    string  method;
    string url;
    string version;
    map<string,string> requestHeaders;
    enum HttpProcessState curState;
};


#endif //REACTORHTTP_HTTPREQUEST_H
