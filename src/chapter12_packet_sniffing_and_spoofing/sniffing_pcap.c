#include <stdio.h>
#include <pcap.h>

// 用户传入的参数，抓取的包的头部，抓取的包
void got_packet(__u_char *args,const struct pcap_pkthdr *header,const __u_char *packet){
    printf("Got a packet\n");
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
    /** dev:网络设备；1024捕获的数据包长度，不能超多65535字节；1” 代表混杂模式
     * 指定需要等待的毫秒数，0 表示一直等待直到有数据包到来；
    */
    pcap_t * pcap_handle = pcap_open_live(dev, BUFSIZ, 1, 0, error_content);
    if(pcap_handle == NULL)
    {
        printf("error_content:%s\n",error_content);
        return -1;
    }

    // 循环捕获网络数据包，直到遇到错误或者满足退出条件。
    pcap_loop(pcap_handle,-1,got_packet,error_content); 

    // 关闭handle
    pcap_close(pcap_handle);

    return 0;
}

