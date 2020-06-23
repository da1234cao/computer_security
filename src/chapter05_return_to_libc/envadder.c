/**
 * 查看环境变量的地址
 * break main | run | x /100s *((char **)environ)
*/

#include <stdio.h>
#include <stdlib.h>

int main(void){
    char *shell = getenv("MYSHELL");

    if(shell){
        printf("now env the char point size : %u\n",sizeof(char *));

        printf("MYSHELL IS %s\n",shell);

        //64位，不能用%x了
        printf("The address of MYSHELL 0x%lx\n", (unsigned long int)shell);
    }

    return 0;
}