/**
 * 展示当使用strcpy时候，遇到\0，终止
*/
#include <stdio.h>
#include <string.h>

int main(void){
    char src[] = "I am a \0 boy";
    char dest[40]={0};
    strcpy(dest,src);

    dest[40-1]='\0';//防止越界输出
    printf("dest is: %s\n",dest);
}