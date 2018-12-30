### 1. nslookup

* Run nslookup to obtain the IP address of a Web server in Asia. What is the IP address of that server? 

> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns1.png)

* Run nslookup to determine the authoritative DNS servers for a university in Europe. 
> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns2.png)

* Run nslookup so that one of the DNS servers obtained in Question 2 is queried for the mail servers for mail.yahoo.com What is its IP address? 

> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns3.png)

### 2. ipconfig 

### 3. Tracing DNS with Wireshark

* Locate the DNS query and response messages. Are then sent over UDP or TCP? 

>
> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns4.png)
> 从上图可以看出，是一个 UDP 的包

* What is the destination port for the DNS query message? What is the source port of DNS response message? 

>
>![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns5.png)
>
>![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns5.png)
> 
> DNS 请求报文的目的端口是 53，DNS 响应报文的源端口是 53

* To what IP address is the DNS query message sent? Use ipconfig to determine the IP address of your local DNS server. Are these two IP addresses the same? 

> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns6.png)
> 
> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns7.png)

* Examine the DNS query message. What “Type” of DNS query is it? Does the query message contain any “answers”? 

>
>![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns8.png)
> A 记录，DNS 请求报文不包含 answers

* Examine the DNS response message. How many “answers” are provided? What do each of these answers contain? 

>
> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns9.png)
> 两条 A 记录，每条都提供一个 IP 地址

* Consider the subsequent TCP SYN packet sent by your host. Does the destination  IP address of the SYN packet correspond to any of the IP addresses provided in the DNS response message? 

>
>![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns10.png)
>![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns11.png)
>
> 从上图可以看出，浏览器与服务器建立 TCP 连接所发送的握手报文的目的 IP 地址就是从 DNS 响应报文中得到的 IP 地址。

* This web page contains images. Before retrieving each image, does your host issue new DNS queries? 

>
>![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns12.png)
> 从上图可以看出，没有发出新的 DNS 请求，查看其他几个获取图片的 GET 请求分析，这是因为图片资源和网页资源都在同一台主机上，所以直接使用最先请求到的 IP 地址即可。

~~~
Now let’s play with nslookup. 
* Start packet capture. 
* Do an nslookup on www.mit.edu 
* Stop packet capture. 
~~~

* What is the destination port for the DNS query message? What is the source port of DNS response message? 

>
> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns13.png)
> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns13.1.png)
> 从上图可以看出，DNS 请求报文的目的端口是 53，DNS 回答报文的源端口是 53

* To what IP address is the DNS query message sent? Is this the IP address of your default local DNS server? 

>
>![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns7.png)
>![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns14.png)
> DNS 请求报文发往 IP 地址 8.8.4.4，这是我的默认本地 DNS 服务器

* Examine the DNS query message. What “Type” of DNS query is it? Does the query message contain any “answers”? 

> 
> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns15.png)
> A 类型；不包含 answers

* Examine the DNS response message. How many “answers” are provided? What do each of these answers contain? 

> 
> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns16.png)
> 3 条 answers，两条 CNAME 记录，一条 A 记录

* Provide a screenshot. 

> 见上述问题下截图

~~~
Now repeat the previous experiment, but instead issue the command: 
nslookup –type=NS mit.edu 
~~~

* To what IP address is the DNS query message sent? Is this the IP address of your default local DNS server? 

> 
> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns7.png)
> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns17.png)
> DNS 请求报文发往 IP 地址 8.8.4.4，这是我的默认本地 DNS 服务器

* Examine the DNS query message. What “Type” of DNS query is it? Does the query message contain any “answers”? 

>
> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns18.png)
> NS 类型，即请求知道如何获取该域名主机 IP 地址的权威 DNS 服务器

* Examine the DNS response message. What MIT nameservers does the response message provide? Does this response message also provide the IP addresses of the MIT namesers? 

> 
> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns19.png)
> 返回了 8 个 MIT 的权威 DNS 服务器域名，但没有返回各自的 IP 地址

* Provide a screenshot. 

> 见上述问题下截图

~~~
Now repeat the previous experiment, but instead issue the command: 
nslookup www.aiit.or.kr bitsy.mit.edu 
~~~

* To what IP address is the DNS query message sent? Is this the IP address of your default local DNS server? If not, what does the IP address correspond to? 

不是向我的本地 DNS 服务器发送，而是向域名为 bitsy.mit.edu 的 IP 地址为 18.72.0.3 的主机发送

~~~
➜  ~ nslookup bitsy.mit.edu
Server:		8.8.4.4
Address:	8.8.4.4#53

Non-authoritative answer:
Name:	bitsy.mit.edu
Address: 18.72.0.3
~~~

* Examine the DNS query message. What “Type” of DNS query is it? Does the query message contain any “answers”? 

> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns20.png)
> A 记录，不包含任何 answers

* Examine the DNS response message. How many “answers” are provided? What does each of these answers contain? 

> 
> ![](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/images/wldns21.png)
> anwser 包含一条 A 记录

* Provide a screenshot. 


