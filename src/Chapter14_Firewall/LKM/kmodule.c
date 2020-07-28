#include <linux/module.h> //加载内核模块到内核使用的核心头文件
#include <linux/kernel.h> // 包含内核使用的类型、宏和函数
#include <linux/init.h>  // 这些定义在，见makefile


static int kmodule_init(void){
    printk(KERN_INFO "Initializing this module!\n");
    return 0;
}

static void kmodule_exit(void){
    printk(KERN_INFO "cleanup this module!\n");
}

module_init(kmodule_init);
module_exit(kmodule_exit);

MODULE_LICENSE("GPL");