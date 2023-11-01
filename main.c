#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>
#include <string.h>
#include <iostream>
using namespace std;

#define MAX_EVENT_NUMBER 10000	//最大事件数

void str_echo(epoll_event event,epoll_event ev){
    int  MAXLINE=10;
    char line[MAXLINE]; 
    int n = 0;
    int sockfd = 0;
    sockfd = event.data.fd;
    if (sockfd < 0){
        return ;
    }      
    if ( (n = read(sockfd, line, MAXLINE)) < 0) {
        if (errno == ECONNRESET) {
            close(sockfd);
            event.data.fd = -1;
        } else{
            std::cout<<"readline error"<<std::endl;
        }
    } else if (n == 0) {
        close(sockfd);
        event.data.fd = -1;
    }
    line[n] = '\n';
    cout << "read " << line << endl;
    ev.data.fd=sockfd;
    ev.events=EPOLLOUT|EPOLLET;
}


void addfd_(int epollfd,int fd,bool one_shot)
{
    epoll_event event;
    event.data.fd=fd;
    event.events=EPOLLIN|EPOLLRDHUP;
    if(one_shot)
        event.events|=EPOLLONESHOT;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
}


int main(int argc,char *argv[]){
    printf("-------WebServer--------\n");
    /*PF_INET--ipv4  SOCK_STREAM--tcp*/

    int port=atoi(argv[1]);
    int ret=0;
    int connfd=0;
    bool stop_server = false;
    struct epoll_event ev;
    int sockfd;

    int listenfd=socket(PF_INET,SOCK_STREAM,0);
    assert(listenfd>=0);

    struct sockaddr_in address;
    memset(&address,0,sizeof(address));
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=htonl(INADDR_ANY);
    address.sin_port=htons(port);

    
    // 设置端口复用，绑定端口
    int flag = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret=bind(listenfd,(struct sockaddr*)&address,sizeof(address));
    assert(ret>=0);
    ret=listen(listenfd,5); 
    assert(ret>=0);


    //创建内核事件
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd=epoll_create(5);
    assert(epollfd!=-1);
    addfd_(epollfd,listenfd,false);

    while (!stop_server){  
        int number=epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
        if(number<0&&errno!=EINTR){
	        printf("%s","epoll failure");
            break;
        }
        for(int i=0;i<number;i++){
            sockfd=events[i].data.fd;
            printf("sockfd=%d\n",sockfd);
            //处理客户端连接
            if(sockfd==listenfd){
                struct sockaddr_in client_address;
                socklen_t client_addrlength=sizeof(client_address);
                connfd=accept(listenfd,(struct sockaddr*)&client_address,&client_addrlength);

                ev.data.fd=connfd;
                ev.events=EPOLLIN|EPOLLET;
                epoll_ctl(epollfd,EPOLL_CTL_ADD,connfd,&ev);
                
            } else if(events[i].events&EPOLLIN){
                printf("%s\n","EPOLLIN");
                //处理客户端发送的消息
                str_echo(events[i],ev);
                ev.events=EPOLLOUT|EPOLLET;
                epoll_ctl(epollfd,EPOLL_CTL_MOD,sockfd,&ev);
           
            }else if(events[i].events&EPOLLOUT) {
                // 如果有数据发送
                sockfd = events[i].data.fd;
                write(sockfd, "test", 5);
                ev.data.fd=sockfd;
                ev.events=EPOLLIN|EPOLLET;
                epoll_ctl(epollfd,EPOLL_CTL_MOD,sockfd,&ev);
                

            }
        }
    } 
    return 0;
}