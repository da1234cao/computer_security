#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// given an IP Packet, send it out using a raw socket.
void sned_raw_ip_packet(struct ip *ippacket){

    // 创建一个原生套接字
    // IPPROTO_RAW表明我们将支持ip头部，系统不需要提供IP头部
    // IPPROTO_RAW同时表明允许IP_HDRINCL
    int sock = socket(AF_INET,SOCK_RAW,IPPROTO_RAW);

    // int enable = 1;
    // setsockopt(sock,IPPROTO_IP,IP_HDRINCL,&enable,sizeof(enable));

    // 提供目的地选项
    // sockaddr_in提供的结构体，用来构建IP头；现在IP头已经有了，应当不用提供这样的结构体
    // 书上说：提供目的地址，帮助内核获得目的IP对应的MAC地址
    struct sockaddr_in dest_info;
    dest_info.sin_family = AF_INET;
    dest_info.sin_addr = ippacket->ip_dst;

    // 发送包
    sendto(sock,ippacket,ntohs(ippacket->ip_len),0,(struct sockaddr *)(&dest_info),sizeof(dest_info));
    
    close(sock);
}