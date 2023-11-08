//
// Created by 34885 on 2023/7/9.
//
#include "HttpRequest.h"

#include <stdlib.h>
#define HeaderSize 12
#include <strings.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/sendfile.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "TcpConnection.h"
#include "Log.h"
HttpRequest::HttpRequest(){
     this->reset();
}

void HttpRequest::reset(){
    this->curState = HttpProcessState::ParseReqLine;
    this->method = string();
    this->url = string();
    this->version = string();
    this->requestHeaders.clear();
}

HttpRequest::~HttpRequest() {

}


enum HttpProcessState HttpRequest::getState(){
    return this->curState;
}

void HttpRequest::addHeader( const string key, const string value){
    this->requestHeaders.insert(make_pair(key,value));
}

//根据key得到请求头value
string HttpRequest::getHeader( const string  key){

    auto item = this->requestHeaders.find(key);
    if(item!=this->requestHeaders.end()) {
        return item->second;
    }

    return string();
}

char * HttpRequest::splitRequestLine(const char * start,const char * end,const char * sub, function<void(string)>callback){
    char * space = (char *) end;
    if(sub!=NULL){
        space = (char*)memmem(start,end-start,sub, strlen(sub));
        assert(space!=NULL);
    }
    int length = space-start;
    //给指针分配空间，所以要传一个二级指针，即一级指针的地址 如果传一级指针，则会发生拷贝
    callback(string(start,length));
    return space+1;
}

bool HttpRequest::parseHttpRequestLine(Buffer * readBuf){
    //读出请求行 字符串结束地址
    char * end = readBuf->findCRLF();
    //保存字符串起始地址
    char * start = readBuf->getBegin();
    //请求行总长度
    int lineSize = end-start;
    auto methodFunc = bind(&HttpRequest::setMethod,this,placeholders::_1);
    auto urlFunc = bind(&HttpRequest::setUrl,this,placeholders::_1);
    auto versionFunc = bind(&HttpRequest::setVersion,this,placeholders::_1);
    if(lineSize){
        start = splitRequestLine(start,end," ",methodFunc);
        start = splitRequestLine(start,end," ",urlFunc);
        splitRequestLine(start,end, nullptr,versionFunc);
#if 0
        //get /xxx/xx.txt http/1.1
        //请求方式


        //请求的静态资源
        start = space+1;
        space = (char*)memmem(start,end-start," ",1);
        assert(space!=NULL);
        int urlSize = space-start;
        request->url = (char * ) malloc(sizeof (char)* urlSize+1);
        strncpy(request->url,start, urlSize);
        request->url[methodSize] = '\0';

        //请求的http协议的版本
        start = space+1;
        //space = (char*)memmem(start,end-start," ",1);
        //assert(space!=NULL);
        //int versionSize = space-start;
        request->url = (char * ) malloc(sizeof (char)* (end-start)+1);
        strncpy(request->url,start, end-start);
        request->url[end-start] = '\0';
#endif
        //为解析请求头做准备
        readBuf->readPosIncrease(lineSize+2);
        //修改状态
        this->setState(HttpProcessState::ParseReqHeaders);
        return true;
    }

    return false;
}
//该函数每次处理请求头中的一行，如果要处理多行调用多次即可
bool HttpRequest::parseHttpRequestHeader(Buffer * readBuf){
    char * end = readBuf->findCRLF();
    if(end!=NULL){
        char * start = readBuf->getBegin();
        int lineSize = end-start;
        //基于：搜索字符串
        char * middle = (char *)memmem(start,lineSize,": ",2);
        if(middle!=NULL){
            if(middle-start>0&&end-middle-2>0) {
                string key = string(start,middle-start);
                string value = string(middle+2,end-middle-2);
                this->addHeader(key,value);
            }
            //这一行请求结束，跳到下一行
            readBuf->readPosIncrease(lineSize+2);
        } else{
            //请求头请求结束，readBuffer跳过空行
            readBuf->readPosIncrease(2);
            //如果是post则空行之后的请求体里面有请求的数据和静态资源url
            //修改解析状态  项目中只处理get请求 所以直接修改为requestDone
            this->setState(HttpProcessState::ParseReqDone);
        }
        return true;
    }
    return false;
}

