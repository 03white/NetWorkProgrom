#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<arpa/inet.h>

int main(){

    //创建通信的套接字
    int fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd==-1){
       perror("socket");
       return -1;
    }
    //绑定
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;//地址族协议
    addr.sin_port=htons(9999);//服务器端口
    inet_pton(AF_INET,"127.0.0.1",&addr.sin_addr.s_addr);
    int ret=connect(fd,(struct sockaddr*)&addr,sizeof(addr));//连接
    if(ret==-1){
        perror("connect");
        return -1;
    }
    int number=0;
    while(1){
        char buffer[1024];
        sprintf(buffer,"hello world:%d",number++);
        send(fd,buffer,strlen(buffer)+1,0);//向缓冲区写入数据，即像服务器发送数据
        memset(buffer,0,sizeof(buffer));//清空缓冲区
        int len=recv(fd,buffer,sizeof(buffer),0);
        if(len>0){
            printf("服务器说:%s\n",buffer);
        }else if(len==0){
            printf("服务器断开连接......");
            break;
        }else{
            perror("recv");
            break;
        }
        sleep(1);
    }
    close(fd);
    return 0;
}



