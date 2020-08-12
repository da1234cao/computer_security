[toc]

## 0. 前言

来源：[《Computer Security》A Hands-on Approach](https://www.handsonsecurity.net/)   — Wenliang Du

所有的代码/文档见github：https://github.com/da1234cao/computer_security

chapter 12 Packet sniffing and spoffing.

这一章有点硬核。知识点不难，但对socket编程有点要求。

我看过[《unix网络编程》第一卷 前五章](https://blog.csdn.net/sinat_38816924/category_9811371.html) ,但背景知识还是相当欠缺。这章内容虽然看完，基本明白，但距离实际动手完成一个简单的有线/无线嗅探工具还挺远。本文，暂时仅仅看看书上的嗅探实现。

<br>

## 1. 摘要与总结

首先介绍了网卡的工作原理，嗅探需要网卡的工作模式（杂乱模式/监控模式）。

为了提早过滤过滤不需要的数据包，介绍了BPF。

接着用三种方式：普通套接字，原生套接字，pcap API，递进的讲述了包的嗅探过程。

<br>

## 2. 包(Packet)的接收

### 2.1 网卡

参考文章：[网卡工作原理详解](https://blog.csdn.net/tao546377318/article/details/51602298) 、[关于网络的混杂模式](https://lihaoquan.me/2018/3/11/network-promisc-mode.html) 、[实战无线网络分析（篇一）无线监听](https://www.aneasystone.com/archives/2016/08/wireless-analysis-one-monitoring.html) 

简单了解下：

> 网卡(Network Interface Card,NIC)是一块被设计用来允许计算机在计算机网络上进行通讯的计算机硬件。每块网卡有一个硬件地址，被称为MAC地址。
>
> 网卡接入网络，使得用户可以通过电缆或无线相互连接。
>
> 用户A要发送信息给B，通过电缆或者无线进行广播。
>
> **每一块网卡都将广播的内容拷贝到自己的网卡内存中，检查头部信息中的MAC地址，判断是否是发送给自己的。如果是则将其拷贝进入内存，否则丢弃。**
>
> 因为MAC唯一，所以最后只有B接受了用户A的信息。

> 维基百科：**混杂模式(promiscuous mode)**。一般计算机网卡都工作在非混杂模式下，此时网卡只接受来自网络端口的目的地址指向自己的数据。当网卡工作在混杂模式下时，网卡将来自接口的所有数据都捕获并交给相应的驱动程序。网卡的混杂模式一般在网络管理员分析网络数据作为网络故障诊断手段时用到，同时这个模式也被网络黑客利用来作为网络数据窃听的入口。在Linux操作系统中设置网卡混杂模式时需要管理员权限。在Windows操作系统和Linux操作系统中都有使用混杂模式的抓包工具，比如著名的开源软件Wireshark。

> **监控模式(monitor mode)**。 类似于有线网络的混杂模式。 无线网卡在监视模式下运行时支持嗅探。 与以太网不同，无线设备面临附近其他设备的干扰。 这将严重阻碍网络连接的性能。 为解决此问题，WIFI设备在不同的信道上传输数据。接入点将附近的设备连接到不同的信道，以减少它们之间的干扰影响。 Wifi网卡还设计为在整个可用带宽和通道的片上进行通信。 将这些卡置于监视模式时，它们将捕获802. 1I帧，这些帧在它们正在侦听的通道上传输。 这意味着，与以太网不同，无线网卡可能会丢失同一网络上的信息，因为它位于不同的信道上。 大多数无线网卡不支持监视器模式或制造商禁用了该模式。

<br>

### 2.2 (BSD Packet Filter)BPF

参考文章：[BPF与eBPF](http://sniffer.site/2019/11/07/BPF%E4%B8%8EeBPF/)

这个参考文章我没懂。目前大概知道怎么回事就好。后面使用pcap API会用到这里的概念。

> 嗅探网络流量时，嗅探器只对某些类型的数据包感兴趣，例如TCP数据包或DNS查询数据包，这是很常见的。系统可以将所有捕获的数据包提供给嗅探器程序，后者可以丢弃不需要的数据包。这是非常低效的，因为**处理这些不需要的数据包并将其从内核传送到嗅探器应用程序需要花费时间**。当有很多不需要的数据包时，浪费的时间相当可观。最好**尽早过滤这些不需要的数据包**。
>
> 随着对数据包捕获的需求的增加，Unix操作系统定义了BSD数据包过滤器（BSD Packet Filter,BPF）以支持较低级别的过滤。 BPF允许用户空间程序将过滤器附加到套接字，这实际上是告诉内核尽早丢弃不需要的数据包[Schulist等，2017]。**过滤器通常使用布尔运算符以人类可读的格式编写**，并被编译为伪代码并传递给BPF驱动程序。然后由BPF伪机（一个专门为数据包过滤设计的内核级状态机）解释该低级代码。

<br>

## 3. 包的嗅探

### 3.1 通常套接字编程接收包

```c
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

    // 用这个初始化为零，比memset函数少一个参数
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


```

<br>

### 3.2 使用原生套接字接收包

> 上面的程序只能接收预定的数据包。 如果目标IP地址是另一台计算机，或者目标端口号不是该程序注册的计端口，则该程序将无法获取数据包。 嗅探器程序需要捕获网络电缆上流动的所有数据包，而不管目标IP地址或端口号如何。这可以使用一种称为原始套接字的特殊类型的套接字来完成。 下面的程序显示了一个使用原始套接字的简单嗅探程序。 除了设置原始套接字，程序的其余部分与普通套接字程序非常相似。

```c
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
    int sock = socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
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
    // 为什么PACKET_SIZE设置512？
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
```

<br>

### 3.3 使用pcap API接收包

参考文章：[Linux 网络编程—— libpcap 详解](https://blog.csdn.net/tennysonsky/article/details/44811899#commentBox) 、man 3 pacp

```shell
sudo apt-get install -y libpcap-dev
```

> 上面的程序是一个非常简单的嗅探器程序，它没有显示如何设置数据包过滤器。 如果确实要设置一些过滤器，则必须使用另一个带有SO_ATTACH_FIIIER选项名称的setsockeopt() call。 这样的程序不能跨不同的操作系统移植。 此外，我们的简单程序不会探索任何优化来提高其性能； 在网络繁忙的网络中，我们简单的嗅探器程序可能无法捕获所有数据包。 有许多优化措施可以使数据包嗅探器更快，更高效。

> **因此创建了pcap（package capture）API，以提供独立于平台的接口，以有效地访问操作系统的数据包捕获工具。** pcap的功能之一是允许程序员使用人类可读的布尔表达式指定过滤规则的编译器。 编译器将表达式转换为BPF伪代码，内核可以使用该伪代码。 pcap API在Unix中以libpcap的形式实现，在Windows中以Winpcap的形式实现。 在Linux中，pcap是使用原始套接字实现的。

```c
#include <stdio.h>
#include <pcap.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <net/ethernet.h> 

// 用户传入的参数，抓取的包的头部，抓取的包
void got_packet(__u_char *args,const struct pcap_pkthdr *header,const __u_char *packet){
    struct ether_header *etheader = (struct ether_header *)packet;
    printf("etheader->ether_type is :%hu",etheader->ether_type);
    // IPv4以太类型
    if(etheader->ether_type == 0x0800){
        // 跨过以太网帧头部，指向IP包头
        // struct ip *ipheader = (struct ip *)(packet + sizeof(struct ether_header) - 2);
        struct ip *ipheader = (struct ip *)(packet + sizeof(struct ether_header));
        // printf("src ip is : %s",inet_ntop())
        printf("src ip is : %s\n",inet_ntoa(ipheader->ip_src));
        printf("des ip is : %s\n\n",inet_ntoa(ipheader->ip_dst));

        // struct iphdr *ipheader = (struct iphdr *)(packet + sizeof(struct ether_header));

        switch (ipheader->ip_p){
        case IPPROTO_TCP:
            printf("Protocal : TCP\n");
            break;
        case IPPROTO_UDP:
            printf("Protocal : UDP\n");
            break;
        case IPPROTO_ICMP:
            printf("Protocal : ICMP\n");
            break;
        default:
            printf("Uknow Protocal\n");
        }
    }

    return;
}


int main(void){
    // 获取网络接口设备名
    char error_content[PCAP_ERRBUF_SIZE] = {0};	// 出错信息
    char *dev = pcap_lookupdev(error_content);
    if(NULL == dev){
        printf("error_content:%s\n",error_content);
        return -1;
    }
    printf("dev:%s\n",dev);

    //获得网络号和掩码
    bpf_u_int32 netp = 0, maskp = 0;
    int ret = 0;
    ret = pcap_lookupnet(dev, &netp, &maskp, error_content);
    if(ret == -1){
        printf("error_content:%s\n",error_content);
        return -1;
    }
    printf("netp: %u  maskp: %u\n",netp,maskp); //无符号32位,对应一个IPv4

    // 打开网络接口
    /** dev:网络设备；BUFSIZ捕获的数据包长度，不能超多65535字节；1” 代表混杂模式
     * 指定需要等待的毫秒数，0 表示一直等待直到有数据包到来；
    */
    pcap_t * pcap_handle = pcap_open_live(dev, BUFSIZ, 1, 0, error_content);
    if(pcap_handle == NULL)
    {
        printf("error_content:%s\n",error_content);
        return -1;
    }

    // 添加一个过滤器
    // 过滤规则：filter_exp；不优化；指定子网掩码
    // char filter_exp[] = "ip proto icmp";
    char filter_exp[] = "dst host 10.0.2.5";
    struct bpf_program fp;
    pcap_compile(pcap_handle,&fp,filter_exp,0,maskp);
    pcap_setfilter(pcap_handle,&fp);

    // 循环捕获网络数据包，直到遇到错误或者满足退出条件。
    pcap_loop(pcap_handle,-1,got_packet,error_content);

    // free up allocated memory pointed to by a bpf_program -  
    // struct generated by pcap_compile() when that BPF program is no longer needed
    pcap_freecode(&fp);

    // 关闭handle
    pcap_close(pcap_handle);
    
    return 0;
}
```

<br>

---

悟空被压在五行山下五百年。

![猴哥](packet_sniffing.assets/猴哥.png)