bool HttpRequest::parseHttpRequest(Buffer * readBuf,HttpResponse * response, Buffer * sendBuf, int socket){
    bool flag = true;
    while(this->curState!=HttpProcessState::ParseReqDone){
        switch (this->curState) {
            case HttpProcessState::ParseReqLine:
                flag = this->parseHttpRequestLine(readBuf);
                break;
            case HttpProcessState::ParseReqHeaders:
                flag = this->parseHttpRequestHeader(readBuf);
                break;
            case HttpProcessState::ParseReqBody: //默认为GET post先不处理了
                break;
            default:
                break;
        }
        if(!flag){
            return flag;
        }
        //判断是否解析完毕，若解析完毕，则回复数据
        if(this->curState==HttpProcessState::ParseReqDone){
            //1.根据解析出的原始数据，对客户端的请求进行处理
            this->processHttpRequest(response);
            //2.组织响应数据块并发送客户端
            response->prepareMsg(sendBuf,socket);
        }
    }
    //第一条请求解析完毕，准备解析第二条请求
    this->curState = HttpProcessState::ParseReqLine;
    return flag;
}



//处理基于get的http请求
bool HttpRequest::processHttpRequest(HttpResponse * response){

    if(strcasecmp(this->method.data(),"get")!=0){  //strcasecmp不区分大小写
        return false;
    }
    this->url = decodeMsg(this->url);
    //处理客户端请求的资源(目录或者文件)
    const char * file = NULL;
    if(strcmp(this->url.data(),"/")==0){  //如果申请的是根目录 那么转换为./ 否则提取文件名
        file = "./";
    }else{
        file = this->url.data()+1;  //file是指针
    }
    //获取文件属性
    struct stat st;
    int ret = stat(file,&st);

    if(ret==-1){
        //文件不存在 --回复404
        const char * type = "Not Found";
        //sendHeadMsg(cfd,404, type, getFileType(".html"), -1); //长度指定为-1 告诉浏览器我不知道长度，让浏览器自己去读
        //sendFile("404.html",cfd);
        response->setFileName("404.html");
        response->setStatusCode(HttpStatusCode::NotFound);
        //响应头
        response->addHeader("Content-type", getFileType(".html"));
        response->sendDataFunc = sendFile;
        return 0;
    }
    //判断文件类型
    response->setFileName(file);
    response->setStatusCode(HttpStatusCode::OK);

    if(S_ISDIR(st.st_mode)){  //判断是不是目录，是目录返回1，不是目录返回0
        //把这个目录中的内容发送给客户端
        //响应头
        response->addHeader("Content-type", getFileType(".html"));
        response->sendDataFunc = sendDir;
    }
    else{
        //把文件的内容发送给客户端
        //响应头
        response->addHeader("Content-type", getFileType(file));
        response->addHeader("Content-length", to_string(st.st_size));
        response->sendDataFunc = sendFile;
    }


    return false;
}


// 将字符转换为整形数
int HttpRequest::hexToDec(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0;
}

// 解码
// to 存储解码之后的数据, 传出参数, from被解码的数据, 传入参数
string HttpRequest::decodeMsg(string msg)
{
    const char * from = msg.data();
    string str = string();
    for (; *from != '\0'; ++from)
    {
        // isxdigit -> 判断字符是不是16进制格式, 取值在 0-f
        // Linux%E5%86%85%E6%A0%B8.jpg
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
        {
            // 将16进制的数 -> 十进制 将这个数值赋值给了字符 int -> char
            // B2 == 178
            // 将3个字符, 变成了一个字符, 这个字符就是原始数据
            str.append(1,this->hexToDec(from[1]) * 16 + this->hexToDec(from[2]));
            // 跳过 from[1] 和 from[2] 因此在当前循环中已经处理过了
            from += 2;
        }
        else
        {
            // 字符拷贝, 赋值
            str.append(1,*from);
        }

    }
    str.append(1,'\0');
    return str;
}

