* TCP 新认识
   * TCP 并不保证数据一定被对端所接收，因为这是不可能做到的。如果有可能，TCP 就把数据传送到对方端点，否则就（通过放弃重传并中断连接这一手段）通知用户。因此 TCP 并不能被描述为 100% 可靠，它提供的是数据的可靠递送或故障的可靠通知。

* TCP 选项
   * MSS 选项
      * 发送 SYN 的 TCP 一端使用本选项通告它的 MSS，即它在本连接的每个 TCP 报文段愿意接收的最大数据量。发送端 TCP  使用接收端的 MSS 作为所发送报文段的最大大小。可以利用 `TCP_MAXSEG` 套接字选项设置和提取该 TCP 选项。
      
   * 窗口规模选项
      * TCP 首部中的接收窗口字段最大只能表示 65536 字节，而在当今高速网络连接或长延迟路径（卫星链路）要求有更大的窗口以获得尽可能大的吞吐量。这个新选项指定 TCP 首部中的接收窗口必须左移的位数 (0 ~ 14)，因此所能提供的最大窗口接近 1 GB `(65536 x 2^14)`
      
   * 时间戳选项
      * 防止发生无穷计数问题导致的失而复现的分组可能造成的数据损坏。作为网络编程人员，无需考虑该选项。

* TCP 状态转换图

   ![](https://github.com/YangXiaoHei/Networking/blob/master/UNP/02%20传输层%20TCP%20UDP%20和%20SCTP/images/TCP_state_change.png)
   
   * ⚠️ 主动打开的状态有哪些？
      * `SYN_SENT` `ESTABLISHED` `CLOSED`

   * ⚠️ 被动打开的状态有哪些？
      * `LISTEN` `SYN_RCVD` `ESTABLISHED`
      
   * ⚠️ 主动关闭的状态？
      * `FIN_WAIT_1` `FIN_WAIT_2` `TIME_WAIT` `CLOSING`
      
   * ⚠️ 被动关闭的状态？
      * `CLOSE_WAIT` `LAST_ACK`
      
   * ⚠️ 同时打开，同时关闭
      * 主动打开方在 `SYN_SENT` 时收到 `SYN` 分组，那么主动打开方发送 `SYN ACK` 后切换为被动打开
      
      * 主动关闭方在 `FIN_WAIT_1` 时收到 `FIN` 分组，那么主动关闭方发送 `ACK` 后进入 `CLOSING` 状态，等待自己发送的 `FIN` 的 `ACK`
      
      * 主动关闭方在 `FIN_WAIT_1` 时收到 `FIN ACK` 分组，说明双方都没有数据要发送了，那么主动关闭方发送 `ACK` 后进入 `TIME_WAIT` 状态
     
   * ⚠️ 被动打开时收到 RST
      * 被动打开方在 `SYN_RCVD` 时收到 `RST`，说明主动打开方异常关闭，那么被动打开方恢复到 `CLOSED` 状态