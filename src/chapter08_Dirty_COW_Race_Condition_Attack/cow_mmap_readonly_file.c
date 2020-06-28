/**
 * 文件名：cow_map_readonly_file.c
 * 编译：gcc -o cow_map_readonly_file.c cow_map_readonly_file
 * 作用：了解copy-on-write(COW)机制
 * 操作：
 * 1. 使用/proc/self/mem对只读内存进行写操作。由于COW机制，并没有修改原内存，是复制到新内存。
 * 2. 之后使用madvise，放弃已修改的新内存
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
    char read_content[20]={0};
    char *new_content = "This is new content";

    int f = open("./zzz",O_RDONLY);//对于others,文件仅有读权限 
    fstat(f,&st);

    void *map = mmap(NULL,st.st_size,PROT_READ,MAP_PRIVATE,f,0);//将文件与内存映射

    int fm = open("/proc/self/mem",O_RDWR);//打开该进程内存对应的伪文件

    lseek(fm,(__off_t)map,SEEK_SET); //定位读写位置到map，我们可以通过内存直接修改文件内容

    /**
     * 将内容写到只读文件？否
     * COW机制:复制一份到内存中--》页表映射到新内存位置--》内容也写入新内存位置
     * 文件映射的内存被标识为COW，在写之前确实检查了，所及执行后面的复制操作。
    */
    write(fm,new_content,strlen(new_content)); 

    
    // memcpy(read_content,map,strlen(read_content));//读取部分新内存位置内容
    memcpy(read_content,map,sizeof(read_content)-1);//读取部分新内存位置内容
    printf("content after write: %s \n",read_content);

    madvise(map,st.st_size,MADV_DONTNEED);//丢弃新的内存位置，map指向原来位置

    memcpy(read_content,map,strlen(read_content));//读取部分原来内存位置内容
    printf("content after madvise: %s \n",read_content);


    return 0;
}
