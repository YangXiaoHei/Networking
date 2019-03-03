# 《Unix Network Programming V1》

* 第 1 章 简介

   * [note](https://github.com/YangXiaoHei/Networking/blob/master/UNP/01%20简介/README.md)
   * **获取时间客户端程序 IPv4 版本** [GetServerDaytimeClient.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/01%20简介/progs/GetServerDaytimeClient.c)
   * **获取时间客户端程序 IPv6 版本**  [GetServerDaytimeClient_IPv6.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/01%20简介/progs/GetServerDaytimeClient_IPv6.c)
   * **获取时间服务器程序** [DaytimeServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/01%20简介/progs/DaytimeServer.c)
   * [习题](https://github.com/YangXiaoHei/Networking/blob/master/UNP/01%20简介/习题.md)

* 第 2 章 传输层: TCP、UDP 和 SCTP

  * [note](https://github.com/YangXiaoHei/Networking/blob/master/UNP/02%20传输层%20TCP%20UDP%20和%20SCTP/README.md)
  * **向 TCP 发送缓冲区注入大量数据 - 阻塞直到全部发出** [SendBigDataByTCP.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/02%20传输层%20TCP%20UDP%20和%20SCTP/progs/SendBigDataByTCP.c)
  * **向 UDP 发送缓冲区注入大量数据 - 返回 EMSGSZIE 错误** [SendBigDataByUDP.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/02%20传输层%20TCP%20UDP%20和%20SCTP/progs/SendBigDataByUDP.c)
  * [习题](https://github.com/YangXiaoHei/Networking/blob/master/UNP/02%20传输层%20TCP%20UDP%20和%20SCTP/习题.md)

* 第 3 章 套接字编程简介
   
   * **IP 字符串数值互转旧版本 API 的问题** [BugOfAddressTransform.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/03%20套接字编程简介/progs/BugOfAddressTransform.c)
   * **大小端判断** [BigEndianOrLittleEndian.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/03%20套接字编程简介/progs/BigEndianOrLittleEndian.c)
   * **套接字结构体中存放的 IP 和端口默认被解释为大端存储** [AddressTransformFunctionUseBigEndianByDefault.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/03%20套接字编程简介/progs/AddressTransformFunctionUseBigEndianByDefault.c)
   * **实现 IP 地址字符串和数值的互相转换** [ImplementationOfAddressTransformFunction.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/03%20套接字编程简介/progs/ImplementationOfAddressTransformFunction.c)
   * **一次性读取指定 n 个字节** [ReadNBytes.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/03%20套接字编程简介/progs/ReadNBytes.c)
   * **一次性写入指定 n 个字节** [WriteNBytes.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/03%20套接字编程简介/progs/WriteNBytes.c)
   * **一次性读取一行 (非常慢版本)** [VerySlowReadLine.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/03%20套接字编程简介/progs/VerySlowReadLine.c)
   * **一次性读取一行 (快速版本-预读技术)** [QuickReadLine.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/03%20套接字编程简介/progs/QuickReadLine.c)
   * [习题](https://github.com/YangXiaoHei/Networking/blob/master/UNP/03%20套接字编程简介/习题.md)

* 第 4 章 基本 TCP 套接字编程
   * [note](https://github.com/YangXiaoHei/Networking/blob/master/UNP/04%20基本%20TCP%20套接字编程/README.md)
   * **TCP 多进程时间获取服务器** [ConcurrentMultiProcessesDaytimeServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/04%20基本%20TCP%20套接字编程/progs/ConcurrentMultiProcessesDaytimeServer.c)
  * **TCP 多进程时间获取服务器壳子 - fork 然后 exec echo 程序** [ConcurrentMultiProcessesDaytimeServerHandleByNewProgram.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/04%20基本%20TCP%20套接字编程/progs/ConcurrentMultiProcessesDaytimeServerHandleByNewProgram.c)
  * **TCP 多进程时间获取服务器 - echo 程序 - 调用 getpeername 显示源 IP 和源端口** [RunByExecFunctionServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/04%20基本%20TCP%20套接字编程/progs/RunByExecFunctionServer.c)
  * **TCP 时间获取客户端 - 调用 getsockname 显示源 IP 和源端口** [GetServerDaytimeClientDisplayAddress.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/04%20基本%20TCP%20套接字编程/progs/GetServerDaytimeClientDisplayAddress.c)
   * [习题](https://github.com/YangXiaoHei/Networking/blob/master/UNP/04%20基本%20TCP%20套接字编程/习题.md)

* 第 5 章 TCP 客户/服务器程序示例
   * [note](https://github.com/YangXiaoHei/Networking/blob/master/UNP/05%20TCP%20客户%20服务器程序示例/README.md)
   * **fgets 和 fputs 对比** [fgetsAndfputs.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/05%20TCP%20客户%20服务器程序示例/progs/fgetsAndfputs.c)
   * **TCP 多进程回显服务器** [ConcurrentMultiProcessEchoServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/05%20TCP%20客户%20服务器程序示例/progs/ConcurrentMultiProcessEchoServer.c)
   * **TCP 回显客户端** [EchoClient.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/05%20TCP%20客户%20服务器程序示例/progs/EchoClient.c)
   * **connect 是否在 accept 之前返回 : YES**
      * [IfConnectRetureBeforeAcceptServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/05%20TCP%20客户%20服务器程序示例/progs/IfConnectRetureBeforeAcceptServer.c)
      * [IfConnectRetureBeforeAcceptClient.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/05%20TCP%20客户%20服务器程序示例/progs/IfConnectRetureBeforeAcceptClient.c)
  * **Wait 和 WaitPid 的测试客户端 (for 中调用 5 次 connect 引发服务器创建 5 个子进程)** [WaitAndWaitpidTestClient.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/05%20TCP%20客户%20服务器程序示例/progs/WaitAndWaitpidTestClient.c)
  * **TCP 多进程回显服务器 - 使用 wait 只回收了部分僵尸进程 ❌** [ConcurrentMultiProcessEchoServerHandleZombieBug.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/05%20TCP%20客户%20服务器程序示例/progs/ConcurrentMultiProcessEchoServerHandleZombieBug.c)  
  * **TCP 多进程回显服务器 - 使用 waitpid 成功回收所有僵尸进程 ✅** [ConcurrentMultiProcessEchoServerHandleZombieCorrect.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/05%20TCP%20客户%20服务器程序示例/progs/ConcurrentMultiProcessEchoServerHandleZombieCorrect.c)
  * **TCP 多进程回显服务器 - 服务器进程终止情形 (用来为 tcpdump 抓包提供情景)** [ServerProcessTerminated.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/05%20TCP%20客户%20服务器程序示例/progs/ServerProcessTerminated.c)

* 第 6 章 简介
   * **客户端缓冲区改进，I/O 多路复用** [SelectEchoClient.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/06%20IO复用%20selet%20和%20poll%20函数/progs/SelectEchoClient.c)
   * **客户端缓冲区改进，I/O 多路复用，半关闭** [SelectEchoClientHalfClose.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/06%20IO复用%20selet%20和%20poll%20函数/progs/SelectEchoClientHalfClose.c)
   * **服务器使用 I/O 多路复用替代多进程 - select** [SelectEchoServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/06%20IO复用%20selet%20和%20poll%20函数/progs/SelectEchoServer.c)
   * **服务器使用 I/O 多路复用替代多进程 - poll** [PollEchoServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/06%20IO复用%20selet%20和%20poll%20函数/progs/PollEchoServer.c)
   * [习题](https://github.com/YangXiaoHei/Networking/blob/master/UNP/06%20IO复用%20selet%20和%20poll%20函数/习题.md)

* 第 7 章 套接字选项
   * [note](https://github.com/YangXiaoHei/Networking/blob/master/UNP/07%20套接字选项/README.md)
   * **展示 TCP 套接字选项的默认值** [TCPDisplayDefaultValue.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/07%20套接字选项/progs/TCPDisplayDefaultValue.c)
   * **展示 UDP 套接字选项的默认值** [UDPDisplayDefaultValue.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/07%20套接字选项/progs/UDPDisplayDefaultValue.c)
   * **linger 对 close 的影响 - 丢弃发送缓冲区数据，发送 RST** [CloseTestClientLinger0.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/07%20套接字选项/progs/CloseTestClientLinger0.c)
   * **linger 对 close 的影响 - 直到接收到对端对 FIN 的 ACK 才返回** [CloseTestClientLinger30.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/07%20套接字选项/progs/CloseTestClientLinger30.c)
   * **通常的 close - 立即返回，尝试发送发送缓冲区中剩余数据，若文件计数为 0 发送 FIN** [CloseTestClientNormal.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/07%20套接字选项/progs/CloseTestClientNormal.c)
   * **上面三个测试客户端使用的 server** [CloseTestServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/07%20套接字选项/progs/CloseTestServer.c)
   * **不使用 `SO_REUSEADDR` 选项的 server** [`NotUseOptionSO_REUSEADDR.c`](https://github.com/YangXiaoHei/Networking/blob/master/UNP/07%20套接字选项/progs/NotUseOptionSO_REUSEADDR.c)
   * **使用 `SO_REUSEADDR` 选项的 server** [`UseOptionSO_REUSEADDR_BindDifferentIp.c`](https://github.com/YangXiaoHei/Networking/blob/master/UNP/07%20套接字选项/progs/UseOptionSO_REUSEADDR_BindDifferentIp.c)
   * **查看 `TCP_NODELAY` 选项的影响的客户端** [`TCP_NODELAYClient.c`](https://github.com/YangXiaoHei/Networking/blob/master/UNP/07%20套接字选项/progs/TCP_NODELAYClient.c)
   * [习题](https://github.com/YangXiaoHei/Networking/blob/master/UNP/07%20套接字选项/习题.md)
  
* 第 8 章 基本 UDP 套接字编程
   * [note](https://github.com/YangXiaoHei/Networking/blob/master/UNP/08%20基本%20UDP%20套接字编程/note.md)
   * **UDP 回显服务器** [UDPEchoServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/08%20基本%20UDP%20套接字编程/progs/UDPEchoServer.c)
   * **UDP 回显客户端 - 检查返包来源是否正确** [`UDPEchoClient_CheckServerIP.c`](https://github.com/YangXiaoHei/Networking/blob/master/UNP/08%20基本%20UDP%20套接字编程/progs/UDPEchoClient_CheckServerIP.c)
   * **UDP 回显客户端 - 未连接套接字** [UDPEchoClient.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/08%20基本%20UDP%20套接字编程/progs/UDPEchoClient.c)
   * **UDP 回显客户端 - 未连接套接字 - 使用 NULL 的 sendto 调用** [`UDPEchoDisconnectedClient_EDSTADDRREQ.c`](https://github.com/YangXiaoHei/Networking/blob/master/UNP/08%20基本%20UDP%20套接字编程/progs/UDPEchoDisconnectedClient_EDSTADDRREQ.c)
   * **UDP 回显客户端 - 已连接套接字 - 使用传非 NULL 参的 sendto 调用** [`UDPEchoConnectedClient_EISCONN.c`](https://github.com/YangXiaoHei/Networking/blob/master/UNP/08%20基本%20UDP%20套接字编程/progs/UDPEchoConnectedClient_EISCONN.c)
   * **UDP 回显客户端 - 已连接套接字** [UDPEchoConnectedClient.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/08%20基本%20UDP%20套接字编程/progs/UDPEchoConnectedClient.c)
   * **UDP 通过已连接套接字和 getsockname 获取源 IP 和源端口** [UDPGetSourceAddrByConnectAndGetSockName.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/08%20基本%20UDP%20套接字编程/progs/UDPGetSourceAddrByConnectAndGetSockName.c)
   * **UDP 缺乏流量控制实验 - 服务器 - 读取速率可调节** [UDPLackFlowControlServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/08%20基本%20UDP%20套接字编程/progs/UDPLackFlowControlServer.c)
   * **UDP 缺乏流量控制实验 - 服务器 - 读取速率可调节 - 发送缓冲区很大** [UDPLackFlowControlServer_BiggerBufferSpace.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/08%20基本%20UDP%20套接字编程/progs/UDPLackFlowControlServer_BiggerBufferSpace.c)
   * **UDP 缺乏流量控制实验 - 客户端 - 发送速率可调节** [UDPLackFlowControlClient_DelaySend.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/08%20基本%20UDP%20套接字编程/progs/UDPLackFlowControlClient_DelaySend.c)
   * **使用 select，对相同端口同时绑定 UDP 和 TCP 的回显服务器** [UDPTCPSelectEchoServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/08%20基本%20UDP%20套接字编程/progs/UDPTCPSelectEchoServer.c)
   * [习题](https://github.com/YangXiaoHei/Networking/blob/master/UNP/08%20基本%20UDP%20套接字编程/习题.md)

* 第 11 章 名字与地址转换

   * **用主机名或域名获取 IP** [GetHostByName.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/11%20名字与地址转换/progs/GetHostByName.c)
   * **用 IP 获取主机名或域名 - 获取域名需要 PTR 记录支持** [GetHostByAddr.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/11%20名字与地址转换/progs/GetHostByAddr.c)
   * **使用服务名和套接字类型获取端口，基于 /etc/services 的配置** [GetHostByAddr.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/11%20名字与地址转换/progs/GetServByName.c)
   * **使用端口和套接字类型获取服务名，基于 /etc/services 的配置** [GetServByPort.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/11%20名字与地址转换/progs/GetServByPort.c)
   * **TCP 时间获取服务器 - 使用 getaddrinfo** [TCPDaytimeServerUseGetAddrInfo.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/11%20名字与地址转换/progs/TCPDaytimeServerUseGetAddrInfo.c)
   * **TCP 时间获取客户端 - 使用 getaddrinfo** [TCPDaytimeClientUseGetAddrInfo.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/11%20名字与地址转换/progs/TCPDaytimeClientUseGetAddrInfo.c)
   * **UDP 时间获取服务器 - 使用 getaddrinfo** [TCPDaytimeClientUseGetAddrInfo.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/11%20名字与地址转换/progs/TCPDaytimeClientUseGetAddrInfo.c)
   * **UDP 时间获取客户端 - 使用 getaddrinfo** [UDPDaytimeClientUseGetAddrInfo.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/11%20名字与地址转换/progs/UDPDaytimeClientUseGetAddrInfo.c)
   * **UDP 时间获取客户端 - 使用 getaddrinfo - 使用已连接套接字** [UDPDaytimeClientUseGetAddrInfo_Connected.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/11%20名字与地址转换/progs/UDPDaytimeClientUseGetAddrInfo_Connected.c)
   * **TCP 回显客户端 - 使用 getaddrinfo** [TCPEchoClientUseGetAddrInfo.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/11%20名字与地址转换/progs/TCPEchoClientUseGetAddrInfo.c)
   * [习题](https://github.com/YangXiaoHei/Networking/blob/master/UNP/11%20名字与地址转换/习题.md)     

* 第 13 章 守护进程和 inetd 超级服务器
   * [note](https://github.com/YangXiaoHei/Networking/blob/master/UNP/13%20守护进程和%20inetd%20超级服务器/README.md)
   * **创建守护进程：演示了所有步骤** [CreateDaemonProcess.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/13%20守护进程和%20inetd%20超级服务器/progs/CreateDaemonProcess.c)
   * **同时支持 TCP 和 UDP 的时间获取服务器守护进程** [TCPUDPDaytimeServerWithDaemonProcess.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/13%20守护进程和%20inetd%20超级服务器/progs/TCPUDPDaytimeServerWithDaemonProcess.c)
   * **TCP 时间获取客户端** [TCPDaytimeClientUseGetAddrInfo.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/13%20守护进程和%20inetd%20超级服务器/progs/TCPDaytimeClientUseGetAddrInfo.c)
   * **UDP 时间获取客户端** [UDPDaytimeClientUseGetAddrInfo.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/13%20守护进程和%20inetd%20超级服务器/progs/UDPDaytimeClientUseGetAddrInfo.c)
   * **通过 xinetd 启动的 TCP 时间获取服务器** [TCPDaytimeServerExecByXinetd.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/13%20守护进程和%20inetd%20超级服务器/progs/TCPDaytimeServerExecByXinetd.c)
   * **通过 xinetd 启动的 UDP 时间获取服务器** [UDPDaytimeServerExecByXinetd.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/13%20守护进程和%20inetd%20超级服务器/progs/UDPDaytimeServerExecByXinetd.c)
   * [习题](https://github.com/YangXiaoHei/Networking/blob/master/UNP/13%20守护进程和%20inetd%20超级服务器/习题.md)

* 第 14 章 高级 IO 函数
   * [note](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/README.md)
   * **使用 SIGALRM 给 connect 设置超时 - 有缺陷** [TimeoutConnectClientBySignal.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/TimeoutConnectClientBySignal.c)
   * **使用 SIGALRM 给 recvfrom 设置超时** [TimeoutRecvfromClientBySignal_UDPDaytimeClient.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/TimeoutRecvfromClientBySignal_UDPDaytimeClient.c)
   * **使用 select 给 recvfrom 设置超时** [TimeoutRecvfromClientBySelect.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/TimeoutRecvfromClientBySelect.c)
   * **使用套接字选项 SO_RCVTIMEO 给 recvfrom 设置超时** [TimeoutRecvfromClientBySockOpt.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/TimeoutRecvfromClientBySockOpt.c)
   * **测试超时 API 的服务器 - 可以指定推迟几秒给回包** [UDPDaytimeServer_Delay.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/TimeoutRecvfromClientBySockOpt.c)
   * **writev 测试客户端**  [TCPWritevTestClient.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/UDPDaytimeServer_Delay.c)
   * **writev 测试服务器** [TCPWritevTestServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/TCPWritevTestServer.c)
   * **readv 测试客户端 - 每次循环写一个字节 - 禁用 Nagle 算法** [TCPReadvTestClient_EveryLoopOneByte_TCP_NODELAY.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/TCPReadvTestClient_EveryLoopOneByte_TCP_NODELAY.c)
   * **readv 测试客户端 - 一次写完所有数据** [TCPReadvTestClient_WriteAllOneTime.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/TCPReadvTestClient_WriteAllOneTime.c)
   * **readv 测试服务器** [TCPReadvTestServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/TCPReadvTestServer.c)
   * **recv flag 测试服务器 - MSG_PEEK** [TCPRecvFlagTestServer_MSG_PEEK.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/TCPRecvFlagTestServer_MSG_PEEK.c)
   * **recv flag 测试服务器 - MSG_WAITALL** [TCPRecvFlagTestServer_MSG_WAITALL.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/TCPRecvFlagTestServer_MSG_WAITALL.c)
   * **recv flag 测试客户端 - 每次循环写一个字节 - 禁用 Nagle 算法 - 每次循环睡 0.5 秒** [TCPRecvTestClient_EveryLoopOneByte_TCP_NODELAY_Delay.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/TCPRecvTestClient_EveryLoopOneByte_TCP_NODELAY_Delay.c)
   * **查看 TCP 套接字排队数据量 - 客户端** [client_TCP_EveryLoopOneByte_TCP_NODELAY_Delay.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/client_TCP_EveryLoopOneByte_TCP_NODELAY_Delay.c)
   * **查看 UDP 套接字排队数据量 - 客户端** [client_UDP_EveryLoopOneByte_Delay.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/client_UDP_EveryLoopOneByte_Delay.c)
   * **查看 TCP 套接字排队数据量 - 服务器 - ioctl** [server_TCP_LookHowManyInRecvBufferButNotRead_ioctl.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/server_TCP_LookHowManyInRecvBufferButNotRead_ioctl.c)
   * **查看 TCP 套接字排队数据量 - 服务器 - MSG_PEEK** [server_TCP_LookHowManyInRecvBufferButNotRead_MSG_PEEK.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/server_TCP_LookHowManyInRecvBufferButNotRead_MSG_PEEK.c)
   * **查看 UDP 套接字排队数据量 - 服务器 - ioctl** [server_UDP_LookHowManyInRecvBufferButNotRead_ioctl.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/server_UDP_LookHowManyInRecvBufferButNotRead_ioctl.c)
   * **查看 UDP 套接字排队数据量 - 服务器 - MSG_PEEK** [server_UDP_LookHowManyInRecvBufferButNotRead_MSG_PEEK.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/server_UDP_LookHowManyInRecvBufferButNotRead_MSG_PEEK.c)
   * **对套接字使用标准 IO 库的缓冲问题 - 服务器** [TCPEchoServer_StandardIO.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/TCPEchoServer_StandardIO.c)
   * **对套接字使用标准 IO 库的缓冲问题 - 客户端 - 使用半关闭修复了 close 的 bug** [TCPEchoClientUseGetAddrInfo.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/TCPEchoClientUseGetAddrInfo.c)
   *  **对 sendmsg 的使用 - 客户端** [UDPClientUseSendmsg.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/UDPClientUseSendmsg.c)
   *  **对 recvmsg 的使用 - 服务器 - 【额外】通过设置 IP_RECVORIGDSTADDR 套接字选项获取 UDP 目的 IP 地址** [UDPServerUseRecvmsg.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/UDPServerUseRecvmsg.c)
   *  [习题](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/习题.md)

* 第 15 章 Unix 域协议
   * [note.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/15%20Unix%20域协议/README.md)
   * **给 Unix 域套接字绑定一个路径** [BindPathForUnixFd.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/15%20Unix%20域协议/progs/BindPathForUnixFd.c)
   * **Unix 域套接字版本 - TCP 回显客户端** [TCPEchoClient_UnixFd.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/15%20Unix%20域协议/progs/TCPEchoClient_UnixFd.c)
   * **Unix 域套接字版本 - TCP 回显服务器** [TCPEchoServer_UnixFd.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/15%20Unix%20域协议/progs/TCPEchoServer_UnixFd.c)
   * **Unix 域套接字版本 - UDP 回显客户端** [UDPEchoClient_UnixFd.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/15%20Unix%20域协议/progs/UDPEchoClient_UnixFd.c)
   * **Unix 域套接字版本 - UDP 回显服务器** [UDPEchoServer_UnixFd.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/15%20Unix%20域协议/progs/UDPEchoServer_UnixFd.c)
   * **socketpair 测试** [SocketPairTest.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/15%20Unix%20域协议/progs/SocketPairTest.c)
   * **backlog 和已连接数的关系 - 可以分别在 Mac 和 Linux 上测试** [GetRatioOfConnectionsByBacklog.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/15%20Unix%20域协议/progs/GetRatioOfConnectionsByBacklog.c)
   * **描述符传递 - 被传递描述符的读取端 - 父进程** [FdTransfer_Read_End.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/15%20Unix%20域协议/progs/FdTransfer_Read_End.c)
   * **描述符传递 - 被传递描述符的写入端 - 子进程** [FdTransfer_Write_End.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/15%20Unix%20域协议/progs/FdTransfer_Write_End.c)
   * [习题](https://github.com/YangXiaoHei/Networking/blob/master/UNP/15%20Unix%20域协议/习题.md)

* 第 16 章 非阻塞式 IO
   * **非阻塞 connect 测试** [NonblockConnect.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/16%20非阻塞式%20IO/progs/NonblockConnect.c)
   * **读写非阻塞 TCP 回显客户端 - 2 个缓冲区，4 个读写控制游标** [NonblockTCPEchoClient.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/16%20非阻塞式%20IO/progs/NonblockTCPEchoClient.c)
   * **读写非阻塞 TCP 回显客户端 - 一次运行的时序日志 - 合并了 tcpdump 日志** [analyze](https://github.com/YangXiaoHei/Networking/blob/master/UNP/16%20非阻塞式%20IO/progs/analyze)
   * **分成两个进程的 TCP 回显客户端 - 代码超简单** [TCPEchoClientByFork.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/16%20非阻塞式%20IO/progs/TCPEchoClientByFork.c)
   * **简单的多进程 Web 服务器 - 只能处理 GET 请求 - 请求路径必须和执行程序处于同一目录下** [WebServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/16%20非阻塞式%20IO/progs/WebServer.c)
   * **Web 客户端 - 批量下载文件 - 每次阻塞型 connect，发送下载请求，接收响应写文件** [WebClientMultiConnectDownload_Block.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/16%20非阻塞式%20IO/progs/WebClientMultiConnectDownload_Block.c) **编译 + 运行脚本** [run_block.sh](https://github.com/YangXiaoHei/Networking/blob/master/UNP/16%20非阻塞式%20IO/progs/run_block.sh)
   * **Web 客户端 - 批量下载文件 - 可以指定最大连接数 - 先创建若干连接 - 然后轮询下载** [WebClientMultiConnectDownload_Nonblock.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/16%20非阻塞式%20IO/progs/WebClientMultiConnectDownload_Nonblock.c) **编译 + 运行脚本** [run_nonblock.sh](https://github.com/YangXiaoHei/Networking/blob/master/UNP/16%20非阻塞式%20IO/progs/run_nonblock.sh)
   * **客户端 - 阻塞型 accept 可能会引发的阻塞 BUG 验证 - connect 返回后睡 1 秒然后发送 RST - 然而并没有得到书上所述的结果 💔** [BlockAcceptBug_TCPClient.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/16%20非阻塞式%20IO/progs/BlockAcceptBug_TCPClient.c)
   * **服务器 - 阻塞型 accept 可能会引发的阻塞 BUG 验证 - select 可读事件后睡 5 秒然后 accept - 然而并没有得到书上所述的结果 💔** [BlockAcceptBug_TCPServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/16%20非阻塞式%20IO/progs/BlockAcceptBug_TCPServer.c)
   * **阻塞型 accept 可能会引发的阻塞 BUG 验证 - 合并了 tcpdump 的时序日志** [timeline.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/16%20非阻塞式%20IO/progs/timeline.c)
   * [习题](https://github.com/YangXiaoHei/Networking/blob/master/UNP/16%20非阻塞式%20IO/习题.md)

* 第 17 章 ioctl 操作
   * [note](https://github.com/YangXiaoHei/Networking/blob/master/UNP/17%20ioctl%20操作/README.md)
   * **ioctl - SIOCGARP 获取指定 IP 的 ARP 表项 - (MAC 没有 SIOCGARP/linux 总是报错)** [ARPOperation.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/17%20ioctl%20操作/progs/ARPOperation.c)
   * **ioctl - SIOCGIFCONF 获取全部接口 - 分配较小缓冲区 - (MAC/linux 皆可)** [AssignMemoryForIfconfTooLess.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/17%20ioctl%20操作/progs/AssignMemoryForIfconfTooLess.c)
   * **ioctl - SIOCGIFCONF - 获取网络接口的工具** [InterfaceTool.h](https://github.com/YangXiaoHei/Networking/blob/master/UNP/17%20ioctl%20操作/progs/InterfaceTool.h) [InterfaceTool.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/17%20ioctl%20操作/progs/InterfaceTool.c)
   * **测试 - 获取网络接口工具** [TestInterface.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/17%20ioctl%20操作/progs/TestInterface.c)
   * **写一个完善的 writen - 客户端 - 子进程不断对父进程发送中断信号 - 考虑非阻塞描述符** [WriteNClient.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/17%20ioctl%20操作/progs/WriteNClient.c)
   * **写一个完善的 writen - 服务器** [WriteNServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/17%20ioctl%20操作/progs/WriteNServer.c)
   * [习题](https://github.com/YangXiaoHei/Networking/blob/master/UNP/17%20ioctl%20操作/习题.md)

* 第 18 章 路由套接字 (⚠️ ***linux 系统不支持 sysctl 和路由套接字, 以下代码均在 Mac OSX 运行测试***)
   * **sysctl 检查 UDP 校验和是否开启** [CheckIfUDPChecksumEnable.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/18%20路由套接字/progs/CheckIfUDPChecksumEnable.c)
   * **sysctl 获取全部 ARP 表项** [GetARPTable_BySysctl.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/18%20路由套接字/progs/GetARPTable_BySysctl.c)
   * **sysctl 获取全部路由表项** [GetRouteTable_BySysctl.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/18%20路由套接字/progs/GetRouteTable_BySysctl.c)
   * **路由套接字 获取指定路由表项 - （⚠️ 路由套接字无法获得整个路由表，只能获得指定地址的那一个表项）** [GetRouteTableEntry_BySockfd.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/18%20路由套接字/progs/GetRouteTableEntry_BySockfd.c)
   * **禁用 SO_USELOOPBACK 使得内核不会向进程发送数据** [Practise_02.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/18%20路由套接字/progs/Practise_02.c)
   * **sysctl 获取网络接口工具** [InterfaceTool_v2.h](https://github.com/YangXiaoHei/Networking/blob/master/UNP/18%20路由套接字/progs/InterfaceTool_v2.h) [InterfaceTool_v2.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/18%20路由套接字/progs/InterfaceTool_v2.c)
   * **sysctl 获取网络接口工具 - 测试** [TestInterface.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/18%20路由套接字/progs/TestInterface.c)
   * **向 2^n 字节对齐 - 宏的测试** [RoundUpTest.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/18%20路由套接字/progs/RoundUpTest.c)
   * **获取接口索引 - by 接口名** [GetInterfaceIndexByName.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/18%20路由套接字/progs/GetInterfaceIndexByName.c)
   * **获取接口名 - by 接口索引** [GetInterfaceNameByIndex.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/18%20路由套接字/progs/GetInterfaceNameByIndex.c)
   * [习题](https://github.com/YangXiaoHei/Networking/blob/master/UNP/18%20路由套接字/习题.md)

* 第 22 章 高级 UDP 套接字编程
   * **服务器端获取 UDP 目的 IP - SO_RECVORIGDSTADDR - recvmsg - 服务器** [GetUDPIP_server.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/22%20高级%20UDP%20套接字编程/progs/GetUDPIP_server.c)
   * **服务器端获取 UDP 目的 IP - SO_RECVORIGDSTADDR - recvmsg - 客户端** [GetUDPIP_client.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/22%20高级%20UDP%20套接字编程/progs/GetUDPIP_client.c)
   * **UDP 回显客户端 - 验证 UDP 会丢包** [UDPEchoClient_TimeoutRecv_TestPacketLoss.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/22%20高级%20UDP%20套接字编程/progs/UDPEchoClient_TimeoutRecv_TestPacketLoss.c)
   * **可靠 UDP - 超时重传 - 动态估计 RTO - 指数退避 - 避免重传二义性 - 序号确认** [reliable_udp.h](https://github.com/YangXiaoHei/Networking/blob/master/UNP/22%20高级%20UDP%20套接字编程/progs/reliable_udp.h) [reliable_udp.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/22%20高级%20UDP%20套接字编程/progs/reliable_udp.c)
   * **可靠 UDP - select 版** [UDPEchoClient_Reliable_select.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/22%20高级%20UDP%20套接字编程/progs/UDPEchoClient_Reliable_select.c)
   * **可靠 UDP 测试客户端** [UDPEchoClient_Reliable.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/22%20高级%20UDP%20套接字编程/progs/UDPEchoClient_Reliable.c)
   * **并发的 UDP 回显服务器 - 模拟 TCP : 监听套接字收到请求连入, 创建新套接字用于通信** [ConccurentUDPEchoServer.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/22%20高级%20UDP%20套接字编程/progs/ConccurentUDPEchoServer.c)
   * **并发的 UDP 回显客户端 - 模拟 TCP : 发送三次握手分组** [ConccurentUDPEchoClient.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/22%20高级%20UDP%20套接字编程/progs/ConccurentUDPEchoClient.c)
   * **创建一个指定字节文件 - 填充任意字符** [CreateData.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/22%20高级%20UDP%20套接字编程/progs/CreateData.c)
          
# 《计算机网络-自顶向下 6th》

* 第 1 章 计算机网络和因特网
  * [复习题](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/01%20计算机网络和因特网/复习题.md) ✅
  * [习题](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/01%20计算机网络和因特网/习题.md) ✅
  * 实验
  	* [Wireshake lab-Intro](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/01%20计算机网络和因特网/Wireshake_lab-Intro.md) ✅
  
* 第 2 章 应用层
  * [复习题](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/复习题.md) ✅
  * [习题](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/习题.md) ✅
  * 套接字编程作业
    * Web 服务器 ✅
      * [WebServer.c](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/progs/WebServer.c)
      * [WebClient.c](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/progs/WebClient.c)
    * UDP ping 程序 ✅
      * [UDPPingerServer.c](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/progs/UDPPingerServer.c)
      * [UDPPingerClient.c](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/progs/UDPPingerClient.c)
    * 邮件客户 ✅
      * [SMTPClient.c](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/progs/SMTPClient.c)
    * 多线程 Web 代理服务器 ✅
      * [WebServerProxy.c](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/progs/WebServerProxy.c)
      * [iOS App "iPhoneTCPClient" for test](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/progs/iPhoneTCPClient)
      * [WebServer.c for test](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/progs/WebServer.c)
  * 实验
     * [Wireshake lab - HTTP](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/Wireshake_lab-HTTP.md)  ✅
     * [Wireshake lab - DNS](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/02%20应用层/Wireshake_lab-DNS.md)  ✅
  
 * 第 3 章 运输层
   * [note](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/README.md) ✅
   * [复习题 3.1 ~ 3.3](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/复习题_31_33.md) ✅
   * [复习题 3.4](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/复习题_34.md) ✅
   * [复习题 3.5](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/复习题_35.md) ✅
   * [复习题 3.7](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/复习题_37.md) ✅
   * [习题 P1 ~ P2 【目的端口、源端口理解】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/习题_01_02.md) ✅
   * [习题 P3 ~ P5 【校验和计算】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/习题_03_05.md) ✅
   * [习题 P6 ~ P16 【可靠传输协议原理】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/习题_06_16.md) ✅
   * [习题 P17 ~ P21 【设计可靠传输协议】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/习题_17_21.md) ✅ 
   * [习题 P22 ~ P24 【GBN、SR】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/习题_22_24.md) ✅ 
   * [习题 P25 ~ P28 【TCP 报文段相关计算】](https://github.com/YangXiaoHei/Networking/blob/master/03%20运输层/习题_25_28.md) ✅ 
   * [习题 P29 & P55 【SYN cookie】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/习题_29&55.md) ✅ 
   * [习题 P30 ~ P33 【TCP 超时时间计算】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/习题_30_33.md) ✅ 
   * [习题 P34 ~ P56 【TCP 拥塞控制算法: 理解 & 计算】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/习题_34_56.md) ✅ 
   * 编程作业
     * [Stop_And_Wait 仿真](https://github.com/YangXiaoHei/Networking/tree/master/计算机网络自顶向下/03%20运输层/progs/Stop_And_Wait) ✅
     * [Go_Back_N 仿真](https://github.com/YangXiaoHei/Networking/tree/master/计算机网络自顶向下/03%20运输层/progs/GBN) ✅
   * 实验
     * [Wireshake lab - TCP](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/Wireshake_lab-TCP.md) ✅ 
     * [Wireshake lab - UDP](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/03%20运输层/Wireshake_lab-UDP.md) ✅
   
* 第 4 章 网络层
   * [note](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/README.md)  ✅
   * [复习题 4.1 ~ 4.2](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/复习题_41_42.md)  ✅
   * [复习题 4.3 ~ 4.4](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/复习题_43_44.md)  ✅
   * [复习题 4.5 ~ 4.7](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/复习题_45_47.md)  ✅
   * [习题 P1 ~ P6 【虚电路】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/习题_01_06.md)  ✅
   * [习题 P7 ~ P9 【路由器交换结构】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/习题_07_09.md)  ✅
   * [习题 P10 ~ P18  【前缀匹配、IPv4 编址】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/习题_10_18.md)  ✅
   * [习题 P19 ~ P20 【IP 数据报分片】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/习题_19_20.md)  ✅
   * [习题 P21 ~ P23 【NAT】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/习题_21_23.md)  ✅
   * [习题 P24 ~ P27 【链路状态路由选择算法】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/习题_24_27.md)  ✅
   * [习题 P28 ~ P34  【距离向量路由选择算法】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/习题_28_34.md)  ✅
   * [习题 P35 ~ P43  【BGP】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/习题_35_43.md)  ✅
   * [习题 P44 ~ P55 【广播、多播、生成树】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/习题_44_55.md)  ✅
   * 编程作业
     * 分布式距离向量算法
     * Ping
   * 实验
     * [Wireshake lab - IP](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/Wireshake_lab-IP.md) ✅
     * [Wireshake lab - NAT](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/Wireshake_lab-NAT.md) ✅
     * [Wireshake lab - ICMP](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/Wireshake_lab-ICMP.md) ✅
     * [Wireshake lab - DHCP](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/04%20网络层/Wireshake_lab-DHCP.md) ✅
     
* 第 5 章 链路层
  * [note](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/05%20链路层/README.md)
  * [复习题 5.1 ~ 5.2](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/05%20链路层/复习题_51_52.md) ✅
  * [复习题 5.3](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/05%20链路层/复习题_53.md) ✅
  * [复习题 5.4](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/05%20链路层/复习题_54.md) 
  * [习题 P1 ~ P7 【奇偶校验、校验和、CRC】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/05%20链路层/习题_01_07.md) 
  * [习题 P8 ~ P13 【时隙 ALOHA、纯 ALOHA、轮询多路访问】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/05%20链路层/习题_08_13.md) ✅
  * [习题 P14 ~ P16 & P21 ~ P22 & P26 【ARP、MAC 地址、交换机寻址】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/05%20链路层/习题_14_16.md) ✅ 
  * [习题 P17 ~ P20 【CSMA/CD】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/05%20链路层/习题_17_20.md)  ✅ 
  * [习题 P23 ~ P25 【交换机对比集线器】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/05%20链路层/习题_23_25.md) ✅ 
  * [习题 P27【IP 语音应用】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/05%20链路层/习题_27.md) ✅ 
  * [习题 P28 ~ P30【VLAN、MPLS】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/05%20链路层/习题_28_30.md) ✅ 
  * [习题 P31【从打开电脑到获取 Web 页面都发生了什么？】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/05%20链路层/习题_31.md) 
  * [习题 P32 ~ P33【等级体系结构--数据中心】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/05%20链路层/习题_28_30.md) ✅ 
  * 实验
     * [Wireshake lab - Ethernet_ARP](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/05%20链路层/Wireshake_lab-Ethernet_ARP.md) ✅
  
* 第 8 章 计算机网络中的安全
  * [note](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/README.md) ✅
  * [复习题 8.1](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/复习题_81.md) ✅
  * [复习题 8.2](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/复习题_82.md) ✅
  * [复习题 8.3 ~ 8.4](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/复习题_83_84.md) ✅
  * [复习题 8.5 ~ 8.8](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/复习题_85_88.md) ✅
  * [复习题 8.9](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/复习题_89.md) ✅
  * [习题 P1 ~ P6【对称加密】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/习题_01_06.md) ✅
  * [习题 P7 ~ P8【非对称加密 RSA】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/习题_07_08.md)  ✅
  * [习题 P9【Diffie-Hellman 协商密钥算法】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/习题_09.md)  ✅
  * [习题 P10【使用 KDC 给 Alice 和 Bob 分发对称密钥】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/习题_10.md)  ✅
  * [习题 P11 ~ P12【书上内容理解：校验和为什么不好，提供机密性和完整性】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/习题_11_12.md)  ✅
  * [习题 P13【BitTorrent 中的完整性校验】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/习题_13.md)  ✅
  * [习题 P15 ~ P16【使用端点鉴别的注意事项】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/习题_15_16.md)  ✅
  * [习题 P17 ~ P18【安全电子邮件和 PGP】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/习题_17_18.md)  ✅
  * [习题 P19 ~ P21【SSL】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/习题_19_21.md)  ✅
  * [习题 P22 ~ P23【IPsec】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/习题_22_23.md)  ✅
  * [习题 P24【WEP】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/习题_24.md)  ✅
  * [习题 P25【传统分组过滤器，状态分组过滤器】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/习题_25.md)  ✅
  * [习题 P26【匿名与隐私】](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/习题_26.md)  ✅
  * 实验
     * [Wireshake lab - SSL](https://github.com/YangXiaoHei/Networking/blob/master/计算机网络自顶向下/08%20计算机网络中的安全/Wireshake_lab-SSL.md) ✅
