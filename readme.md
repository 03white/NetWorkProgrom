# NetWorkProgram
## 一.概述
本项目是关于c/c++网络编程的一些study项目，包括基本的socket到网络库，再到基于网络库的协议库等。
## 二.NPtest
NPtest里是一些简单的网络编程api使用例子，比如socket,select,poll,epoll等
## 三.Netmux
Netmux是一个经典的基于epoll的多反应堆reactor模型的网络库，类似muduo,目前仅支持linux下的epoll，不支持windows的iocp。
### 3.1 Netmux的实例
以一个简单的echoServer为例子。
```cpp
#include"../TcpServer.h"
#include<functional>
#include<iostream>
#include<string>
class EchoServer{
public:
    //constructor
    EchoServer(EventLoop*loop,const InetAddress& addr,const std::string &name):server_(loop,addr,name){
        server_.setThreadNum(3);
        server_.setConnectionCallback(std::bind(&EchoServer::onConnection,this,std::placeholders::_1));
        server_.setMessageCallback(std::bind(&EchoServer::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    }
    void onConnection(const TcpConnectionPtr &connPtr){
        std::cout<<connPtr->peerAddress().toIpPort()<<"has etablish connection !"<<std::endl;
    }
    void onMessage(const TcpConnectionPtr &connPtr,Buffer*buf,Timestamp time){
        // 读取所有可读数据
        std::string msg(buf->retrieveAllAsString());
    
        std::cout<< "EchoServer received " << msg.size() << " bytes from "
             << connPtr->peerAddress().toIpPort() << " at " << time.toString()<<std::endl;
    
        std::cout<< "Message content: " << msg; 
        // 将收到的消息原样发回
        connPtr->send(msg);
    }
    void start(){
        server_.start();
    }
private:
    TcpServer server_;
};
int main(){
    EventLoop loop;
    InetAddress addr(8888);
    EchoServer echoserver(&loop,addr,"echoserver");
    echoserver.start();
    loop.loop();
    return 0;
}
```
使用这个例子的方法如下:
```cpp
//克隆本项目
git clone git@github.com:03white/NetherLink-static.git / https://github.com/03white/NetherLink-static.git
```

```cpp
 //构建项目makefile，这里默认使用make作为构建工具
cmake -S . -B build
```

```cpp
//构建Test目标,运行这个命令后,你会在Netmux/Test/build/bin下找到Test运行文件，你可以运行它。
cmake --build build --target Test
```
```cpp
//除了构建Test目标外，我们还可以构建Netmux目标，在Netmux/build/lib下生成静态库
cmake --build build --target Netmux
```
