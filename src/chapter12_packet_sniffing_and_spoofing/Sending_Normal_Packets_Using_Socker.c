#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


int main(void){
    char send_data[] = "UDP DATA\n";
    int sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);

    struct sockaddr_in dest;
    bzero(&dest,sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(9090);
    inet_pton(AF_INET,"192.168.1.108",&dest.sin_addr);
    // dest.sin_addr.s_addr = inet_addr("192.168.1.108");

    int data_size = sendto(sock,send_data,sizeof(send_data),0,(struct sockaddr *)&dest,sizeof(dest));
    printf("传输字符个数：%d\n",data_size);

    close(sock);
    return 0;
}