   在这个实验中，我们会快速观察 DHCP，4.4.2 节中被讲述了 DHCP。回忆一下，DHCP 被扩展性地应用于在企业，大学，以及家庭的有线或无线网络中，为主机动态分配 IP 地址（同时也用于配置其他网络配置信息）。
  这个实验是简要的，因为我们只检查被主机捕获的 DCHP 分组，如果你也有权访问你的
DHCP 服务器，你可能想在进行一些配置的改变后重复该实验（比如租期），如果你在家中有一台路由器，你很有可能有能力配置你的 DHCP 服务器，因为很多 linux/Unix 机器（特别是那些为用户所服务的）有一个静态的 IP 地址，并且因为在这些机器上操作 DHCP 经常需要超级用户特权，因此我们仅仅在下面展示这个实验的一个 Windows 版本。

DHCP 实验
为了观察 DHCP 的运转，我们将执行一系列 DHCP 相关的命令，并捕获这些命令所产生的 DCHP 交换报文，做下列的步骤：

⚠️我用的是 Mac，所以实验描述的 Windows 的下列步骤改成相应的 Mac 版

 * 1、打开终端

 * 2、在终端输入 `sudo ipconfig set en0 NONE`，这句 shell 命令作用是解除当前 DHCP 服务器分配给接口 en0 的 IP 地址。
 
 * 3、打开 Wireshake，开始捕获分组，并输入 `bootp` 过滤出 DHCP 分组报文。
 
 * 4、在终端输入 `sudo ipconfig set en0 DHCP`，这句 shell 命令的作用是发送 DCHP request 报文，重新请求 DHCP 服务器分配 IP 地址。
 
 * 5、停止捕获分组。 

 根据上述步骤获得了下列结果如图：
 
 ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_dhcp_1.png)

现在让我们来看一看 Wireshake 窗口的结果，为了看到 DHCP 报文，在过滤输入框输入 `bootp` (DHCP 起源于更老的被称为 BOOTP 的协议，BOOTP 和 DHCP 都使用想相同的端口号，67 和 68，为了在当前版本的 Wireshake 中看到 DHCP 分组，你需要输入 `bootp` 而非 `dhcp`)我们可以从上图中看出 `sudo ipconfig set en0 DHCP` 导致了 DHCP 报文的产生：DHCP Request、DHCP NAK、DHCP Discover、DHCP Offer、DHCP Request、DHCP ACK

