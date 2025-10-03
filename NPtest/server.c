#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>


int main(){

    //创建监听的套接字
    int fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd==-1){
       perror("socket");
       return -1;
    }
    //绑定
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(9999);
    addr.sin_addr.s_addr=INADDR_ANY;
    int ret=bind(fd,(struct sockaddr*)&addr,sizeof(addr));
    if(ret==-1){
        perror("bind");
        return -1;
    }
    //设置监听
    ret=listen(fd,128);
    if(ret==-1){
       perror("listen");
       return -1;
    }
    //接受
    struct sockaddr_in caddr;
    int caddrlen=sizeof(caddr);
    int cfd=accept(fd,(struct sockaddr*)&caddr,&caddrlen);//只能接受一个客户端连接的单线程程序
    if(cfd==-1){
        perror("accept");
        return -1;
    }
    char ip[50];
    //建立连接成功，打印客户端ip和端口信息
    printf("客户端ip: %s,端口:%d\n",inet_ntop(AF_INET,&caddr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(caddr.sin_port));//端口大端转小端
    while(1){
         
        char buffer[1024];
        int len=recv(cfd,buffer,sizeof(buffer),0);//接受缓冲区数据，即读客户端发来的数据
        if(len>0){
          printf("客户端：%s\n",buffer);
          send(cfd,buffer,sizeof(buffer),0);
        }else if(len==0){
          printf("客户端断开连接......\n");
          break;
        }else{
            perror("recv");
            break;
        }
    }
    close(fd);
    close(cfd);
    return 0;
}