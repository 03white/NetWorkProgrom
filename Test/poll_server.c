#include<stdio.h>//处理io的头文件
#include<sys/socket.h>//包含套接字api的头文件
#include<arpa/inet.h>
#include<poll.h>//包含poll函数的头文件


int main(){
   int fd=socket(AF_INET,SOCK_STREAM,0);
   if(fd==-1){
     perror("socket");
     return -1;
   }
   struct sockaddr_in addr;
   addr.sin_family=AF_INET;
   addr.sin_port=htons(9999);
   addr.sin_addr.s_addr=INADDR_ANY;
   int ret=bind(fd,(struct sockaddr*)&addr,sizeof(addr));
   if(ret==-1){
     perror("bind");
     return -1;
   }
   ret=listen(fd,128);
   if(ret==-1){
    perror("listen");
    return -1;
   }
   
   struct pollfd fds[1024];//文件描述符集合
   for(int i=0;i<1024;i++){
      fds[i].fd=-1;//-1表示内核检测不感兴趣
      fds[i].events=POLLIN;//检测读缓冲区
   }
   fds[0].fd=fd;//通信文件描述符，其中度缓冲区是新的socket连接
   nfds_t maxPos=0;//最大的检测下标
   //循环调用poll委托内核去检测所有文件描述符的读缓冲区
   while(1){

    int ret=poll(fds,maxPos+1,-1);
    if(ret<0){
      perror("poll");
      break;
    }
    //判断是否有新的连接
    if(fds[0].revents&POLLIN){
        int cfd=accept(fds[0].fd,NULL,NULL);
        for(int i=1;i<1024;i++){
          if(fds[i].fd==-1){
            fds[i].fd=cfd;//添加到待检测文件描述符集合中
            fds[i].events = POLLIN;   // 显式设置
            maxPos=maxPos>i?maxPos:i;
            break;
          }
        }
    }
    
    //查看通信套接字文件描述符的读缓冲区结果
    for(int i=1;i<maxPos+1;i++){

         if(fds[i].fd!=-1){

           if(fds[i].revents&POLLIN){

            char buffer[1024];
            int res=recv(fds[i].fd,buffer,sizeof(buffer),0);
            if(res>0){
              printf("客户端说:%s\n",buffer);
              send(fds[i].fd,buffer,sizeof(buffer),0);
            }else if(res==0){
              printf("客户端断开连接");
              close(fds[i].fd);
              fds[i].fd=-1;
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