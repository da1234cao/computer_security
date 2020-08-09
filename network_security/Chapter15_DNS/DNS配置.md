参考文章：	http://cn.linux.vbird.org/linux_server/0350dns.php



打开centos服务器的53号端口后，防火墙允许通过。

```shell
# 查看已经开放的端口
sudo firewall-cmd --list-ports

# 53号端口(DNS),开放
sudo firewall-cmd --zone=public --add-port=53/tcp --permanent
sudo firewall-cmd --zone=public --add-port=53/udp --permanent

# 重启服务
sudo systemctl restart firewalld.service
```

查看有哪些配置文件

```shell
# 查看有哪些配置文件
rpm -qc bind
/etc/logrotate.d/named
/etc/named.conf  《-----
/etc/named.iscdlv.key
/etc/named.rfc1912.zones
/etc/named.root.key
/etc/rndc.conf
/etc/rndc.key
/etc/sysconfig/named
/var/named/named.ca 《-----
/var/named/named.empty
/var/named/named.localhost
/var/named/named.loopback
```

先简单尝试下，我们的DNS功能能否使用。

```shell
# 对/etc/named.conf文件进行简单修改
# 监听本机(所有网卡)的53号端口
# 允许任何主机进行DNS查询
# 其中部分如下
options {
        listen-on port 53  { any; };     //可不设定，代表全部接受   《-----
        directory          "/var/named"; //数据库默认放置的目录所在
        dump-file          "/var/named/data/cache_dump.db"; //一些统计信息
        statistics-file    "/var/named/data/named_stats.txt";
        memstatistics-file "/var/named/data/named_mem_stats.txt";
        allow-query        { any; };     //可不设定，代表全部接受《-----
};  //最终记得要结尾符号！

# 那13个根域名服务器
zone "." IN {
	type hint;
	file "named.ca";
};

# 由于这个DNS服务器没有向上一层注册；所以使用的时候，我们进行指定
# 49.234.233.219这个服务器2020//8/25便不续费，之后无法使用
# 发现挺好，至少可以简单使用了
dig www.baidu.com @49.234.233.219
```

我们顺便看下服务重启的过程有没有问题

```shell
systemctl status named
sudo tail -n 500 /var/log/messages | grep named
```

现在，我们来对DNS进行一些“练功”配置。配置完，同样看一看log日志，并进行测试。

```shell
# 在/etc/named.conf后面加上两个zone
zone "centos.vbird" IN {            // 这个 zone 的名称
        type master;                // 是什么类型
        file "named.centos.vbird";  // 档案放在哪里
};
zone "100.168.192.in-addr.arpa" IN {
        type master;
        file "named.192.168.100";
};

# 这两个域内容配置可以参考如下
--------------------------------------------------------------------
# vim /var/named/named.centos.vbird
; 与整个领域相关性较高的设定包括 NS, A, MX, SOA 等标志的设定处！
$TTL    600
@                       IN SOA   master.centos.vbird. vbird.www.centos.vbird. (
                                 2011080401 3H 15M 1W 1D ) ; 与上面是同一行
@                       IN NS    master.centos.vbird.  ; DNS 服务器名称
master.centos.vbird.    IN A     192.168.100.254         ; DNS 服务器 IP
@                       IN MX 10 www.centos.vbird.     ; 领域名的邮件服务器

; 针对 192.168.100.254 这部主机的所有相关正解设定。
www.centos.vbird.       IN A     192.168.100.254
linux.centos.vbird.     IN CNAME www.centos.vbird.
ftp.centos.vbird.       IN CNAME www.centos.vbird.
forum.centos.vbird.     IN CNAME www.centos.vbird.

; 其他几部主机的主机名正解设定。
slave.centos.vbird.       IN A    192.168.100.10
clientlinux.centos.vbird. IN A    192.168.100.10
workstation.centos.vbird. IN A    192.168.1.101
winxp.centos.vbird.       IN A    192.168.100.20
win7                      IN A    192.168.100.30  ; 这是简化的写法！
------------------------------------------------------------------------------
# vim /var/named/named.192.168.100
$TTL    600
@       IN SOA  master.centos.vbird. vbird.www.centos.vbird. (
                2011080401 3H 15M 1W 1D )
@       IN NS   master.centos.vbird.
254     IN PTR  master.centos.vbird.  ; 将原本的 A 改成 PTR 的标志而已

254     IN PTR  www.centos.vbird.     ; 这些是特定的 IP 对应
10      IN PTR  slave.centos.vbird.
20      IN PTR  winxp.centos.vbird.
30      IN PTR  win7.centos.vbird.

101     IN PTR  dhcp101.centos.vbird.  ; 可能针对 DHCP (第十二章) 的 IP 设定
102     IN PTR  dhcp102.centos.vbird.
--------------------------------------------------------------------------------

```

至于slave服务器，上层服务器的设置，见上面的参考文章，我没有尝试。

