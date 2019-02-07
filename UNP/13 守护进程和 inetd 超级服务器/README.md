* 守护进程的启动方式
   * 从 `/etc/rc` 下的某个脚本启动，比如 `inetd` `xinetd` `web 服务器`
 `sendmail` `cron` `syslogd`
   * 被用户使用命令行启动
   * `cron` 定时执行的任务，这些任务作为守护进程被启动。
   * 当有网络请求到达时，`inetd` 或 `xinted` 启动相应的服务器，这些服务器作为守护进程被启动。比如 daytime, echo, discard, time, chargen
   
* syslogd 守护进程启动步骤
  * 读取配置文件, 在我的主机上，配置文件路径是 `/etc/rsyslog.conf` 和 `/etc/rsyslog.d/xxx` 等一系列文件。
  * 1⃣️创建 Unix 域数据报套接字，给它绑定路径名 `/var/run/log` 
  * 2⃣️创建 UDP 套接字，绑定端口 514，从 `/etc/services` 中可以查看到 514 是 syslog 服务使用的 udp 端口。
  * 3⃣️打开路径名 `/dev/klog`。准备写入来自内核中的错误信息。
  * 然后在一个无限循环中调用 select 以等待上述 3 个描述符的可读事件，然后按照配置文件将其写入到指定日志文件。
  * 如果 syslogd 守护进程收到 `SIGHUP` 信号，就重新读取配置文件。

* 在应用程序中想要使用 syslog 时
  * 在我的主机上，日志文件被写入到 `/var/log/xxx` 下的某个文件，不用特别纠结，直接 grep 一下我打印的内容就能找到具体写入到哪个文件。
  * openlog 时指定 `LOG_PID` 可以在写日志时加上进程号信息。
  
* `daemon_init` 函数
   * fork 第一次，杀死父进程，这一步目的是为了让子进程不是进程组组长
   * setsid 让子进程成为新会话的头进程。
   * 忽略信号 SIGHUP，因为会话头进程被杀死会给会话中所有进程发送该信号。
   * fork 第二次，杀死会话头进程，这一步目的是防止会话的头进程不小心取得控制终端。
   * 切换工作路径到根路径 `/` 
   * 关闭所有文件描述符
   * 在 `/dev/null` 上打开 0 只读、1 读写、2 读写。
   * openlog 打开日志文件

* `xinetd` 工作流程
   * 读取配置文件，`/etc/xinetd.conf` `/etc/xinetd.d/xxx`， 对其中每个列出的服务，socket, bind, 如果是 tcp 还要 listen
   * select 所有上一步创建的套接字。
   * 不论是 tcp 还是 udp 套接字可读。 fork 子进程，并且关闭所有描述符，然后 dup2(connfd, 0), dup2(connfd, 1), dup2(connfd, 2)。最后 exec 相应的服务器。
   * 如果是 tcp 套接字可读而 exec 的服务器子进程，那么只能通过 getpeername 来取得客户端的套接字地址。

* 要让 `xinetd` 启动自己的服务器程序，需要先在 `/etc/services` 中添加比如 
`hansondaytime 50000/udp;  hansondaytime 50000/tcp`。然后再在 `/etc/xinetd.d/` 目录下添加自己的服务，先 cp 其他的来改一下就可以。最后 `/etc/init.d/xinetd restart` 就可以，可以通过 `netstat -an | grep ${port}` 来检查自己程序的端口是否已经处于 listen。


   
   
    