## What to hand in 
The goal of this first lab was primarily to introduce you to Wireshark. The following questions will demonstrate that you’ve been able to get Wireshark up and running, and have explored some of its capabilities. Answer the following questions, based on your Wireshark experimentation: 

--

#### 1、 List 3 different protocols that appear in the protocol column in the unfiltered packet-listing window in step 7 above. 

### HTTP

![](https://github.com/YangXiaoHei/Networking/blob/master/01%20计算机网络和因特网/images/wireshake_http.png)

### DNS

![](https://github.com/YangXiaoHei/Networking/blob/master/01%20计算机网络和因特网/images/wireshake_dns.png)

### ICMP

![](https://github.com/YangXiaoHei/Networking/blob/master/01%20计算机网络和因特网/images/wireshake_icmp.png)

### TCP

![](https://github.com/YangXiaoHei/Networking/blob/master/01%20计算机网络和因特网/images/wireshake_tcp.png)


#### 2、How long did it take from when the HTTP GET message was sent until the HTTP OK reply was received? (By default, the value of the Time column in the packet-listing window is the amount of time, in seconds, since Wireshark tracing began.  To display the Time field in time-of-day format, select the Wireshark View pull down menu, then select Time Display Format, then select Time-of-day.) 

![](https://github.com/YangXiaoHei/Networking/blob/master/01%20计算机网络和因特网/images/wireshake_time.png)

`T = 740659 us - 647275 us = 93384 us = 0.09 ms`

#### 3、What is the Internet address of the gaia.cs.umass.edu (also known as wwwnet.cs.umass.edu)?  What is the Internet address of your computer? 

通过 Source 栏和 Destination 栏查看

#### 4、Print the two HTTP messages (GET and OK) referred to in question 2 above. To do so, select Print from the Wireshark File command menu, and select the “Selected Packet Only” and “Print as displayed” radial buttons, and then click OK. 

 1、选中分组
 2、文件
 3、导出分组解析结果 -> JSON、C 语言数组 ...
 4、仅到处选中/或者标记分组
 5、命名
