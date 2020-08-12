#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <linux/if_tun.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/select.h>

#include "vpn.h"

int main(int argc, char *argv[]){
    int tunfd = createTunDevice();
    int sockfd = initUDPServer();

    while (1){
        fd_set readFDset;

        FD_ZERO(&readFDset);
        FD_SET(tunfd,&readFDset);
        FD_SET(sockfd,&readFDset);
        select(FD_SETSIZE,&readFDset,NULL,NULL,NULL);

        if(FD_ISSET(tunfd,&readFDset))
            tunSelected(tunfd,sockfd);
        if(FD_ISSET(sockfd,&readFDset))
            socketSelected(tunfd,sockfd);
    }
    
}