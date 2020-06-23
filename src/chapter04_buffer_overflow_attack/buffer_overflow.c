/**
 * 演示缓冲区溢出
 * gcc -g -fno-stack-protector -z execstack -o stack stack.c
*/

#include <stdio.h>
#include <string.h>

void func(char *src){
    /*我们可以尝试注释这一段，然后运行查看结果;这里恰好dest_bak变量在dest之后*/
    char dest_bak[20]={0}; 

    char dest[5]={0};
    strcpy(dest,src);

    printf("%s\n",dest);
}

int main(void){
    char src[]="I am a boy";
    func(src);

    printf("Done.\n");
    return 0;
}