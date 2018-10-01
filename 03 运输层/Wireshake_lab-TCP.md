
### 2. A first look at the captured trace 

### 问题 1~2 是用 `tcp-ethereal-trace-1` 命名的 `Wireshake` 文件回答的

* 1.What is the IP address and TCP port number used by the client computer (source) that is transferring the file to gaia.cs.umass.edu?  To answer this question, it’s probably easiest to select an HTTP message and explore the details of the TCP packet used to carry this HTTP message, using the “details of the selected packet header window” (refer to Figure 2 in the “Getting Started with Wireshark” Lab if you’re uncertain about the Wireshark windows. 

	> ![](https://github.com/YangXiaoHei/Networking/blob/master/03%20运输层/images/wl_tcp.1.png)

* 2.What is the IP address of gaia.cs.umass.edu? On what port number is it sending and receiving TCP segments for this connection? 

	> 见上图


### If you have been able to create your own trace, answer the following question: (以下分组均用宏 `tcp && ((ip.src == 192.168.1.101 && ip.dst == 128.119.245.12) || (ip.src == 128.119.245.12 && ip.dst == 192.168.1.101))` 过滤)

* 3.What is the IP address and TCP port number used by your client computer (source) to transfer the file to gaia.cs.umass.edu?

	> ![](https://github.com/YangXiaoHei/Networking/blob/master/03%20运输层/images/wl_tcp.2.png) 

### 3. TCP Basics
 
#### Answer the following questions for the TCP segments: 

* 4.What is the sequence number of the TCP SYN segment that is used to initiate the TCP connection between the client computer and gaia.cs.umass.edu?  What is it in the segment that identifies the segment as a SYN segment? 

	> ![](https://github.com/YangXiaoHei/Networking/blob/master/03%20运输层/images/wl_tcp.3.png)  

* 5.What is the sequence number of the SYNACK segment sent by gaia.cs.umass.edu to the client computer in reply to the SYN?  What is the value of the Acknowledgement field in the SYNACK segment?  How did gaia.cs.umass.edu determine that value? What is it in the segment that identifies the segment as a SYNACK segment? 

	> ![](https://github.com/YangXiaoHei/Networking/blob/master/03%20运输层/images/wl_tcp.4.png)   

* 6.What is the sequence number of the TCP segment containing the HTTP POST command?  Note that in order to find the POST command, you’ll need to dig into the packet content field at the bottom of the Wireshark window, looking for a segment with a “POST” within its DATA field. 
	
	> 先找到包含 `HTTP POST` 方法名的 TCP 报文段如下图所示
	>
	> ![](https://github.com/YangXiaoHei/Networking/blob/master/03%20运输层/images/wl_tcp.5.png)
	>
	> 然后查看里面的详细字段，如下图所示
	> 
	> ![](https://github.com/YangXiaoHei/Networking/blob/master/03%20运输层/images/wl_tcp.6.png)  

* 7.Consider the TCP segment containing the HTTP POST as the first segment in the TCP connection. What are the sequence numbers of the first six segments in the TCP connection (including the segment containing the HTTP POST)?  At what time was each segment sent?  When was the ACK for each segment received? Given the difference between when each TCP segment was sent, and when its acknowledgement was received, what is the RTT value for each of the six segments?  What is the EstimatedRTT value (see Section 3.5.3, page 239 in text) after the receipt of each ACK?  ***Assume that the value of the EstimatedRTT is equal to the measured RTT for the first segment***, and then is computed using the EstimatedRTT equation on page 239 for all subsequent segments. 

	* **Note**: **Wireshark has a nice feature that allows you to plot the RTT for each of the TCP segments sent.  Select a TCP segment in the “listing of captured packets” window that is being sent from the client to the gaia.cs.umass.edu server.  Then select: Statistics->TCP Stream Graph->Round Trip Time Graph.** 

	* 1、What are the sequence numbers of the first six segments in the TCP connection (including the segment containing the HTTP POST)?
	
	> 由下图可以看出，该 TCP 连接的前 6 个发送端发出的包是 4、5、6、7、11、12。这 6 个包的序号是 1、615、754、2182、3610、5038
	>  ![](https://github.com/YangXiaoHei/Networking/blob/master/03%20运输层/images/wl_tcp.7.png)  
	
	* 2、At what time was each segment sent
	
		> 由上图可以看出，这六个包的发送时间分别是 
		>
		> xx.50.861068 --> 第 1 个数据包
		>
		> xx.50.862648 --> 第 2 个数据包
		>
		> xx.50.862812 --> 第 3 个数据包
		>
		> xx.50.862813 --> 第 4 个数据包
		>
		> xx.51.168450 --> 第 5 个数据包
		>
		> xx.51.168451 --> 第 6 个数据包
	
	* 3、When was the ACK for each segment received
		
		> ![](https://github.com/YangXiaoHei/Networking/blob/master/03%20运输层/images/wl_tcp.8.png) 
		>
		> 由上图可以看出，这 6 个包的 ACK 分别是 8、9、10、16、17、18
		> 这 6 个 ACK 包的接收时间分别为 
		>
		> xx.51.168351  --> 第 1 个数据包的确认
		>
		> xx.51.168355  --> 第 2 个数据包的确认
		>
		> xx.51.168355  --> 第 3 个数据包的确认
		>
		> xx.51.475423  --> 第 4 个数据包的确认
		>
		> xx.51.475428  --> 第 5 个数据包的确认
		>
		> xx.51.475429  --> 第 6 个数据包的确认
	
	* 4、what is the RTT value for each of the six segments?
		
	 	> 第 1 个包的 RTT：
	 	> 
	 	> `168351 us + (1000000 us - 861068 us) = 307283 us = 307.283 ms`
	 	> 
	 	> 第 2 个包的 RTT:
	 	> 
	 	> `168355 us + (1000000 us - 862648 us) = 305707 us = 305.707 ms`
	 	> 
	 	> 第 3 个包的 RTT: 
	 	> 
	 	> `168355 us + (1000000 us - 862812 us) = 305543 us = 305.543 ms`
	 	> 
	 	>第 4 个包的 RTT: 
	 	> 
	 	> `475423 us + (1000000 us - 862813 us) = 612610 us = 612.61 ms`
	 	>
	 	>第 5 个包的 RTT: 
	 	> 
	 	> `475428 us - 168450 us = 306978 us = 306.978 ms`
	 	>
	 	> 第 6 个包的 RTT: 
	 	> 
	 	> `475429 us - 168451 us = 306.978 ms`
	 	>
	 	
	* 5、What is the EstimatedRTT value (see Section 3.5.3, page 239 in text) after the receipt of each ACK ? ***Assume that the value of the EstimatedRTT is equal to the measured RTT for the first segment***
		
		> 
		> `EstimatedRTT = 0.875 x EstimatedRTT + 0.125 x SampleRTT` 
		>
		> 第 1 个包: 
		> 
		> `EstimatedRTT = SampleRTT = 307.283 ms`
		>
		> 第 2 个包：
		>
		> `EstimatedRTT = 0.875 x 307.283 + 0.125 x 305.707 = 307.040 ms`
		>
		> 第 3 个包：
		>
		> `EstimatedRTT = 0.875 x 307.04 + 0.125 x 305.543 = 306.853 ms`
		>
		> 第 4 个包：
		>
		> `EstimatedRTT = 0.875 x 306.853 + 0.125 x 612.61 = 345.073 ms`
		>
		> 第 5 个包：
		>
		> `EstimatedRTT = 0.875 x 345.073 + 0.125 x 306.978 = 340.311 ms`
		>
		> 第 6 个包：
		>
		> `EstimatedRTT = 0.875 x 340.311 + 0.125 x 306.987 ms = 336.1455 ms`

* 8.What is the length of each of the first six TCP segments?
	
	> 第 1 个包：614 bytes
	>
	> 第 2 个包：139 bytes
	>
	> 第 3 个包：1428 bytes
	>
	> 第 4 个包：1428 bytes
	>
	> 第 5 个包：1428 bytes
	>
	> 第 6 个包：1428 bytes

* 9.What is the minimum amount of available buffer space advertised at the received for the entire trace?  Does the lack of receiver buffer space ever throttle the sender? 

	> 是用筛选宏 `tcp && ((ip.src == 128.119.245.12 && ip.dst == 192.168.1.101))` 得到如下图所示的接收方的确认包，从中观察接收窗口字段可以看出接收缓存的变化趋势。
	>
	>![](https://github.com/YangXiaoHei/Networking/blob/master/03%20运输层/images/wl_tcp.9.png) 
	>
	>![](https://github.com/YangXiaoHei/Networking/blob/master/03%20运输层/images/wl_tcp.10.png) 
	>
	> 从中可以看到，接收方会扩充自己的缓存大小，从而使得发送方从始至终没有被接收方的接收缓存遏制发送速率
 
* 10.Are there any retransmitted segments in the trace file? What did you check for (in the trace) in order to answer this question? 

	>
	> 使用筛选宏 `tcp && ((ip.src == 192.168.1.101 && ip.dst == 128.119.245.12))` 得到发送方发送的所有数据包
	>
	> ![](https://github.com/YangXiaoHei/Networking/blob/master/03%20运输层/images/wl_tcp.11.png) 
	> ![](https://github.com/YangXiaoHei/Networking/blob/master/03%20运输层/images/wl_tcp.12.png) 
	> ![](https://github.com/YangXiaoHei/Networking/blob/master/03%20运输层/images/wl_tcp.13.png) 
	> 
	> 没有重传的分组，通过检查分组序号是否有重复的来推断。


* 11.How much data does the receiver typically acknowledge in an ACK?  Can you identify cases where the receiver is ACKing every other received segment (see Table 3.2 on page 247 in the text). 

	> ![](https://github.com/YangXiaoHei/Networking/blob/master/03%20运输层/images/wl_tcp.14.png) 
	> 从中可以看出，有两个冗余 ACK 产生，但是发送方并没有任何重传分组，因此可以推断出到达接收方的某两个分组发生了失序。
	
* 12.What is the throughput (bytes transferred per unit time) for the TCP connection? Explain how you calculated this value. 
	
	> 总吞吐量可以由总传输字节除以总耗时来计算
	>
	> 最后一个 ACK 的确认号为 149339，因此从发送方接收到的总字节数为 149338
	>
	> 第一个数据包发送的时间为 13:54:50.861068
	> 
	> 总耗时为最后一个 ACK 的时间 13:54:52.704054
	>
	> 
	>
	> 因此总耗时为 (1000000 us - 861068 us) + 1000000 us + 704054 us = 1842.986 ms
	>
	>
	> 吞吐量为 1499339 bytes / 1.843 s = 813531.7 bps = 813.5 kbps
	> 

*  **The TCP segments in the tcp-ethereal-trace-1 trace file are all less that 1460 bytes. This is because the computer on which the trace was gathered has an Ethernet card that limits the length of the maximum IP packet to 1500 bytes (40 bytes of TCP/IP header data and 1460 bytes of TCP payload). This 1500 byte value is the standard maximum length allowed by Ethernet. If your trace indicates a TCP length greater than 1500 bytes, and your computer is using an Ethernet connection, then Wireshark is reporting the wrong TCP segment length; it will likely also show only one large TCP segment rather than multiple smaller segments. Your computer is indeed probably sending multiple smaller segments, as indicated by the ACKs it receives.  This inconsistency in reported segment lengths is due to the interaction between the Ethernet driver and the Wireshark software. We recommend that if you have this inconsistency, that you perform this lab using the provided trace file.**

### 4. TCP congestion control in action

* 13.Use the Time-Sequence-Graph(Stevens) plotting tool to view the sequence number versus time plot of segments being sent from the client to the gaia.cs.umass.edu server.  Can you identify where TCP’s slowstart phase begins and ends, and where congestion avoidance takes over?  Comment on ways in which the measured data differs from the idealized behavior of TCP that we’ve studied in the text.

 
* 14.Answer each of two questions above for the trace that you have gathered when you transferred a file from your computer to gaia.cs.umass.edu