回答下列问题：

  * 1、DHCP 基于 UDP 还是 TCP 发送？
    *  ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_dhcp_1.png)
    * 从上图可以看出，DHCP 基于的运输层协议是 UDP，DHCP 是一个应用层协议。
  
  * 2、画出数据报的时序图解 在服务器和客户端之间交换的头四个分组 Discover/Offer/Request/ACK 。对每个分组，指出源端口号和目的端口号，这些端口号与在实验作业中给出的相同吗？
    * ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_dhcp_2.png)
  
  * 3、你的主机的链路层地址是什么？
    * 即 Mac 地址，见下图
      *  ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_dhcp_3.png)
  
  * 4、在 DHCP 发现报文中什么字段的值区分了它和 DHCP 请求报文。
    * DHCP 报文中的 `Option` 字段中的 `DHCP Message Type` 字段区分了 `DHCP` 报文的类型，见下图
    * ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_dhcp_4.png)
    * ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_dhcp_5.png)
    * ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_dhcp_6.png)
    * ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_dhcp_7.png)
  
  * 5、这头四个报文的 Transaction-ID 是什么值？第二次 Request/ACK DHCP 报文中，Transaction-ID 又是什么值？设计这个 Transaction-ID 字段的目的是什么？
  	* ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_dhcp_8.png)
   * 头四个 DHCP 交换报文的 Transaction-ID 值是 0xc02d4d34
  
   * 第二次 Request/ACK DHCP 报文中，Transaction-ID 是 0xc02d4d35，递增了一
  
  * 设计这个字段的目的是区分每次客户端发起 IP 地址请求的事务，在一次 IP 地址的请求，分配和确认中，使用的都是同一个 Transaction-ID
  
  * 6、一台主机使用 DHCP 获取一个 IP 地址，但是一台主机的 IP 地址直到四个报文的交换结束之后才能被确认。如果 IP 地址直到四种报文交换之后才被确认，那么这四种报文的 IP 数据报中的 IP 字段是什么值呢？对于每种 DHCP 报文，指出在 IP 数据包中携带的源 IP 地址和目的 IP 地址。
  
    * **DHCP Discover** 负责侦察附近的 DHCP 服务器，因为客户端即不知道自己的 **IP** 地址，也不知道 **DHCP** 服务器的地址（它甚至不知道是否存在 **DHCP** 服务器），因此它将目的 **IP** 地址填为 `255.255.255.255` 然后广播分组，因为不知道自己的 **IP**，因此源 **IP** 填 `0.0.0.0`
    
    * **DHCP Offer** 是收到 **DHCP Discover** 报文的服务器给客户端提供的回应，相当于告知客户端自己的存在，“嗨！我知道你想要一个 **IP** 地址，我在这呢～” 然而，悲剧的是 **DHCP** 服务器不知道客户端的 **IP** 地址，因为客户端此时还没有被分配 **IP** 地址，于是服务器将该 **Offer** 分组广播出去。因此源 **IP** 填自己的 **IP** 地址，目的 **IP** 填 `255.255.255.255`
    
    * **DHCP Request** 是收到 **Offer** 报文后，向特定 **DHCP** 服务器请求特定的 **IP** 地址，因为可能会有很多个 **DHCP** 服务器给客户端发 **Offer**，因此客户端会在该 **DHCP Request** 报文中携带可选项比如 **DHCP Server Identify** 指定 **DHCP** 服务器，然后用 **DHCP Requested IP** 指定请求的 **IP** 地址，其实在这一步，客户端已经将该分配的 **IP** 地址使用起来了。它将目的 **IP** 填为 `255.255.255.255` 广播分组，为了让其他分配了 **IP** 地址的 **DHCP** 服务器回收 **IP**，并将源 **IP** 填为 `0.0.0.0` 或者被分配的 **IP** 地址，两种都可能。
    
    * **DHCP ACK** 就是收到 **Request** 后服务器返回给客户端的确认，在这个 **ACK** 中，会携带 **DNS** 服务器 **IP** 地址，第一跳路由器地址，子网掩码，租期等各种信息。

    * ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_dhcp_9.png)
  
  * 7、DHCP 服务器的 IP 地址是什么？
    * ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_dhcp_9.png)
    * 从上图可以看出，DHCP 服务器的 IP 地址是 `192.168.1.1`
  
  * 8、DHCP 服务器使用 DHCP offer 报文提供给你的主机的 IP 地址是什么？
    * ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_dhcp_10.png)
    * 从上图可以看出，提供给我主机的 IP 地址是 `192.168.1.101`
  
  * 9、在截图的例子中，在主机和 DHCP 服务器之间，没有中继的存在。在 trace 中什么值指示了没有中继的存在，在你的实验中存在一个中继吗？如果存在，那么该中继的 IP 地址是什么？
  * ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_dhcp_10.png)
  * 从上图的 **Relay Agent IP Adress** 可以判断中继是否存在，因为 IP 地址是 `0.0.0.0`，所以可以判断中继不存在。
  
  * 10、解释在 DHCP offer 报文中，路由器和子网掩码这几行的作用
  
  * 11、在页脚 2 提供的 DHCP 抓包轨迹文件中，DHCP 服务器提供了一个特定的 IP 地址给客户端，在客户端对第一个服务器的 offer 报文响应中，客户端是否接收了这个 IP 地址？在客户端响应的哪个位置是这个客户端请求的地址？
  
  * 12、解释租期的作用，在你的实验中租期是多久？
  * ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_dhcp_10.png)
  * 从上图中可以看出，租期是 2 小时，这代表每经过 1 个小时就会发送一个单播的 DHCP Request 给服务器请求续期，如果没有收到对应的 ACK，那么就会在 3/4 租期时发送广播的 DHCP Request。
  
  * 13、DHCP release 报文的作用是啥？DHCP 是否会发布一个客户端 DHCP 请求报文回执的公告？如果客户端的 DHCP release 报文丢失了会发生什么？
  
  * 14、清除 bootp 筛选，在 DHCP 报文交换期间，是否有任何的 ARP 分组被发送或者接收。如果是的话，解释这些 ARP 报文的目的。
   
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  


































