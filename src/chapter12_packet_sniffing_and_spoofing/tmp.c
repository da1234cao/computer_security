/**
 * 代码来源于：https://zhuanlan.zhihu.com/p/37324898
*/

#include<stdio.h>
#include<pcap.h>
#include<unistd.h>
#include<stdlib.h>
#include<netinet/ip.h>
#include<netinet/ip_icmp.h>
#include<netinet/tcp.h>
#include<netinet/udp.h>
#include<netinet/ether.h>
#include<arpa/inet.h>
#include<ctype.h>
#define ETHER_SIZE 14

void print_hex_ascii_line(const __u_char *payload, int len, int offset)
{
    int i;
    int gap;
    const __u_char *ch;
    /* offset */
    printf("%05d   ", offset);
    /* hex */
    ch = payload;
    for(i = 0; i < len; i++) {
        printf("%02x ", *ch);
        ch++;
        /* print extra space after 8th byte for visual aid */
        if (i == 7)
            printf(" ");
    }
    /* print space to handle line less than 8 bytes */
    if (len < 8)
        printf(" ");

    /* fill hex gap with spaces if not full line */
    if (len < 16) {
        gap = 16 - len;
        for (i = 0; i < gap; i++) {
            printf("   ");
        }
    }
    printf("   ");

    /* ascii (if printable) */
    ch = payload;
    for(i = 0; i < len; i++) {
        if (isprint(*ch))
            printf("%c", *ch);
        else
            printf(".");
        ch++;
    }

    printf("\n");

return;
}

void print_payload(const __u_char *payload, int len)
{

    int len_rem = len;
    int line_width = 16;            /* number of bytes per line */
    int line_len;
    int offset = 0;                 /* zero-based offset counter */
    const __u_char *ch = payload;

    if (len <= 0)
        return;

    /* data fits on one line */
    if (len <= line_width) {
        print_hex_ascii_line(ch, len, offset);
        return;
    }

    /* data spans multiple lines */
    for ( ;; ) {
        /* compute current line length */
        line_len = line_width % len_rem;
        /* print line */
        print_hex_ascii_line(ch, line_len, offset);
        /* compute total remaining */
        len_rem = len_rem - line_len;
        /* shift pointer to remaining bytes to print */
        ch = ch + line_len;
        /* add offset */
        offset = offset + line_width;
        /* check if we have line width chars or less */
        if (len_rem <= line_width) {
            /* print last line and get out */
            print_hex_ascii_line(ch, len_rem, offset);
            break;
        }
    }

return;
}

///get_packet()回调函数
///header:收到的数据包的pcap_pkthdr类型的指针
///packet:收到的数据包数据
void get_packet(__u_char*args, const struct pcap_pkthdr *header,const __u_char *packet){

    static int count = 1;
    const char * payload;
    printf("==================================packet number: %d=============================\n",count++);

    ///ETHER_SIZE：以太帧首部长度14个字节
    ///IP包头(tcp包头(数据))
    ///IP包头(udp包头(数据))
    ///IP包头(icmp包头(数据))
    struct ip * ip = (struct ip *)(packet + ETHER_SIZE);
    printf("IP header length: %d\n",ip->ip_hl<<2);
    printf("From %s\n",inet_ntoa(ip->ip_src));
    printf("To %s\n",inet_ntoa(ip->ip_dst));
    int ip_hl = ip->ip_hl<<2;

    ///对报文类型进行了扩展
    ///可以分析IP包、ICMP包、TCP包、UDP包
    switch(ip->ip_p){

        case IPPROTO_TCP:
        {
            printf("----Protocol TCP----\n");
            struct tcphdr *tcp = (struct tcphdr *)(packet + 14 + ip_hl);           
            printf("tcp -> source:%d\n",ntohs(tcp -> source));
            printf("tcp -> dest:%d\n",ntohs(tcp -> dest));
            printf("tcp -> seq:%d\n",ntohs(tcp -> seq));
            printf("tcp -> ack_seq:%d\n",ntohs(tcp -> ack_seq));     
            printf("tcp -> headerLenth:%d\n",tcp -> doff << 2);
            printf("tcp -> fin:%d\n",tcp -> fin);
            printf("tcp -> syn:%d\n",tcp -> syn);
            printf("tcp -> rst:%d\n",tcp -> rst);
            printf("tcp -> psh:%d\n",tcp -> psh);
            printf("tcp -> ack:%d\n",tcp -> ack);
            printf("tcp -> urg:%d\n",tcp -> urg);
            printf("tcp -> window:%d\n",ntohs(tcp -> window));
            printf("tcp -> check:%d\n",ntohs(tcp -> check)); 

            int h_size = tcp->doff<< 2;
            int payload_size = ntohs(ip->ip_len) - ip_hl - h_size;

            int i = payload_size;
            printf("payload is:%d\n",i);
            print_payload((char*)tcp + h_size,payload_size);

            break;
        }
        case IPPROTO_UDP:
        {
            printf("----Protocol UDP----\n");
            struct udphdr *udp = (struct udphdr *)(packet + 14 + ip_hl);           
            printf("udp -> source:%d\n",ntohs(udp -> source));
            printf("udp -> dest:%d\n",ntohs(udp -> dest));
            printf("udp -> length:%d\n",ntohs(udp -> len));
            printf("udp -> check:%d\n",ntohs(udp -> check));
            int payload_size = ntohs(ip->ip_len) - ip_hl - 8;
            int i = payload_size;
            printf("payload is:%d\n",i);
            print_payload((char*)udp +8,payload_size);

            break;
        }
        case IPPROTO_ICMP:
        {
            printf("----Protocol ICMP----\n");
            struct icmphdr *icmp = (struct icmphdr *)(packet + 14 + ip_hl);

            if(icmp -> type == 8)
            {
                printf("--icmp_echo request--\n");
                printf("icmp -> type:%d\n",icmp -> type);
                printf("icmp -> code:%d\n",icmp -> code);
                printf("icmp -> checksum:%d\n",icmp -> checksum);

                printf("icmp -> id:%d\n",icmp -> un.echo.id);
                printf("icmp -> sequence:%d\n",icmp -> un.echo.sequence);
                int payload_size = ntohs(ip->ip_len) - ip_hl - 8;
                int i = payload_size;
                printf("payload is:%d\n",i);
                print_payload((char*)ip + ip_hl +8,payload_size);


            }
            else if(icmp -> type == 0)
            {
                printf("--icmp_echo reply--\n");
                printf("icmp -> type:%d\n",icmp -> type);
                printf("icmp -> code:%d\n",icmp -> code);
                printf("icmp -> checksum:%d\n",icmp -> checksum);            

                printf("icmp -> id:%d\n",icmp -> un.echo.id);
                printf("icmp -> sequence:%d\n",icmp -> un.echo.sequence);
                int payload_size = ntohs(ip->ip_len) - ip_hl - 8;
                int i = payload_size;
                printf("payload is:%d\n",i);
                print_payload((char*)ip + ip_hl +8,payload_size);

            }
            else
            {
                printf("icmp -> type:%d\n",icmp -> type);
                printf("icmp -> code:%d\n",icmp -> code);
                printf("icmp -> checksum:%d\n",icmp -> checksum);
                int payload_size = ntohs(ip->ip_len) - ip_hl - 8;
                int i = payload_size;
                printf("payload is:%d\n",i);
                print_payload((char*)ip + ip_hl +8,payload_size);
            }
            break;     
        }
        case IPPROTO_IP:
        {
            printf("----Protocol IP----\n");
            //printf("IP header length: %d\n",ip -> ip_hl<<2);
            printf("IP version: %d\n",ip -> ip_v);
            printf("IP type of service: %d\n",ip -> ip_tos);
            printf("IP total length: %d\n",ip -> ip_len);
            printf("IP identification: %d\n",ip -> ip_id);
            printf("IP fragment offset field: %d\n",ip -> ip_off);
            printf("IP time to live: %d\n",ip -> ip_ttl);
            printf("IP protocol: %d\n",ip -> ip_p);
            printf("IP checksum: %d\n",ip -> ip_sum);
            int payload_size = ntohs(ip->ip_len) - ip_hl;
            int i = payload_size;
            printf("payload is:%d\n",i);           
            print_payload((char*)ip + ip_hl,payload_size);
            break;         
        }          
        default:printf("Protocol unknown\n");
        return;
    }
}

