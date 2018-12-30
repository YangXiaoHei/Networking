* 1.Select one UDP packet from your trace. From this packet, determine how many fields there are in the UDP header. (You shouldn’t look in the textbook! Answer these questions directly from what you observe in the packet trace.) Name these fields. 

	> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/images/wl_udp.1.png)
	> 源端口，目的端口，长度，校验和

* 2.By consulting the displayed information in Wireshark’s packet content field for this packet, determine the length (in bytes) of each of the UDP header fields. 

	> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/images/wl_udp.2.png)
	>![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/images/wl_udp.3.png)
	![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/images/wl_udp.4.png)
	>![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/images/wl_udp.5.png)

* 3.The value in the Length field is the length of what? (You can consult the text for this answer). Verify your claim with your captured UDP packet. 

	> 是整个包的长度，包括首部字段
	> 
	> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/images/wl_udp.6.png)

* 4.What is the maximum number of bytes that can be included in a UDP payload? (Hint: the answer to this question can be determined by your answer to 2. above) 

	> 一个 UDP 包的载荷的长度信息是 2 个字节，因此最大的 UDP 包长为 `2^16 - 1 = 65535` 字节

* 5.What is the largest possible source port number? (Hint: see the hint in 4.) 

	> 2 字节，因此是 65535

* 6.What is the protocol number for UDP? Give your answer in both hexadecimal and decimal notation. To answer this question, you’ll need to look into the Protocol field of the IP datagram containing this UDP segment (see Figure 4.13 in the text, and the discussion of IP header fields). 

	> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/images/wl_udp.7.png)

* 7.Examine a pair of UDP packets in which your host sends the first UDP packet and the second UDP packet is a reply to this first UDP packet. (Hint: for a second packet to be sent in response to a first packet, the sender of the first packet should be the destination of the second packet).  Describe the relationship between the port numbers in the two packets. 

	> 呵呵，让我来一波 nslookup 的 DNS 查询不就搞到了吗？
	> 
	> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/images/wl_udp.8.png)
	> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/images/wl_udp.9.png)
	> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/images/wl_udp.10.png)