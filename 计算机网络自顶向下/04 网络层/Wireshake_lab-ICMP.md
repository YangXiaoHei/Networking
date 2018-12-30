在这个实验中，我们将要从不同方面探索 ICMP 协议。

   * 通过 Ping 程序产生的 ICMP 报文
   
   * 通过 Traceroute 程序产生的 ICMP 报文
   
   * 一个 ICMP 报文的格式和内容

在攻占该实验前，我们鼓励你复习书中 4.4.3 节的 ICMP 材料。我们将基于微软 Windows 平台来进行该实验，虽然将其翻译成一个 Unix 或 Linux 环境的实验也很直接。

* 1. ICMP 和 Ping
  * 让我们以捕获 Ping 程序所产生的分组来开始我们的 ICMP 探险之旅。回想 Ping 程序是一个简单的工具，用于让任何人（比如，一个网络管理员）验证一台主机是否处于活跃（接受请求并服务）状态。源主机的 Ping 程序向目的 IP 发送一个分组，如果目标是活跃状态，那么目标主机的 Ping 程序就向源主机响应一个分组。正如你所猜测的那样（毕竟这是个关于 ICMP 的实验），这两个 Ping 分组都是 ICMP 分组。
  
  * 跟着下面步骤动手做:
    * 打开 Wireshake 并开始捕获分组。
    
    * 在终端中输入 `ping -c 10 域名`
    
    * 等待 Ping 程序退出，然后停止 Wireshake 分组捕获
 
 当实验结束时，你的终端窗口应该看到如图 figure 1 所示。在这个例子中，源主机的 Ping 程序在深圳而目的主机的 Ping 程序在北京，从下面的窗口中我们可以看到，Ping 程序发送了 10 个查询分组并且收到 10 个响应，同时也注意对于每个响应，源主机计算了往返时间，对于这 10 个分组来说平均往返时间是 14.526 ms。
 	
 ![figure 1](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/images/wl_icmp_1.png)	
 
  在 figure 2 中提供了 Wireshake 输出的截图，在筛选框中输入 `icmp` 之后，注意到分组监控展示了 20 个分组，10 个 Ping 查询被源所发出而 10 个 Ping 响应被源所接收。同时也注意到源 IP 地址是一个 CIDR 为 192.168/12 的私有 IP 地址。而目的 IP 地址是百度北京的 Web 服务器 IP 地址，现在让我们把镜头移转到被客户端发送的第一个分组上，在下图中，这个分组的内容区域提供了关于该分组的信息，我们可以看到分组中的 IP 数据报的上层协议号是 01，这标示上层协议是 ICMP，也就是说这个 IP 数据报的载荷是 ICMP 分组报文。
  
  ![figure 2](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/images/wl_icmp_2.png)	
  
  Figure 3 同样强调 ICMP，只是在分组内容窗口中展开了 ICMP 协议的详细信息，管擦好这个 Type 为 8 而且 Code 为 0 的 ICMP 回显分组，同时注意到这个 ICMP 分组包含了一个校验和字段，一个标志位字段，还有一个序列号字段。
  
  ![figure 2](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/images/wl_icmp_3.png)	
  
你应该回答下列问题

* 1、你电脑的 IP 地址是什么？目标主机的 IP 地址是什么？
  * IP 地址是 `10.10.5.168`，目标主机 IP 是 `103.235.46.39`
  
* 2、为什么一个 ICMP 分组没有源和目的端口号？
  * ⚠️ICMP 被设计用来交流主机和路由器之间的网络层信息，而不是在应用层进程之间。每个 ICMP 分组有一个 Type 和 Code 字段，Type/Code 的组合指示了接收到的分组的特定类型，因为网络软件本身解释了所有 ICMP 报文，因此不需要端口号来将 ICMP 报文提交给应用层让应用程序来解释。
  
* 3、检查一个被你电脑发送出去的 Ping 请求分组。ICMP type 和 code 分别是什么？这个 ICMP 分组还有其他什么字段？校验和、序列号、identifier 字段各有多少字节？
  * 发出去的 Type 是 8, code 是 0，还有序号字段，identifier 字段，校验和字段和 data 字段。校验和，序列号，identifier 字段都是 2 字节。
  
* 4、检查相对应的 Ping 回应分组。ICMP type 和 code 分别是什么？这个 ICMP 还有其他什么字段？校验和、序号和 identifier 各有多少字节？
  * 接受到的 ICMP 回应报文 Type 是 0，code 是 0，还有序号字段，identifier 字段，校验和字段和 data 字段，这校验和、序列号、identifier 字段都是 2 字节。


* 2. ICMP 和 Traceroute

