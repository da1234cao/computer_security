[toc]

## 0.前言

出于需要，我搭建了一个最简单的DNS服务器。

开始的时候，我在腾讯云的服务器上搭建，但是被官方通知：”数据违规处理，在24小时内提交域名解析许可资质....“。所以我转到虚拟机上，进行搭建。

问题来了，我只有一台使用NAT转换模式的虚拟机，没法验证DNS服务是否搭建起来（因为没有可以访问该虚拟机的机器，所以无法验证）。再创建一个虚拟机比较消耗资源。所以，我决定给虚拟机加一张网卡，用以虚拟机和主机之间的访问。

虽然网上虚拟机双网卡配置的文章巨多，但是折腾这个花了我三天时间。。

我这里**整理一个虚拟机配置双网卡的可行解**，非最优解。

环境：

|        软件        |   版本   |
| :----------------: | :------: |
| 主机操作系统Ubuntu | 18.04LTS |
|     VirtualBox     |  5.2.42  |
|  虚拟机系统Ubuntu  |  16.04   |

相关命令：

|                             命令                             |      作用       |
| :----------------------------------------------------------: | :-------------: |
|                           ifconfig                           | 查看IP、netmask |
| [route -n](https://blog.csdn.net/vevenlcf/article/details/48026965) |    查看网关     |
| [nmcli](https://access.redhat.com/documentation/zh-cn/red_hat_enterprise_linux/7/html/networking_guide/sec-using_the_networkmanager_command_line_tool_nmcli) |     查看DNS     |

<br>

## 1. 摘要

> <font color=red>”桥接模式“毕竟是相当于在局域网内新加入了一台独立计算机，具有指定的IP地址、子网掩码、网管、DNS等。当然地，尤其是IP地址，这是有限的，而且更多情况下如果是在公司里，这个是由网管/运维来统筹处理的，这就有了点”受制于人“的色彩。</font>
>
> <font color=red>现在想要做的是使用NAT网络模式，使得虚拟机可上网，而宿主机也可以通过诸如ssh等协议连接到虚拟机。</font>

首先，我们需要了解两个背景知识：不同网络模式的虚拟机对应的网络拓扑结构、ubuntu中的网络管理方式。网络拓扑结构帮助我们(理解)该如何组织我们的网卡，网络管理使得这些网卡得以正确的配置。

接着，我们分步操作，在一个虚拟机中使用了NAT模式和Host-Only模式。

最终，虚拟机可以同时在外网和内网通信。

通过这篇文章，我们希望掌握：**网络的配置，虚拟机不同模式网络的配置，网络不通的排错处理。**

<br>

## 2. 虚拟机的网络模式

参考：[VMware虚拟机三种网络模式的区别及配置方法](https://blog.csdn.net/Alpha_B612/article/details/80979101#commentBox)、[快速理解VirtualBox的四种网络连接方式(old)](https://www.cnblogs.com/york-hust/archive/2012/03/29/2422911.html)

VMware和VirtualBox除了一个免费，一个非免费的区别外。

<font color=blue>它们在NAT模式上表现不同</font>。VMware的NAT模式中，有个虚拟交换机，所以虚拟机和主机之间可以通信。VirtualBox的NAT(service)模式中，没有虚拟交换机，虚拟机可以访问主机，但主机无法访问虚拟机。

注：VirtualBox Version 6.1.12的[VirtualBox官方文档](http://download.virtualbox.org/virtualbox/UserManual.pdf#section.6.4) 中，NAT(service)模式是可以端口转发的。但是我目前的VirtualBox Version 5.2.42，图形化界面中的端口转发按钮是灰色的，无法使用。

两个东西，必然还有很多不同，我暂时不关心。

我了解下就好，具体我不知道。

<br>

## 3. ubuntu中网络管理

`netwoking`、`NetworkManager`与`netplan`的区别。

**3.1** 先说比较新的系统配置，比如Ubuntu18.04。使用[Netplan](https://www.hi-linux.com/posts/49513.html) 进行网络配置。

如果是桌面端，netplan中`renderer`配置为`NetworkNamager`,接着使用[NetworkNamager 图形化](https://askubuntu.com/questions/1170385/how-to-set-static-ip-address-on-ubuntu-16-04-lts) 进行配置。配置完之后，可以使用[nmcli 命令](https://access.redhat.com/documentation/zh-cn/red_hat_enterprise_linux/7/html/networking_guide/sec-using_the_networkmanager_command_line_tool_nmcli) 搭配着查看信息。

如果是服务器端，netplan中`renderer`配置为`networkd`。接着在`/etc/netplan/*.yaml`写入配置信息。Netplan 根据描述文件中定义的内容自动生成其对应的后端网络守护程序所需要的配置信息，后端网络守护程序再根据其配置信息通过 Linux 内核管理对应的网络设备。

**3.2** 如果是比较旧的系统配置，比如说Ubuntu16.04。

如果是桌面端，使用[NetworkNamager 图形化](https://askubuntu.com/questions/1170385/how-to-set-static-ip-address-on-ubuntu-16-04-lts) + [nmcli 命令](https://access.redhat.com/documentation/zh-cn/red_hat_enterprise_linux/7/html/networking_guide/sec-using_the_networkmanager_command_line_tool_nmcli) 。

如果是服务器端，配置`/etc/network/interfaces`，然后重启`networking.service`就好。

这两者是搭配使用：[NetworkManager](https://wiki.debian.org/NetworkManager)

> NetworkManager will only handle interfaces not declared in `/etc/network/interfaces`
>
> Set `managed=false` in `/etc/NetworkManager/NetworkManager.conf`.
>
> <br>
>
> If you want NetworkManager to handle interfaces that are enabled in /etc/network/interfaces
>
> Set `managed=true` in `/etc/NetworkManager/NetworkManager.conf`.

<br>

## 4. 虚拟机配置NAT模式上网

参考：[virtualbox中linux设置NAT和Host-Only上网（实现双机互通同时可上外网）](https://blog.csdn.net/IndexMan/article/details/79558386) 、[How to set static IP address on Ubuntu 16.04 LTS?](https://askubuntu.com/questions/1170385/how-to-set-static-ip-address-on-ubuntu-16-04-lts)、[Virtualbox中Nat和Nat Network模式区别？](https://www.zhihu.com/question/277077127)

**使用NetworkManager图形化设置**，不要修改interfaces文件。

此时虚拟机可以访问主机，主机无法访问虚拟机。

我建议：<font color=blue>主机和虚拟机采用不同的网段。避免虚拟机的IP和主机的IP相同</font>。

|            |     主机      |        虚拟机        |
| :--------: | :-----------: | :------------------: |
|    Inet    | 192.168.1.108 |    192.168.50.10     |
|  Netmask   | 255.255.255.0 |    255.255.255.0     |
|  Gateway   |  192.168.1.1  |     192.168.50.1     |
| Nameserver |  172.31.5.5   | 192.168.50.2,8.8.8.8 |

<br>

## 5. 虚拟机配置NAT模式上网,Host-Only模式

此时主机多出来一张虚拟网卡`vboxnet0`，它是由VirtualBox创建，用于主机访问虚拟机。

|            |   主机网卡    | 主机虚拟网卡  |    虚拟机NAT网卡     | 虚拟机Host-Only网卡 |
| :--------: | :-----------: | :-----------: | :------------------: | :-----------------: |
|    inet    | 192.168.1.108 | 192.168.56.1  |    192.168.50.10     |    192.168.56.10    |
|  netmask   | 255.255.255.0 | 255.255.255.0 |    255.255.255.0     |    255.255.255.0    |
|  Gateway   |  192.168.1.1  |    0.0.0.0    |     192.168.50.1     |       0.0.0.0       |
| nameserver |  172.31.5.5   |      ---      | 192.168.50.2,8.8.8.8 |         ---         |

我们可以看到主机虚拟网卡和虚拟机Host-Only网卡的网关都被设置成0.0.0.0。

```shell
# 主机的路由表
route -n
目标            网关            子网掩码        标志  跃点   引用  使用 接口
0.0.0.0         192.168.1.1     0.0.0.0         UG    600    0        0 wlp3s0
169.254.0.0     0.0.0.0         255.255.0.0     U     1000   0        0 wlp3s0
192.168.1.0     0.0.0.0         255.255.255.0   U     600    0        0 wlp3s0
192.168.56.0    0.0.0.0         255.255.255.0   U     0      0        0 vboxnet0

# 虚拟机的路由表
Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
0.0.0.0         192.168.50.1    0.0.0.0         UG    100    0        0 enp0s3
169.254.0.0     0.0.0.0         255.255.0.0     U     1000   0        0 enp0s3
192.168.50.0    0.0.0.0         255.255.255.0   U     100    0        0 enp0s3
192.168.56.0    0.0.0.0         255.255.255.0   U     100    0        0 enp0s8
```

好的设置方法，如上所示，虚拟机Host-Only网卡的网关被设置成0.0.0.0，表示无网关，即与本机IP同一网段，不需要经过网关。可以参考：[教你读懂路由表](http://blog.smallmuou.xyz/%E8%BF%90%E7%BB%B4/2017/04/14/%E6%95%99%E4%BD%A0%E8%AF%BB%E6%87%82%E8%B7%AF%E7%94%B1%E8%A1%A8.html)

另一种设置方法，是主机双网卡网关的设置方法，参考：[Ubuntu系统双网关配置](https://www.kclouder.cn/ubuntu-gateway/)。双网卡，如果都设置网关，将有两个默认路由，导致访问非路由网段的时候，冲突。超链接文章中，通过修改metric(路由开销,这个我不清楚)，优先使用连接外网的网卡。为了可以使用内部网卡，访问内网，添加了一条访问内网段的路由。由于只有路由表中找不到，才使用默认路由，所以这个方法目测可行。这也使得，内部网卡的网关没作用，不如上一种方法，直接设置成0.0.0.0来的方便。

<br>

## 参考文章

虚拟机：

[VMware虚拟机三种网络模式的区别及配置方法](https://blog.csdn.net/Alpha_B612/article/details/80979101#commentBox)

[virtualbox中linux设置NAT和Host-Only上网（实现双机互通同时可上外网）](https://blog.csdn.net/IndexMan/article/details/79558386)

网络管理：

[如何在 Ubuntu 18.04 下正确配置网络](https://www.hi-linux.com/posts/49513.html)

[How to set static IP address on Ubuntu 16.04 LTS?](https://askubuntu.com/questions/1170385/how-to-set-static-ip-address-on-ubuntu-16-04-lts)

[2.3. 使用 NetworkManager 命令行工具 nmcli](https://access.redhat.com/documentation/zh-cn/red_hat_enterprise_linux/7/html/networking_guide/sec-using_the_networkmanager_command_line_tool_nmcli)



[VirtualBox 官方文档](https://www.virtualbox.org/wiki/Documentation) ---> 我没看