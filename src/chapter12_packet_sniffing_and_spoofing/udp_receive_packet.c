/**
 * 作用：将进入服务器所有IP的9090端口的UDP内容，输出。
 * 
 * 准备：
 * 一个套接字描述符：scoket函数，指定期望的通信协议类型
 * 一个监听套接字：指定协议族，ip，端口
 * bind：将一个本地协议地址赋予一个套接字（描述符）
 * recvfrom:接受经指定的socket 传来的数据，放入buf --》准备个buffer，一个客户端套接字，套接字长度
 * 
 * 头文件：
 * netinet/in.h 定义了ip地址结构 struct sockaddr_in （struct in_addr作为sockaddr_in 的成员) 
 *                 还有函数 htons等，以及一些宏。
 * sys/socket.h 定义了一些列的socket API函数 如 
 *              socket()，bind() listen() ， send() ，sendmsg()，setsockopt()，等等。
 * unistd.h close函数
 * 
 * 验证：echo "Hello World\!" | nc -4u  127.0.0.1 9090
*/

#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

int main(void){
    int sock;
    struct sockaddr_in server;
    int client_len;
    struct sockaddr_in client;
    char buf[1000];

    // IPV4,数据包套接字，UDP传输协议
    sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(sock <= 0){
        perror("error in socket");
        return -1;
    }

    // 用这个初始胡为零，比memset函数少一个参数
    // sockaddr_in结构体中sin_zero，还没用；整个结构体用之前，我们全部初始化为零
    // IPv4协议族，监听该服务器上所有IP的9090端口
    // 按照网络标准转化字节序
    bzero(&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(9090);

    // 将一个本地协议地址赋予一个套接字（描述符）
    if( bind(sock,(struct sockaddr *)&server,sizeof(server)) < 0){
        perror("error in bind");
        return -1;
    }

    while (1){
        bzero(buf,sizeof(buf));
        // 进入一个慢系统调用；
        recvfrom(sock,buf,sizeof(buf)-1,0,(struct sockaddr *)&client,&client_len);
        printf("%s",buf);
    }
    
    close(sock);

    return 0;
}

