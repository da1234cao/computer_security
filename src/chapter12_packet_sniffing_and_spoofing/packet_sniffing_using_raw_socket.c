/**
 * 目的：使用原生套接字+混杂模式，获取该局域网中所有包
 * 
 * 原生套接字：SOCK_ROW + ETH_P_ALL，所有协议的包都将传递给raw socket。
 * 
 * 混杂模式：设置socket可选参数PACKET_ADD_MEMBERSHIP选项，
 * 使用packet_mreq结构体指定一个**以太网接口**和混杂模式行为(PACKET_MR_PROMISC)。
 * 
 * 头文件：
 * sys/socket.h 定义了一些列的socket API函数 如 socket()，bind()，listen()，
 *              send()，sendmsg()，setsockopt()，等等。
 * netinet/in.h 定义了ip地址结构 struct sockaddr_in （struct in_addr作为sockaddr_in 的成员) 
 *                 还有函数 htons等，以及一些宏。
 * linux/if_packet.h 原始数据包的数据结构定义
 * net/ethernet.h 包括几个以太网的数据结构
 * 
 * errno.h 有一个 errno 宏，它就用来存储错误代码
 * unistd.h close函数      
 * arpa/inet.h     inet_ntop函数
*/

#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>

#include <unistd.h>
#include <arpa/inet.h>  

int main(void){
    
    // 创建一个原生套接字，返回套接字描述符
    // 原生套接字:发送给本机的包都能收到，不进行过滤。
    // 它仅仅是一份副本，包含头部，不影响原来packet到到协议栈。
    int sock;
    sock = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
    if(sock <= 0){
        perror("error in socket");
        return -1;
    }

    // 打开以太网卡的混杂模式
    // 无线网卡怎么打开监听模式，我还不知道
    struct packet_mreq mr;
    mr.mr_type = PACKET_MR_PROMISC;
    setsockopt(sock,SOL_PACKET,PACKET_ADD_MEMBERSHIP,&mr,sizeof(mr));

    // 抓取包
    //为什么PACKET_SIZE设置512？
    int PACKET_SIZE = 512; 
    char buf[PACKET_SIZE];
    struct sockaddr_in client;
    int client_len;
    while (1){
        
        int receive_date_size = recvfrom(sock,buf,sizeof(buf)-1,0,(struct sockaddr *)&client,&client_len);
        printf("receive data size : %d\n",receive_date_size);

        // if( ntohs(client.sin_port) == 1080 ){
        //     bzero(buf,sizeof(buf));
        //     // 1.0.0.0:2048 、 3.0.0.0:2048
        //     printf("receive data size : %d\n",receive_date_size);
        //     printf("client address is %s:%d\n",inet_ntop(AF_INET,&client.sin_addr,buf,sizeof(client)), 
	 	// 								ntohs(client.sin_port));
        // }

    }
    
    close(sock);
    return 0;
}