/**
 * 我们看下main中包含该的sh
 * 直接编译出来的二进制文件是什么样子的
 * 也看不出来什么哈
*/

#include<unistd.h>

int main(void){
    char *name[2];
    name[0]="/bin/sh";
    name[1]=NULL;

    execve(name[0],name,NULL);
}