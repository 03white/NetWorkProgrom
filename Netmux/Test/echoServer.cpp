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