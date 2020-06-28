/**
 * mmap function example
 * gcc -o mmap_eaxmple mmap_eaxmple.c
*/

#include <stdio.h>
#include <sys/types.h>  
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

int main(void){
    struct stat st;
    void *map;
    char read_contents[30]={0};
    char *new_contents = "mmap function example\n";

    int f = open("./zzz", O_RDWR);
    fstat(f,&st);
    
    /*将整个文件映射到内存*/
    map = mmap(NULL,st.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,f,0);

    if(map == MAP_FAILED)
        return 0;
    
    /*从映射内存中读取文件内容*/
    // memcpy(read_contents,map,18);
    // printf("read_contents : %s \n",read_contents);
    // printf("firt line : %s  \n",(char *)map);

    //从文件中，读取一行；一行超过20,取出前20
    FILE *fp = fopen("./zzz","r");
    if(fp == NULL){
        printf("open file failed.");
        return 0;
    }
    fgets(read_contents,sizeof(read_contents),fp);
    fclose(fp);
    printf("first line : %s\n",read_contents);

    /*通过映射内存，向文件中写入内容*/
    memcpy(map,new_contents,strlen(new_contents));

    munmap(map,st.st_size);
    close(f);

    return 0;
