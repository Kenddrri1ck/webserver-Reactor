#include <iostream>
#include <unistd.h>
#include "TcpServer.h"
int main(int argc,char * argv[]) {
//    if(argc<3){
//        printf("./a.out port path\n");  //可执行程序的名字，端口，路径
//        return -1;
//    }
//    unsigned short port = atoi(argv[1]);
    unsigned short port = 10000;
    chdir("/home");
    //切换服务器的工作路径
    //chdir(argv[2]);
    //启动服务器
    TcpServer * server = new TcpServer(port,4);

    server->run();
}
