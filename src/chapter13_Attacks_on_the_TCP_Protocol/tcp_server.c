/**
 * 写一个最简单的tcp服务器程序，不考虑error处理
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define LISTENQ 5
#define READBUFFER 101

int main(void){
    int listenfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    struct sockaddr_in server;
    bzero(&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(9090);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listenfd,(struct sockaddr *)&server,sizeof(server));

    listen(listenfd,LISTENQ);

    struct sockaddr_in client;
    bzero(&client,sizeof(client));
    int client_len = sizeof(client);
    while(1){
        int connfd = accept(listenfd,(struct sockaddr *)&client,&client_len);

        if(fork() == 0){
            //子进程进行复制，导致listenfd计数为+1，所以里面得进行close
            close(listenfd);

            char buffer[READBUFFER];
            bzero(buffer,READBUFFER);
            int read_len = read(connfd,buffer,READBUFFER-1);

            printf("Receive %d bytes : %s",read_len,buffer);

            close(connfd);
            return 0; //我开始的时候，没有写这一句，整个系统直接卡住？
        }else{
            close(connfd);
        } 
    }
    
    close(listenfd);
    return 0;
}