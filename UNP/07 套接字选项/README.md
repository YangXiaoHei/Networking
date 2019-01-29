
* setsockopt
* getsockopt

* 通用套接字选项
   * 级别 : `SOL_SOCKET`
		* `SO_KEEPALIVE`
		* `SO_RCVBUF`
		* `SO_SNDBUF`
		* `SO_RCVLOWAT`
		* `SO_SNDLOWAT`
		* `SO_REUSEADDR`
		* `SO_REUSEPORT`
		* `SO_LINGER`
		* `SO_RCVTIMEO` // 接收超时
		* `SO_SNDTIMEO`  // 发送超时
		* `SO_TYPE`
		
	* 级别 : `IPPROTO_IP`
	   * `IP_TTL` // 设置或获取默认 TTL 值

	* 级别 : `IPPROTO_TCP`
	   * `TCP_MAXSEG`
	
* SO_KEEPALIVE
   * ⚠️ 可能会中断仍然存活的连接。由于中间路由器的故障。
   * ⚠️ 如果将该时间调小，那么主机上所有套接字都会收到影响。
   * 2 小时内 TCP 套接字上任何一方没有数据交换，那么 TCP 自动给对端发送一个存活探测分组。
   
      * 若对端正常响应 ACK，那么应用程序得不到通知，在又经过没有动静的两小时后，TCP 发出另一个探测分组。
      
      * 若对端响应 RST。那么套接字待处理错误被置为 ECONNRESET，套接字本身被关闭。
      
      * 若对端对探测分组没有任何响应。那么源自 Berkeley 的 TCP 将另外发送 8 个探测分组，两两相隔  75 秒。TCP 在发出第一个探测分组后 11 分 15 秒内若没有任何响应则放弃。
      
* SO_LINGER

   * 默认动作: close 立即返回，但会发送完发送缓冲区中的数据，然后如果文件描述符引用计数减为 0，则发送 FIN。
   
   * `l_onoff = 1, l_linger = 0`，close 立即返回，丢弃发送缓冲区中的数据。并且立即给对端发送 RST。
   
   * `l_onoff = 1, l_linger != 0` close 会等待最多 `l_linger` 时长再返回，等待 `l_linger` 的过程中，如果已经收到对端对本端 FIN 的确认，close 就提前返回。  
   
   * ⚠️ 设置 `SO_LINGER` 后，close 成功返回值是告诉我们先前发送的数据和 FIN 已经由对端确认。而不能告诉我们对端应用程序是否已读取数据。如果不设置的话，那么连对端 TCP 是否确认了数据都不知道。


* 本端 TCP 如何返回问题
   * close 后立即返回（默认）
   * close 一直拖延到收到对端对数据和 FIN 的确认才返回
   * 后跟一个 read 调用的 shutdown 一直等到接收了对端的 FIN 才返回
   * 应用级 ACK

* shutdown 和 `SO_LINGER` 各种情况总结
   * `SHUT_RD` 不能对 fd 调用读函数；接受缓冲区中数据都被丢弃；再接收到的任何数据由 TCP 丢弃。
   
   * `SHUT_WR`不能再对 fd 调用写函数；发送缓冲区内容均被发到对端，后跟 FIN。
   
   * `close l_onoff = 1, l_linger = 0` 不能对 fd 调用读写函数；发送和接收缓冲区中数据均被丢弃；若引用计数减为 0，发送 RST 到对端，然后连接状态被置为 CLOSED（而没有 TIME_WAITED）。
   
   * `close l_onoff = 1, l_linger != 0` 不能对 fd 调用读写函数；接收缓冲区中数据被丢弃，发送缓冲区中数据被发送到对端，若引用计数减为 0，则后跟 FIN; 如果在连接变为 CLOSED 状态前 `l_linger` 时间到，那么 close 返回 `EWOULDBLOCK` 错误。

* `SO_REUSEADDR`
   * ⚠️ `SO_REUSEADDR` 选项的设置一定要在 bind 前调用，否则无效！！
   * ⚠️ 设置 `SO_REUSEADDR` 选项的服务器在主动关闭后，仍然会有 `TIME_WAIT` 状态，而不是直接进入 CLOSED，只不过设置了 `SO_REUSEADDR` 而处于 `TIME_WAIT` 状态的套接字仍然可以绑定同一个 IP 和端口。
   * ⚠️ 即使不设置 `SO_REUSEADDR`，绑定 0.0.0.0:20001 后再绑定 127.0.0.1:20001 仍然可以成功，因为通配地址直到 SYN 连接到达才确定本端的源 IP，而一般这个源 IP 会被 NAT 成私网 IP。TCP 套接字用四元祖来唯一标示。
   * ⚠️ TCP 套接字即使设置 `SO_REUSEADDR` 也不能让多个进程绑定同一个 IP 和同一个端口。但是 UDP 设置 `SO_REUSEADDR` 后，同样的 IP 和端口可以绑定到另一个套接字上。
   * ⚠️ `SO_REUSEADDR` 允许单个进程捆绑同一个端口到多个套接字上。只要每次绑定指定不同的本地 IP 地址即可。

* `TCP_MAXSEG`
   * 如果在套接字连接建立之前获得该值，那么返回值是未从对端收到 MSS 选项的情况下所用的默认值。在连接后取得的话，则是对端使用 SYN 分组通告的 MSS。
   * 如果使用 TCP 时间戳选项的话，那么实际使用的 MSS 将会小于对端用 SYN 分组通告的 MSS，因为首先 MTU 是天花板，既然 TCP 首部字段还要多一些空间，那么只能再挤占 MSS 的地盘。
   * 如果 TCP 支持路径 MTU 发现功能，那么它将发送的每个分组的最大数据量可能在连接存活期内改变，如果到对端的路径变动，该值就会有所调整。
   * 有的系统支持设置该值，有的不支持，即使支持设置，一般也只能调低，而不能增加。

* `TCP_NODELAY`
   *  

   



