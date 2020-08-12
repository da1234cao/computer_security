/**
 * 文件名：tun.c
 * 创建一个临时虚拟网卡
*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <linux/if_tun.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(void){
    int tunfd;
    struct ifreq ifr;
    bzero(&ifr,sizeof(ifr));

    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    tunfd = open("/dev/net/tun",O_RDWR);
    ioctl(tunfd,TUNSETIFF,&ifr);

    // 我们可以通过这个文件描述符和这个虚拟网卡设备交互
    // 我们通过创建一个shell进行交互

    printf("TUN file descriptor : %d \n",tunfd);
    execve("/bin/bash",NULL,NULL);

    return 0;
}