#ifndef _VPN_H_

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <linux/if_tun.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/select.h>

#define PORT_NUMBER 55555
#define SERVER_IP "192.168.56.10"
struct sockaddr_in peerAddr;

#define BUFF_SIZE 1024

int createTunDevice(){
    int tunfd;
    struct ifreq ifr;
    bzero(&ifr,sizeof(ifr));

    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    tunfd = open("/dev/net/tun",O_RDWR);
    ioctl(tunfd,TUNSETIFF,&ifr);

    return tunfd;
}

int connectToServer(){
    int sockfd;
    char *hello = "Hello";
    struct sockaddr_in clientAddr;

    bzero(&clientAddr,sizeof(struct sockaddr_in));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(PORT_NUMBER);
    clientAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    sockfd = socket(AF_INET,SOCK_DGRAM,0);

    sendto(sockfd,hello,strlen(hello),0,(struct sockaddr *)&clientAddr,sizeof(struct sockaddr_in));

    printf("Connected with the server : %s\n",hello);
    return sockfd;
}

int initUDPServer(){
    int sockfd;
    struct sockaddr_in server;
    char buff[100];

    bzero(&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT_NUMBER);

    sockfd = socket(AF_INET,SOCK_DGRAM,0);
    bind(sockfd,(struct sockaddr *)&server,sizeof(server));

    bzero(buff,sizeof(buff));
    int peerAddrLen = sizeof(struct sockaddr_in);
    int len = recvfrom(sockfd,buff,sizeof(buff)-1,0,(struct sockaddr *)&peerAddr,&peerAddrLen);
    
    printf("Connected with the client : %s\n",buff);
    return sockfd;
}

void tunSelected(int tunfd,int sockfd){
    int len;
    char buff[BUFF_SIZE];

    printf("Got a packet from TUN\n");

    bzero(buff,BUFF_SIZE);
    len = read(tunfd,buff,BUFF_SIZE);

    sendto(sockfd,buff,len,0,(struct sockaddr *)&peerAddr,sizeof(peerAddr));
}

void socketSelected(int tunfd,int sockfd){
    int len;
    char buff[BUFF_SIZE];

    printf("Got a packet from the tunnel\n");

    bzero(buff,BUFF_SIZE);
    len = recvfrom(sockfd,buff,BUFF_SIZE,0,NULL,NULL);
    write(tunfd,buff,len);
}


#endif