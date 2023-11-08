cmake_minimum_required(VERSION 3.22)
project(ReactorHttp_cpp)

set(CMAKE_CXX_STANDARD 14)

add_executable(ReactorHttp main.cpp channel.cpp channel.h Dispatcher.h EventLoop.cpp EventLoop.h EpollDispatcher.cpp PollDispatcher.cpp SelectDispatcher.cpp WorkerThread.h WorkerThread.cpp ThreadPool.cpp ThreadPool.h Buffer.h Buffer.cpp TcpServer.h TcpServer.cpp TcpConnection.h TcpConnection.cpp HttpRequest.cpp HttpRequest.h HttpResponse.cpp HttpResponse.h Log.h EpollDispatcher.h PollDispatcher.h SelectDispatcher.h)

