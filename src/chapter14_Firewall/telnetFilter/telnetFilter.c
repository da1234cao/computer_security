/**
 * 文件名：简单使用netfilter，构建一个包过滤防火墙
 * 作用：阻止TCP协议&&23号端口，数据包流出
 * telnet [IP],被阻止
 * 相关命令：insmod,lsmod,dmsg,rmmod
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/tcp.h>


// 存储勾子的数据结构
// 成员hook即用户定义的勾子函数
// struct nf_hook_ops {
// 	struct list_head list;
// 	/* User fills in from here down. */
// 	nf_hookfn *hook;
// 	struct module *owner;
// 	u_int8_t pf;
// 	unsigned int hooknum;
// 	/* Hooks are ordered in ascending priority. */
// 	int priority;
// };
static struct nf_hook_ops telnetFilterHook;

unsigned int telnetFilter(void *priv, struct sk_buff * skb, const struct nf_hook_state *state){

            // 注意下 sk_buff的数据结构，直接取不出来的
            // https://blog.csdn.net/hzj_001/article/details/100621914
            // struct iphdr *iph = (struct iphdr *)skb;
            // struct tcphdr *tcph = (struct tcphdr *)(iph+iph->ihl*4);

            struct iphdr *iph;
            struct tcphdr *tcph;
            iph = ip_hdr(skb);
            tcph = (void *)iph+iph->ihl*4;

            if(iph->protocol == IPPROTO_TCP && tcph->dest == htons(23)){
                printk(KERN_INFO "Dropping telent packdt to %d.%d.%d.%d\n",
                ((unsigned char *)&iph->daddr)[0],
                ((unsigned char *)&iph->daddr)[1],
                ((unsigned char *)&iph->daddr)[2],
                ((unsigned char *)&iph->daddr)[3]);
                return NF_DROP;
            }else{
                return NF_ACCEPT;
            }
            
        }


void removeFilter(void){
    printk(KERN_INFO "Telnet filter has been removed.\n");
    nf_unregister_net_hook(&init_net,&telnetFilterHook);
}

int setUpFilter(void){
    
    telnetFilterHook.hook = telnetFilter;
    telnetFilterHook.hooknum = NF_INET_POST_ROUTING;
    telnetFilterHook.pf = PF_INET;
    telnetFilterHook.priority = NF_IP_PRI_FILTER;

    if(nf_register_net_hook(&init_net,&telnetFilterHook)!=0){
        printk(KERN_WARNING "register Telnet filter hook error!\n");
        goto err;
    }
    printk(KERN_INFO "Registering a Telnet filter");
    return 0;

err:
    removeFilter();
    return -1;
}

module_init(setUpFilter);
module_exit(removeFilter);

MODULE_LICENSE("GPL");
