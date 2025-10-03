#include<stdlib.h>
#include<stdio.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/epoll.h>
int main(){
    int lfd=socket(AF_INET,SOCK_STREAM,0);
    if(lfd==-1){
        perror("socket error");
        return;
    }
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(9999);
    addr.sin_addr.s_addr=INADDR_ANY;
    int ret=bind(lfd,(struct sockaddr*)&addr,sizeof(addr));
    if(ret==-1){
        perror("bind error");
        return;
    }
    if(listen(lfd,128)==-1){
        perror("listen error");
        return;
    }
    int epfd=epoll_create(1);//创建一个epoll实例
    if(epfd==-1){
        perror("epoll_create error");
        return;
    }
    struct epoll_event ev;
    ev.events=EPOLLIN;
    ev.data.fd=lfd;
    epoll_ctl(epfd,EPOLL_CTL_ADD,lfd,&ev);//将监听socket添加到epoll树结构中
    struct epoll_event events[1024];
    int size=sizeof(events)/sizeof(events[0]);
    while(1){
        int readyNum=epoll_wait(epfd,events,size,-1);//检测是否发生事件并将事件写入传出参数中
        for(int i=0;i<readyNum;i++){
            int fd=events[i].data.fd;
            if(fd==lfd){
                int newcfd=accept(lfd,NULL,NULL);
                    struct epoll_event ev;
                    ev.events=EPOLLIN;
                    ev.data.fd=newcfd;
                    epoll_ctl(epfd,EPOLL_CTL_ADD,newcfd,&ev);
                    continue;
           }
           char buffer[1024];
           int len=recv(fd,buffer,sizeof(buffer),0);
           if(len==-1){
                perror("recv error");
           }else if(len==0){
                printf("客户端断开连接!\n");
                epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
                close(fd);
           }else{
                printf("客户端:%s\n",buffer);
                send(fd,buffer,sizeof(buffer),0);
           }
           
        }
    }
    close(lfd);
    return 0;
}
