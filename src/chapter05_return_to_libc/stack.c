/**
 * 用来演示缓冲区溢出攻击，return-to-libc:stack.c
 * 我们关闭地址随机化，栈保护，开启栈不可执行
 * sudo sysctl -w kernel.randomize_va_space=0
 * gcc -g -fno-stack-protector -z noexecstack -o stack stack.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void func(char *src){
    char buffer[100]={0};

    strcpy(buffer,src);
}


int main(void){

    char src[400]={0};

    FILE *badfile = fopen("badfile","r");
    if (!badfile){
        printf("no open badfile");
        return 0;
    }

    fread(src,sizeof(char),300,badfile);

    func(src);

    printf("return properly\n");
    return 0;    
}