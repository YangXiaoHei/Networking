在这个实验中，我们将探寻以太网协议和 ARP 协议。在开始这个实验前，你可能想重温课本 5.4.1 节和 5.4.2 节，以及关于 ARP 协议细节的 RFC 826。ARP 被一台 IP 设备用来确定 MAC 地址众所周知的远程接口的 IP 地址。

### 1. 捕获和分析以太网帧

  * 通过下列步骤捕获一系列的以太网帧用于我们的学习。
  * 首先，确保浏览器的缓存是空的，在火狐浏览器中，选择 工具-清空最近历史并检查缓存选框。对于 IE 浏览器，选择 工具-因特网选项-删除文件。开启 Wireshake 分组捕获。
  * 在浏览器中输入 "http://gaia.cs.umass.edu/wireshark-labs/HTTP-ethereal-lab-file3.html"，你的浏览器应该展示相当冗长的 "US Bill of Rights"。
  * 停止 Wireshake 分组捕获，首先，找到从你的电脑发出的 HTTP GET 报文分组序号（在 Wireshake 窗口的最左一列), 再找出从 gaia.cs.umass.edu 获得的 HTTP 响应分组的序号。你应该看到如下所示的窗口 
  * 因为这个实验是关于以太网和 ARP 的，我们不关心 IP 或者更高层的协议，所以让我们改变 Wireshake "listing of capture packets" 窗口以便它只显示 IP 以下协议的信息。为了让 Wireshake 做到这件事，选择 分析-启用协议，然后去除 IP 的勾选然后选择 OK，你应该看到 Wireshake 窗口如下所示。
  
为了回答下列问题，你应该观察分组的细节和分组窗口的内容。

选择包含 HTTP GET 报文的以太网帧 （回忆 HTTP GET 报文被携带在 TCP 报文段中，TCP 报文段被携带在 IP  数据报中，IP 数据报被携带在以太网帧中，如果你感到有一丝迷惑，重读课本 1.5.2 节）在分组详情窗口中展开以太网二级信息，注意到以太网帧的内容（包括头部和负载在内）已经被展示在分组内容窗口中。

回答下列问题，基于包含 HTTP GET 报文的以太网帧的内容。

 * 1、你电脑的 48 bit 以太网地址是什么？
   * a4:5e:60:ef:99:4d
   
 * 2、48 bit 的目的以太网地址是什么？这个以太网地址是 gaia.cs.umass.edu 的吗？这个以太网地址是什么设备的？
   * 3c:8c:40:bd:4d:fc
   * 这是网关路由器的 MAC 地址。
   
 * 3、帧的类型字段中值是什么？用两字节 16 进制表示？这对应于上层的什么协议？
   * 类型字段的值是 0x8000，对应于上层的 IPv4 协议。
   
 * 4、从以太网帧的开始到 ASCII 码的 "G" 之间有多少字节？
   * 66 字节

之后，回答下列问题，基于包含 HTTP 响应报文第一个字节的以太网帧内容。

 * 5、源 MAC 地址是什么？这是你的电脑的 MAC 地址吗？或者是 gaia.cs.umass.edu 的 MAC 地址吗？这是什么设备的 MAC 地址？
   * 源 MAC 地址是 3c:8c:40:bd:4d:fc
   * 不是 gaia.cs.umass.edu 的 MAC 地址。
   * 这是网关路由器的 MAC 地址。
   
 * 6、这个以太网帧的目的 MAC 地址是什么？这是你的电脑的 MAC 地址吗？
   * 目的 MAC 地址是 a4:5e:60:ef:99:4d，这是我的电脑的 MAC 地址。
   
 * 7、该以太网帧中类型字段的值是什么？用两个字节的 16 进制数表示，这对应于上层的什么协议？
   * 类型字段的值为 0x8000，对应于 IPv4 协议。
  
 * 8、从以太网帧一开始到 ASCII 码的 "O" 之间有多少个字节？ 
   * 总共有 79 个字节
  

* 2、地址解析协议 (ARP)

在这一节中，我们观察 ARP 协议的交互行为，我们强烈建议你在继续之前，重读课本 6.4.1 节的内容。

### 2、ARP 缓存

