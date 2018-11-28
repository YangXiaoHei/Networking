在这个实验中，我们将观察 IP 协议，并专注在 IP 数据报上。我们将为此分析一个由 traceroute 程序发送和接收的 IP 数据报轨迹（traceroute 程序本身的细节在 Wireshake ICMP lab 中被仔细地探讨）。我们将调查 IP 数据报中不同的域，并且学习 IP 分片的细节。

在开始这个实验前，你可能想去复习 1.4.3 节和 RFC 2151 的 3.4 节，借此来更新你自己关于 traceroute 程序运作的认识。你也可能会想要阅读课本 4.4 节，并把 RFC 791 放在手边以供随时查阅。这是为了准备接下来对 IP 协议的讨论。

 * 1. 从 traceroute 的执行中捕获分组
 
   * 为了在这个实验中产生一个 IP 数据报轨迹，我们将使用 traceroute 程序向一些目的地发送不同大小的数据报。回想 traceroute 的执行会先发送一个或多个 TTL 置为 1 的数据报；然后向相同的目的地发送一系列一或多个将 TTL 置为 2 的数据报，然后向相同目的地发送一系列一个或多个将 TTL 域置为 3 的数据报，等等。回想一个路由器一定会对每个收到的数据报执行 TTL 减 1 操作。如果 TTL 减为 0，路由器向主机返回一个 ICMP 类型为 11 的报文（TTL-exceeded）。一个 TTL 为 1 的数据报（被运行 traceroute 的主机所发送）将会使距发送者一跳的路由器返回一个 ICMP TTL-exceeded 报文。TTL 被设置为 2 的数据报将使距离发送者两跳的路由器给自己返回一个 ICMP TTL-exceeded 报文，等等。借助这个原理，运行 traceroute 的主机将观察 TTL-exceeded ICMP 报文的数据报中的 IP 地址，获知自己和目的地之间路由器的身份特性。
   
我们将运行 traceroute 并让它发送不同长度的数据报

* Linux / Unix / MacOs. 通过 Unix/MacOS traceroute 命令，发往指定目的地的 UDP 报文段的尺寸可以被显示设置。尺寸的值紧跟在目的地名字后面。比如，向 gaia.cs.umass.edu 发送 2000 字节的 traceroute 数据报将写成 : `$traceroute gaia.cs.umass.edu 2000`
   
* 跟着下面动手做:
  * 启动 Wireshake 并开始捕获分组(Capture->Start) 之后在 Wireshake Packet Options screen 点击 OK（我们不需要在这选择任何选项）。
     
  * 如果你使用 Mac 或者 Unix 平台，输入三个 traceroute 命令。一个使用 50 字节长度，一个使用 2000 字节，一个使用 3500 字节。
     
  * 停止 Wireshake 分组捕获。
     
  * 如果你无法在真实的网络环境中运行 Wireshake，你可以下载分组轨迹文件，这些轨迹文件在作者的 Windows 电脑上产生。当你探索下面的问题时，下载并使用这些轨迹文件同样有益处，即使你可以捕获并使用自己的分组轨迹。
  
* 2. 观察被捕获的分组轨迹
  * 在你的轨迹中，你应该可以看到一系列从你的主机发送的 ICMP 回显请求（在 Windows 平台）或者 UDP 报文段（在 Unix 平台），以及中间路由器返回的 TTL-exceeded ICMP 报文。在下列问题中，我们将假设你使用 Windows 机器。对应的 Unix 版本的问题应该很清晰。
      
  * 下列回答使用的是 shell 命令 `traceroute gaia.cs.umass.edu 2000`，wifi 使用的是公司的香港专线。
   
  * 1、你的电脑的 IP 地址是什么？
    * ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_ip_1.png)
    * 从上图可以看出，向目的地发送一系列 UDP 报文时，源 IP 地址是 10.10.5.168，这就是我电脑的 IP 地址，一个内网地址。
      
  * 2、在 IP 分组首部，上层协议的值是什么？
    * ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_ip_2.png)
    * 从图中可以看出，上层协议号 17，运输层协议是 UDP。
      
  * 3、IP 首部总共多少字节？IP 数据报的载荷有多少字节？你回答载荷长度的依据是什么。
    * ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_ip_2.png)
    * IP 首部总共 20 字节，载荷为 1480 字节，依据是总长度为 1500 字节，1500 - 20 = 1480 字节。
      
  * 4、这个 IP 数据报被分片了吗？你回答是或否的依据是什么？
    * ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_ip_3.png)
    * 被分片了，因为 2000 字节的数据报，总大小只有 1500 字节。而且 flag 字段不为 0，这代表此 IP 数据报后面还有其他分片。
      
