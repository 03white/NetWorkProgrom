#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/select.h>

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
    //一直使用select检测各个套接字的读/写缓冲区
    fd_set readfds;//读缓冲区集合
    int maxFd=fd;
    FD_ZERO(&readfds);
    FD_SET(fd,&readfds);
    //使用select循环反复检测读缓冲区
    while(1){

      fd_set tmp=readfds;
      int ret=select(maxFd+1,&tmp,NULL,NULL,NULL);
      if(ret<0){
        perror("select error");
        break;
      }
      for(size_t i=0;i<maxFd+1;i++){//检查返回的结果集
           if(FD_ISSET(i,&tmp)){
             if(i==fd){//如果是监听socket缓冲区
               //读取通信socket的读缓冲区，拿到客户端socket文件描述符
               int cfd=accept(i,NULL,NULL);
               FD_SET(cfd,&readfds);//写入待检测列表
               maxFd=maxFd>cfd?maxFd:cfd;
             }else{//如果是通信socket
              
              char buffer[1024];
              int len=recv(i,buffer,sizeof(buffer),0);//接受缓冲区数据，即读客户端发来的数据
              if(len>0){
                 printf("客户端说：%s\n",buffer);
                 send(i,buffer,sizeof(buffer),0);
              }else if(len==0){
                printf("客户端断开连接......\n");
                FD_CLR(i,&readfds);
                close(i);
                break;
              }else{
               perror("recv");
               break;
              }
             }        
           
            }
          }
      }
    close(fd);
    return 0;
}