int main(int argc,char*argv[]){

    char *dev, errbuf[PCAP_ERRBUF_SIZE];
    struct bpf_program fp;
    char filter_exp[] = "icmp";
    bpf_u_int32 mask;
    bpf_u_int32 net;
    struct pcap_pkthdr header;
    const __u_char *packet;  
    int num_packets = 10;

    ///pcap_lookupdev()自动获取网络接口，返回一个网络接口的字符串指针
    ///如果出错，errbuf存放出错信息
    ///若想手动指定，则跳过此步，将要监听的网络字符串硬编码到pcap_open_live中
    dev = pcap_lookupdev(errbuf);
    if(dev==NULL){
        printf("ERROR:%s\n",errbuf);
        exit(2);
    }
    printf("The sniff interface is:%s\n",dev);

    ///pcap_lookupnet()获得设备的IP地址，子网掩码等信息
    ///net：网络接口的IP地址
    ///mask：网络接口的子网掩码
    if(pcap_lookupnet(dev,&net,&mask,errbuf)==-1){
        printf("ERROR:%s\n",errbuf);
        net = 0;
        mask = 0;
    }  

    ///pcap_open_live()打开网络接口
    ///BUFSIZ:抓包长度
    ///第三个参数：0代表非混杂模式，1代表混杂模式
    ///第四个参数：等待的毫秒数，超过这个值，获取数据包的函数会立即返回，0表示一直等待直到有数据包到来
    pcap_t * handle = pcap_open_live(dev,BUFSIZ,1,0,errbuf);
    if(handle == NULL){
        printf("ERROR:%s\n",errbuf);
        exit(2);
    }
    ///pcap_compile()编译过滤表达式
    ///fp指向编译后的filter_exp
    ///filter_exp过滤表达式
    ///参数四：是否需要优化过滤表达式
    if(pcap_compile(handle,&fp,filter_exp,0,net)==-1){
        printf("Can't parse filter %s:%s\n",filter_exp,pcap_geterr(handle));
        return(2);
    }

    ///pcap_setfilter()应用这个过滤表达式
    ///完成过滤表达式后，我们可以使用pcap_loop()或pcap_next()登抓包函数抓包了
    if(pcap_setfilter(handle,&fp)==-1){
        printf("cant' install filter %s:%s\n",filter_exp,pcap_geterr(handle));
        return(2);
    }  
    printf("Hello hacker! i am ailx10.welcome to http://hackbiji.top\n"); 
    ///num_packets:需要抓的数据包的个数，一旦抓到了num_packets个数据包，pcap_loop立即返回。负数表示永远循环抓包，直到出错
    ///get_packet：回调函数指针
    //pcap_loop(handle,num_packets,get_packet,NULL);
    pcap_loop(handle,-1,get_packet,NULL);

    pcap_freecode(&fp);

    ///pcap_close()释放网络接口
    ///关闭pcap_open_live()获取的pcap_t的网络接口对象并释放相关资源
    pcap_close(handle);
    return(0);
}