接下来，根据 IP 源地址来排序分组轨迹。通过点击 Source 列来排序。选择第一个被你电脑发送的 ICMP 回显请求报文。

* 5、 在一系列你的电脑所发送的 ICMP 回显报文中，从一个到接下来发送的另一个，IP 数据报的哪个域总是在改变？
   * TTL 递增 1，所以 checksum 一定会改变。因为不同的 ip 数据报，所以 identification 一定会改变。
	
* 6、哪些域保持不变？这些域中的哪一个必须保持不变？那个域一定会改变？为什么？
   * 保持不变的域：版本号、首部长度、服务类型、源和目的 IP 地址，上层协议号。
   * 必须保持不变的域：版本号、首部长度、服务类型、源和目的 IP 地址，上层协议号。
   * 一定会改变的域：TTL、校验和  
	  
* 7、描述你观察到的在 IP 数据报中的 Identification 域的值的样式 
	* 属于相同 IP 数据报的同一组分片的 Identification 都相同， 不同 IP 数据报的 Identification 不同。
	* ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_ip_4.png)
	* ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_ip_5.png)
	
	||第 1 bit| 第 2 bit|第 3 bit|第 4 ~ 第 16 总共 13 bit |
	|:---:|:---:|:---:|:---:|:---:|
	||Reserved bit|Don't fragment|More fragment|fragment offset|

接下来(仍然使用已经根据源 IP 地址排序的分组轨迹) 寻找一系列由第一跳路由器返回给你主机的 TTL-exceeded ICMP 报文。
 
  * 8、TTL 域和 Identification 域的值是什么？
   * 如下图所示:
   * ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_ip_6.png)
   * TTL 是 255、Identification 是 43642
	
  * 9、上面问题中的两个字段，是否在第一跳路由器返回的所有 TTL-exceeded ICMP 报文中都没有变化？为什么？
  * Identification 一直在变化，因为 Identification 标志着不同的 IP 数据报，如果 Identification 相同，那说明相同的 IP 数据报都是一个更大的 IP 数据报的分片。
  * TTL 都没有变化，都是 255，因为第一跳路由器直接返回给你的电脑，因此没有被中间路由器递减 TTL 字段。
	
* 分片
  * 根据 Time 列对分组轨迹进行排序。
  
* 10、找到在你把分组尺寸修改到 2000 字节后，你电脑发送的第一个 ICMP 回显请求报文。这个报文是否分片成了多个 IP 数据报？[注意：如果你发现你的分组没有被分片，你应该下载 zip 文件 `http://gaia.cs.umass.edu/wireshark-labs/wireshark-traces.zip` 并解压缩得到文件名为 ip-ethereal-trace-1 的分组轨迹文件，如果你的电脑有一个以太网接口，超过 2000 字节的分组应该会导致分片]
    * 是的，被分片
  
  * 11、打印出被分片的 IP 数据报的第一个分片。在 IP 首部中什么信息指示了数据报被分片？什么信息指示了这是相较于后续分片的第一个分片？这个 IP 数据报有多长？
    * 在 flag 字段中第 3 bit 被设置为 1，这代表有更多的分片。
  
  * 12、打印这个被分片 IP 数据报的第二个分片。首部中什么信息指示出这不是第一个分片？是否有更多的分片？你如何辨别出这一点？
    * offset 域不为 0 指出了这不是第一个分片，如果此时 flag 被置为 1 表示着后面还有更多的分片，如果为 0，说明这就是最后一个分片。
  
  * 13、在第一个 IP 分片和第二个分片中，IP 首部中什么字段发生了变化？
    * 改变的字段：校验和，是否有更多分片的 flag，片偏移
   
* 现在，把你的主机发送的 ICMP 回显报文的尺寸修改到 3500 字节，然后找出第一个发送的报文段（不是分片）。
 
  * 12、原始的数据报产生了多少个分片？
     * 从下图可以看出，产生了 3 个分片:
     * ![](https://github.com/YangXiaoHei/Networking/blob/master/04%20网络层/images/wl_ip_7.png)
   
  * 在这些分片中，IP 首部中的什么字段在变化？
    * 校验和，是否有更多分片的 flag，片偏移，总长度

      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
      