const string HttpRequest::getFileType(const string name)
{
    // a.jpg a.mp4 a.html
    // 自右向左查找‘.’字符, 如不存在返回NULL  因为文件的扩展名是在右边 比如.jpeg
    const char* dot = strchr(name.data(),'.');
    if (dot == NULL)
        return "text/plain; charset=utf-8";	// 纯文本
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp(dot, ".wav") == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";
    return "text/plain; charset=utf-8";
}

void HttpRequest::sendFile(const string fileName, Buffer * sendBuf,int cfd){  //读一部分，发送一部分。因为是tcp流式协议，所以可以这么做
    int fd = open(fileName.data(),O_RDONLY);

    assert(fd>0);
#if 1
    while(1){
        char buf[1024];
        int len = read(fd,buf,sizeof(buf));
        if(len>0){
            //send(cfd,buf,len,0);
            sendBuf->appendString(buf, len);
#ifndef MSG_SEND_AUTO
            sendBuf->sendData(cfd);
#endif
            usleep(10); //确保发送端不会发送得太快  这非常重要
        }else if(len==0){
            break;
        } else{
            close(fd);
            perror("read");
        }
    }
#else
    off_t offset = 0;
    int size = lseek(fd,0,SEEK_END); //SEEK_END指把文件指针移动到末尾之后再加上 第二个参数的0
    lseek(fd,0,SEEK_SET);
    while(offset<size){
        //想知道偏移量，直接读offset的值即可
        int ret = sendfile(cfd,fd,&offset,size-offset); //每次发送的大小应该是总的-已经发送出去的偏移量
        printf("ret value: %d\n", ret);
        if (ret == -1 && errno == EAGAIN)
        {
            printf("没数据...\n");
        }
    }

#endif
    close(fd);
}

void HttpRequest::sendDir(const string dirName, Buffer * sendBuf,int cfd){
    char buf[4096] = {0};
    sprintf(buf,"<html><head><title>%s</title></head><body><table>",dirName.data());
    struct dirent ** nameList;
    int num = scandir(dirName.data(),&nameList,NULL,alphasort);
    for(int i = 0;i<num;i++){
        //取出文件名 nameList指向指针数组  struct dirent * list[]
        char * name = nameList[i]->d_name;
        struct stat st;
        char subPath[1024] = {0};
        sprintf(subPath,"%s/%s",dirName.data(),name);
        stat(subPath,&st);
        if(S_ISDIR(st.st_mode)){ //如果是目录 按目录方式处理
            //a标签 超链接标签 <a href = "">name</a>
            sprintf(buf+ strlen(buf),"<tr><td><a href = \"%s/\">%s</a></td><td>%ld</td></tr>",name,name, st.st_size);
        } else{  //如果不是目录，按文件的方式处理
            sprintf(buf+ strlen(buf),"<tr><td><a href = \"%s\">%s</a></td><td>%ld</td></tr>",name,name, st.st_size);
        }
        sendBuf->appendString(buf);
#ifndef MSG_SEND_AUTO
        Debug("开始发送数据");
        sendBuf->sendData(cfd);
#endif
        //send(cfd, buf, strlen(buf), 0);
        memset(buf,0, sizeof(buf));
        //释放掉 防止内存泄漏
        free(nameList[i]);
    }
    sprintf(buf+ strlen(buf),"</table></body></html>");
    //send(cfd, buf, strlen(buf), 0);
    sendBuf->appendString(buf);
#ifndef MSG_SEND_AUTO
    sendBuf->sendData(cfd);
#endif
    free(nameList);
}

