* 仔细研读代码 [InterfaceTool.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/17%20ioctl%20操作/progs/InterfaceTool.c) 注意理解是如何修复作者在 UNPv3 中 `len = sizeof(struct sockaddr)` 所导致的 bug 的。

* 使用 ioctl 可以做的事
   * FIONREAD 获取接收缓冲区中的排队数据量
   * SIOCGIFCONF 获取所有接口信息 struct ifreq 结构体中存储。
      * 一定要注意缓冲区的分配，以及缓冲区中如何寻找下一个 struct ifreq，需要使用 `sizeof(test.ifr_ifru)`
   * 使用 SIOGIFFLAGS 获取接口标志
      * IFF_UP  // 是否处于工作状态
      * IFF_POINTOPOINT 
      * IFF_LOOPBACK  // 是否是环回地址接口
      * IFF_BROADCAST  // 是否支持广播
      * IFF_MULTICAST // 是否支持多播
   * 使用 SIOGIFMTU 获取 MTU
   * 使用 SIOCGIFBRDADDR 获取广播地址
   * 使用 SIOCGIFMULTIADDR 获取多播地址
   * 使用 SIOCGIFDSTADDR 获取点对点地址
   * 使用 SIOCGARP 获取 arp 映射 -- 仅对于支持的操作系统来说。。然而据我测试，Mac 上没有 SIOCGARP 宏定义，在 linux 上，使用 ioctl(fd, SIOCGARP, xxx) 无论我传什么，都是报错。测试程序 [ARPOperation.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/17%20ioctl%20操作/progs/ARPOperation.c)