### 1. The Basic HTTP GET/response interaction 

* Is your browser running HTTP version 1.0 or 1.1?  What version of HTTP is the server running? 

> 从上图 GET 报文可以看出，我的浏览器运行的 HTTP 协议版本是 1.1
> 
> 从上图 GET 响应报文可以看出，服务器运行的 HTTP 协议版本是 1.1

* What languages (if any) does your browser indicate that it can accept to the server? 

> 从上图 GET 报文可以看出，我的浏览器表示可以从服务器接受中文

* What is the IP address of your computer?  Of the gaia.cs.umass.edu server? 

> 打开 IP 数据包，可以看到源端 IP 和 目的 IP 信息
> 从上图可以看出，我的 IP 是 10.10.5.165，目的地 IP 是 128.119.245.12

* What is the status code returned from the server to your browser? 

> 从上图可以看出，响应报文的状态码是 200

* When was the HTML file that you are retrieving last modified at the server? 

> 从上图可以看出，这个 HTML 文件最后修改时间是 2018 年 9 月 12 日下午 13:59:01 （中国时区 +8 小时）

* How many bytes of content are being returned to your browser? 

> 从上图可以看出，返回了 128 个字节