会想 ARP 协议在你的电脑中维护一个 IP 到以太网地址的缓存映射对。`arp` 命令用来观察和操纵这份缓存的内容，因为 arp 命令和 ARP 协议有相同的名字，所以造成混淆是可以理解的。但是记住它们其实是不同的，`arp` 命令用来观察和操纵 ARP 缓存内容，而 ARP 协议是定义了消息收发的格式，并且定义了消息传输和回执时采取的动作。

让我们看一看你电脑上的 ARP 缓存的内容:

* 在 Linux/Unix/MacOS 上, 可执行的 `arp` 命令可以在很多地方，流行的所在是 `/sbin/arp` (对于 linux 来说)，以及 `/usr/etc/arp` (对于其他 Unix 变种)。

Windows 的 `arp` 命令不带任何参数时将会展示你电脑上的 ARP 缓存内容，运行 `arp` 命令。

* 9、写下你电脑的 ARP 缓存的内容。每一列值的含义是什么？

~~~C
? (10.10.5.1) at 3c:8c:40:bd:4d:fc on en0 ifscope [ethernet]
? (10.10.5.76) at a4:5e:60:31:1a:13 on en0 ifscope [ethernet]
? (10.10.5.115) at 30:b4:9e:f:c6:a5 on en0 ifscope [ethernet]
? (10.10.5.142) at 88:19:8:17:59:c1 on en0 ifscope [ethernet]
? (10.10.5.143) at 3c:2e:f9:80:13:c5 on en0 ifscope [ethernet]
? (10.10.5.146) at 20:ab:37:1c:c3:b9 on en0 ifscope [ethernet]
? (10.10.5.150) at 30:b4:9e:d6:c0:82 on en0 ifscope [ethernet]
? (10.10.5.165) at 94:e9:6a:85:8a:5a on en0 ifscope [ethernet]
? (10.10.5.180) at e4:9a:79:db:4f:b8 on en0 ifscope [ethernet]
? (10.10.5.182) at 60:f8:1d:b7:f4:48 on en0 ifscope [ethernet]
? (224.0.0.251) at 1:0:5e:0:0:fb on en0 ifscope permanent [ethernet]
? (239.255.255.250) at 1:0:5e:7f:ff:fa on en0 ifscope permanent [ethernet]
~~~

为了观察你电脑收发 ARP 报文，我们需要清理 ARP 缓存，因为如果不清除的话你的电脑可能会发现需要的 IP-MAC 映射对从而就不用发送 ARP 查询报文。 

  * Linux/Unix/MacOS 使用 `arp -d *` 命令将会清除你的 ARP 缓存，为了执行这条命令你需要根用户权限，如果你没有根用户权限并且无法在 Windows 机器上运行 Wireshake，你可以忽略分组收集部分并直接使用我们提供的轨迹文件。

在交互中观察 ARP

跟着下面动手做：

  * 清除你的 ARP 缓存 `sudo arp -d -i en0 -a`，正如上面所描述那样。
  * 接下来，确保你的浏览器缓存是空的
  * 开启 Wireshake 分组捕获
  * 在浏览器中输入下列 URL "http://gaia.cs.umass.edu/wireshark-labs/HTTP-wireshark-lab-file3.html"，你的浏览器应该再次展示相当冗长的 "US Bill of Rights"
  * 停止 Wireshake 分组捕获，再一次，我们不关心 IP 或更高层的协议，所以让我们改变 Wireshake "listing of capture packets" 窗口以便它只显示 IP 以下协议的信息。为了让 Wireshake 做到这件事，选择 分析-启用协议，然后去除 IP 的勾选然后选择 OK，你应该看到 Wireshake 窗口如下所示。
  * 在上面的例子中，最开始的两个帧包含了 ARP 报文（第 6 个报文亦然），这份截图来自注脚 1 所提供的轨迹分组文件。

