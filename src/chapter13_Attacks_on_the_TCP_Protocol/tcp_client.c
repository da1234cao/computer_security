/**
 * 写一个最简单的tcp客户端程序，不考虑error处理
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(void){
    int sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    struct sockaddr_in client;
    bzero(&client,sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(9090);
    inet_pton(AF_INET,"127.0.0.1",&client.sin_addr);

    connect(sock,(struct sockaddr *)&client,sizeof(client));   

    char *buffer1 = "Tcp Message\n";
    char *buffer2 = "Tcp Mseeage Again\n";
    int size_buffer1 = write(sock,buffer1,strlen(buffer1));
    int size_buffer2 = write(sock,buffer2,strlen(buffer2));

    close(sock);
    return 0;
}