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


int main(int argc,char *argv[]){
    printf("-------WebServer--------\n");
    /*PF_INET--ipv4  SOCK_STREAM--tcp*/

    int port=atoi(argv[1]);
    int ret=0;


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

    while (1)  {  
        struct sockaddr_in client_address;
        socklen_t client_addrlength=sizeof(client_address);
        int connfd=accept(listenfd,(struct sockaddr*)&client_address,&client_addrlength);

        char receiveBuf[10];
        memset(receiveBuf,0,sizeof(receiveBuf));
        recv(connfd,receiveBuf,sizeof(receiveBuf),0);  
        printf("%s\n",receiveBuf);  
        close(connfd);//关闭  
 
    } 


    return 0;
}