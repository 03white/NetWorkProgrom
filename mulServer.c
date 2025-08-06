#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<pthread.h>

/*
主线程监听，子线程处理通信
*/
struct SockInfo{
    int cfd;
    struct sockaddr_in caddr;
};
void*working(void*arg);
struct SockInfo infos[512];
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
    for(int i=0;i<512;i++){
        bzero(&infos[i],sizeof(infos[i]));
        infos[i].cfd=-1;
    }
    //主线程一直接受连接，并产生新的socket
    int caddrlen=sizeof(struct sockaddr_in);
    while(1){
       struct SockInfo*pinfo;
       for(int i=0;i<512;i++){
          if(infos[i].cfd==-1){
            pinfo=&infos[i];
            break;
           }
        }
       int cfd=accept(fd,(struct sockaddr*)&(pinfo->caddr),&caddrlen);
       if(cfd==-1){
        perror("accept");
        break;
        }
       pinfo->cfd=cfd;
       pthread_t tid;
       pthread_create(&tid,NULL,working,pinfo);
       pthread_detach(tid);
    }
    close(fd);
    return 0;
}
void*working(void*arg){
     struct SockInfo*pinfo=(struct SockInfo*)arg;
     char ip[50];
     //建立连接成功，打印客户端ip和端口信息
     printf("客户端ip: %s,端口:%d\n",inet_ntop(AF_INET,&pinfo->caddr.sin_addr.s_addr,ip,sizeof(ip)),ntohs(pinfo->caddr.sin_port));//端口大端转小端
     while(1){
         
        char buffer[1024];
        int len=recv(pinfo->cfd,buffer,sizeof(buffer),0);//接受缓冲区数据，即读客户端发来的数据
        if(len>0){
          printf("客户端：%s\n",buffer);
          send(pinfo->cfd,buffer,sizeof(buffer),0);
        }else if(len==0){
          printf("客户端断开连接......\n");
          break;
        }else{
            perror("recv");
            break;
        }
    }
    close(pinfo->cfd);
    pinfo->cfd=-1;
}