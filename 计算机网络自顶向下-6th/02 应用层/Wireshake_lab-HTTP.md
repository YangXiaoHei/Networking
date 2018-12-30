### 1. The Basic HTTP GET/response interaction 

* Is your browser running HTTP version 1.0 or 1.1?  What version of HTTP is the server running? 

> 
> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl1.png)
> 从上图 GET 报文可以看出，我的浏览器运行的 HTTP 协议版本是 1.1
> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl2.png)
> 从上图 GET 响应报文可以看出，服务器运行的 HTTP 协议版本是 1.1

* What languages (if any) does your browser indicate that it can accept to the server? 

> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl3.png)
> 从上图 GET 报文可以看出，我的浏览器表示可以从服务器接受中文

* What is the IP address of your computer?  Of the gaia.cs.umass.edu server? 

> 打开 IP 数据包，可以看到源端 IP 和 目的 IP 信息
> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl4.png)
> 从上图可以看出，我的 IP 是 10.10.5.165，目的地 IP 是 128.119.245.12

* What is the status code returned from the server to your browser? 

> 
> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl5.png)
> 从上图可以看出，响应报文的状态码是 200

* When was the HTML file that you are retrieving last modified at the server? 

>
> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl6.png)
> 从上图可以看出，这个 HTML 文件最后修改时间是 2018 年 9 月 12 日下午 13:59:01 （中国时区 +8 小时）

* How many bytes of content are being returned to your browser? 

>
> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl7.png)
> 从上图可以看出，返回了 128 个字节

### 2.The HTTP CONDITIONAL GET/response interaction 

* Inspect the contents of the first HTTP GET request from your browser to the server.  Do you see an “IF-MODIFIED-SINCE” line in the HTTP GET? 

>
> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl8.png)
> 从上图可以看出，第一个 GET 请求报文中没有 "IF-MODIFIED-SINCE" 首部行

* Inspect the contents of the server response. Did the server explicitly return the contents of the file?  How can you tell? 

>
>![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl9.png)
> 从上图可以看出，GET 响应报文显示返回了文件内容

* Now inspect the contents of the second HTTP GET request from your browser to the server.  Do you see an “IF-MODIFIED-SINCE:” line in the HTTP GET? If so, what information follows the “IF-MODIFIED-SINCE:” header? 

>
>![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl10.png)
> 从上图可以看出，第二个 GET 请求报文中包含了 "IF-MODIFIED-SINCE" 首部行，该首部行包含了上次响应报文中缓存的服务器中该文件的最后修改时间。

* What is the HTTP status code and phrase returned from the server in response to this second HTTP GET?  Did the server explicitly return the contents of the file? Explain. 

>
>![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl11.png)
> 从上图可以看出，状态码是 304，短语是 "Not Modified"，该请求中未包含文件的内容。因为当前浏览器缓存的文件在服务器中还未被修改，已经是最新的，不必再次返回一模一样的文件。

### 3. Retrieving Long Documents 

* How many HTTP GET request messages did your browser send? Which packet number in the trace contains the GET message for the Bill or Rights? 

>
> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl12.png)
> 从上图可以看出，浏览器发送了一个 GET 请求报文。该报文的编号是 8

* Which packet number in the trace contains the status code and phrase associated with the response to the HTTP GET request? 

>
>![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl13.png)
> 从上图可以看出，包的编号是 10

* What is the status code and phrase in the response? 

> 200 OK

* How many data-containing TCP segments were needed to carry the single HTTP response and the text of the Bill of Rights? 

>
>![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl14.png)
> 从上图可以看出，需要 4 个 TCP 报文段才可以携带一个单独的 HTTP 响应报文体和报文 body

### 4. HTML Documents with Embedded Objects

* How many HTTP GET request messages did your browser send?  To which Internet addresses were these GET requests sent? 

>
> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl15.png)
> 从上图可以看出，总共发了 3 个 GET 请求报文
> 
> 第一个 GET 发到 128.119.245.12，其他两个报文分别发到 165.193.123.218 和 134.241.6.82

* Can you tell whether your browser downloaded the two images serially, or whether they were downloaded from the two web sites in parallel?  Explain. 

>
> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl16.png)
> 并行下载，从上图可以看出，在一张图片下载完成之前，已经发出了第二个 GET 请求报文。

### 5. HTTP Authentication

* What is the server’s response (status code and phrase) in response to the initial HTTP GET message from your browser? 

> 
> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl17.png)
> 从上图可以看出，401 Authorization Required

* When your browser’s sends the HTTP GET message for the second time, what new field is included in the HTTP GET message? 

> 
> ![](https://github.com/YangXiaoHei/Networking/blob/master/02%20应用层/images/wl18.png)
> 从上图可以看出，新首部行 “Authorization: ” 被加入到 GET 请求报文中。


