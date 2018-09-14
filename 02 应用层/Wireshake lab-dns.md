### 1. nslookup

* Run nslookup to obtain the IP address of a Web server in Asia. What is the IP address of that server? 

> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wldns1.png)

* Run nslookup to determine the authoritative DNS servers for a university in Europe. 

> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wldns2.png)

* Run nslookup so that one of the DNS servers obtained in Question 2 is queried for the mail servers for mail.yahoo.com What is its IP address? 

> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wldns3.png)

### 2. ipconfig 

### 3. Tracing DNS with Wireshark

* Locate the DNS query and response messages. Are then sent over UDP or TCP? 

>
> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wldns4.png)
> 从上图可以看出，是一个 UDP 的包

* What is the destination port for the DNS query message? What is the source port of DNS response message? 

>
>![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wldns5.png)
>
>![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wldns5.png)
> 
> DNS 请求报文的目的端口是 53，DNS 响应报文的源端口是 53

* To what IP address is the DNS query message sent? Use ipconfig to determine the IP address of your local DNS server. Are these two IP addresses the same? 

> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wldns6.png)
> 
> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wldns7.png)

* Examine the DNS query message. What “Type” of DNS query is it? Does the query message contain any “answers”? 

>
>![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wldns8.png)
> A 记录，DNS 请求报文不包含 answers

* Examine the DNS response message. How many “answers” are provided? What do each of these answers contain? 

>
> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wldns9.png)
> 两条 A 记录，每条都提供一个 IP 地址