让我们通过捕获 Traceroute 程序产生的分组继续我们的 ICMP 探险之旅。回想 Traceroute 程序将可以被用于标明一条分组从源到目的地的所走过的路径。Traceroute 在课本 1.4 和 4.4 节中被讨论过。

traceroute 在 Unix/Linux/MacOs 和 Windows 平台上有不同的实现方式，在 Unix/Linux 平台上，源向目的地发送一系列的 UDP 报文段(使用的端口号未必是目的端口号)，在 Windows 平台上，源向目的地发送一系列的 ICMP 分组，无论是在 Windows 还是 Unix/Linux 系统上，traceroute 程序都会将第一个分组的 TTL 设置为 1,第二个设置为 2，第三个设置为 3 以及诸如此类。回想一个路由器将会递减一个接收分组的 TTL 值，当一个分组到达路由器时如果 TTL 等于 1，那么路由器将向源主机发送一个 ICMP 错误分组。在下面，我们将使用 Windows 本地 tracert 程序，一个更广泛使用的棒棒的 Windows traceroute 程序是 pingplotter。我们将在 IP lab 中使用 pingplotter 因为它提供了附加的功能。

跟着下面动手做:

* 打开终端
* 打开 Wireshake 分组嗅探，开始捕获分组。
* `traceroute hostname`
* 当 traceroute 程序终止时，停止分组捕获。

在这个实验的最后，你的终端窗口将显示如图 Figure 4，在这幅图像中，客户端 traceroute 程序在深圳，而目的地在美国，从这幅图像中我们可以看到每个 TTL 值，源主机程序发送 3 个探测分组，traceroute 展示了每个探测分组的往返时间，同时也展示了返回 ICMP TTL-exceeded 报文的路由器的 IP 地址 (也可能是路由器的名字)。

Figure 5 显示了 Wireshake 捕获每台路由器返回的 ICMP 分组的窗口。注意 ICMP 错误分组比 Ping ICMP 报文包含了更多的字段。

回答下列问题

* 5、你电脑的 IP 地址是什么？目的主机的 IP 地址是什么？
  * 10.10.5.168
  * 128.119.245.12

* 6、如果 ICMP 发送 UDP 报文段作为替代，探测分组 IP 首部的上层协议号还会是 01 吗？如果不是，那会是什么？
  * IP 首部上层协议号不是 01，而是 17

* 7、检查你截图中的 ICMP 回显分组，它是否和该实验前半部分的 ICMP Ping 请求分组有所不同？如果是，不同在何处？
  * 字段都相同

* 8、检查你截图中的 ICMP 错误分组，它是否比 ICMP 回显分组有更多的字段？这些字段都是什么？
  * 它比 ICMP 回显分组有更多的字段，它包含了原先 IP 数据报的首部，以及原先 IP 数据报载荷的前 8 个字节。因为 Unix/Linux 平台发送的是 UDP 分组，所以可以看到 ICMP TTL-exceeded 分组中包含了 UDP 的首部（刚好 8 个字节）

* 9、检查源主机接受到的最后三个 ICMP 分组，这些分组和 ICMP 错误分组有何不同？为什么会有这些不同？
  * 如图所示 
  * ![figure 2](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/images/wl_icmp_4.png)	
  * 前面的错误分组是 TTL-exceeded ICMP 分组，这代表 TTL 被减为 0 然后分组被路由器丢弃。但被目的主机接收到的分组 TTL 肯定不为 0，也就不会产生 ICMP TTL-exceeded 分组，那么如何判断是否到达了目的主机呢？在 Linux/Unix 平台上，traceroute 是用 UDP 来发送的，它会选择一个很大的端口号，以确保目的主机上所有应用程序都不会使用该端口号。比如下图
  ![figure 2](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/images/wl_icmp_5.png)	
  * 这样的话目的主机就会回传一个 Destination unreachable 的 ICMP 分组，源主机根据 ICMP 是目的地不可达还是超时 ( TTL被减为 1 ) 来判断是否到达目的地。
  * 在 Windows 平台上，traceroute 发送的是 ICMP 回显请求报文，因此当目的主机收到 TTL 为 1 的回显请求报文时，会给源主机回传 ICMP 回显 replay 报文。
  * 综上所述：根据 Unix/Linux 和 Windows 两种平台的不同，分别处理。如果是 Unix/Linux，最后三个分组是 ICMP Destination unreachable，如果是 Windows，最后三个分组是 ICMP echo replay。

* 10、通过 traceroute 的测量，是否有一条链路的延迟比其他链路显著的长？参考 figure 4 中的截图，是否有一条链路的延迟比其他链路显著的长？基于路由器的这些名称，你能否猜出最后一条链路所连接的两台路由器的位置？























  
  
     
 