回答下列问题：

	![](https://github.com/YangXiaoHei/Networking/blob/master/05%20链路层/images/wl_arp_1.png)
	
  * 10、包含 ARP 请求报文的以太网帧的源和目的地址是什么？用 16 进制表示。
    * 由上图可以看出，源和目的地址分别是 a4:5e:60:ef:99:4d 和 ff-ff-ff-ff-ff-ff
    
  * 11、以太网帧的类型字段的值是什么？用 16 进制表示，相对应的上层协议是什么？
    * 类型字段是 0x0806，对应的上层协议就是 ARP
    
  * 12、从 [arp specification](ftp://ftp.rfc-editor.org/in-notes/std/std37.txt)下载 ARP 说明书，同样的可以在站点 [detail discussion of ARP](http://www.erg.abdn.ac.uk/users/gorry/course/inet-pages/arp.html) 发现一份可读的关于 ARP 的讨论。 
    * a、从以太网帧开始到 ARP opcode 域之间有多少字节？
      * 20 字节
      
    * b、对于有效载荷是 ARP 请求报文的以太网帧，opcode 字段的值是什么？
      * ARP 请求报文的 opcode 是 1
      
    * c、ARP 报文是否包含发送者的 IP 地址？
      * 从上图可以看出，ARP 报文包含了发送者的 IP 和 MAC，还有接收者的 IP，而接收者的 MAC 填为 00-00-00-00-00-00
      
    * d、ARP 请求报文的何处位置展示了 “问题查询” -- 被查询的 IP 地址相对应的机器的 MAC 地址。
      * 首先由 ARP 报文的 opcode 字段表明这是一个 ARP 请求报文，同时 Target MAC address 填为了 00-00-00-00-00-00 表明这是一个查询 MAC 地址的 ARP 请求报文。
    
  * 13、现在找出 ARP 查询对应的 ARP 响应报文。
    * a、从以太网帧开始到 ARP opcode 域之间有多少字节？
      * ![](https://github.com/YangXiaoHei/Networking/blob/master/05%20链路层/images/wl_arp_2.png)
      * 从上图可以看出，有 20 字节。
      
    * b、对于有效载荷是 ARP 响应报文的以太网帧，opcode 字段的值是什么？
      * 从上图可以看出，对于 ARP 响应报文的以太网帧，opcode 字段的值是 2
      
    * c、ARP 响应报文的何处位置展示了 “查询结果” -- 被查询的 IP 地址相对应的机器的 MAC 地址。
      * 首先由 opcode 为 2 指明这是一个 ARP 响应报文，同时从 Target MAC address 和 Sender MAC address 都不为 00-00-00-00-00-00 可以看出这是一个查询结果报文。
    
  * 14、携带 ARP 响应报文的以太网帧的源和目的 MAC 地址是什么？
    * 源和目的分别是 3c:8c:40:bd:4d:fc 和 a4:5e:60:ef:99:4d
    
  * 15、打开轨迹文件 [ethernet-ethereal-trace-1](http://gaia.cs.umass.edu/wireshark-labs/wireshark-traces.zip)，轨迹中的第一和第二个 ARP 分组对应于一个运行着 Wireshake 电脑发出的 ARP 请求，和一台运行着 Wireshake 并且有 ARP 所查询 MAC 地址的电脑所发出的 ARP 响应。但是在该网络中还有另一台电脑，正如分组 6 所展示的那样 -- 另一个 ARP 请求，为什么在分组轨迹中没有该 ARP 请求的 ARP 响应分组？
   * 当我发出 ARP 请求时，ARP 请求报文经过网卡接口所以被 Wireshake 捕获，当 ARP 响应回来时，响应报文经过网卡接口所以被 Wireshake 捕获，而其他电脑的 ARP 查询因为是广播，所以也被我的电脑接收，但是因为适配器检查到自己的 IP 地址并不是目标查询的 IP 地址，所以直接把该 ARP 丢弃，不会发送响应回去。所以在 Wireshake 的轨迹文件中没有该查询的响应分组。

### 额外的关卡
* EX-1 `arp` 命令
  
  `arp -s InetAddr EtherAddr`  这条命令允许你手动在 ARP 缓存中添加一条 IP-MAC 映射，如果你手动添加了一条正确 IP 地址但是错误 MAC 地址的映射会发生什么呢？
  
   * 不会发生任何事情，因为该错误的 MAC 地址不会被任何适配器所接收，所以它会在 ARP 缓存过期后被淘汰。
  
* EX-2 在你的 ARP 缓存被移除前，该 ARP 缓存默认会被保存多久呢？你可以通过经验来判断（通过不停地观察缓存内容）或者通过参阅你所使用系统的说明文档。