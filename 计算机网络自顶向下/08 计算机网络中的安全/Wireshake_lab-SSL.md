在这个实验中，我们研究 SSL 协议，重点放在基于 TCP 所发送的 SSL 记录项，我们分析在你的电脑和一个电子商业服务器之间发送的 SSL 记录项轨迹，我们将研究不同种 SSL 记录项同时也观察这些 SSL 报文里的字段的值。在此之前，你也许想温习课本 8.6 节的有关内容。

* **1、捕获 SSL 会话中的分组**

第一步是捕获一个 SSL 会话中的分组。为了做到这一点，你将去往你最喜欢的电子商务站点并开始购买物品的流程（但在最终付款前住手！）使用 Wireshake 捕获分组后，你应该在筛选栏中填写 SSL，这样只会显示你电脑收到和发送的包含 SSL 记录项的分组（SSL 记录项和 SSL 报文是一回事）。你应该会得到一个类似于上面截图所示的分组轨迹。

如果你难以制造一个类似的分组轨迹，你应该下载 [wireshark-traces.zip](http://gaia.cs.umass.edu/wireshark-labs/wireshark-traces.zip) 并导出 `ssl-ethereal-trace-1` 分组。

* **2、观察这个被捕获的分组轨迹**

你的 Wireshake 界面应该只展示含有 SSL 记录项的帧，请记住很重要的一点，一个以太网帧中可能包含一个或多个 SSL 记录项。（这非常不同于 HTTP 报文，在 HTTP 报文中要么包含一个完整的 HTTP 报文，要么包含 HTTP 报文的一部分）。同样的，一个 SSL 记录项也可能不能完整地被塞进一个以太网帧，在这种情况下多个帧会携带该 SSL 记录项。

回答下列问题:

   * 1、对于最开始的 8 个以太网帧，指定帧的源（客户端或者服务器），指出在这些帧中包含了多少个 SSL 记录项，并列出 SSL 记录项的类型。画出客户端和服务器使用每个 SSL 记录项通信的时序图。
   
   * ![](https://github.com/YangXiaoHei/Networking/blob/master/08%20计算机网络中的安全/image/wl_ssl_1.png)

   * 2、每个 SSL 记录项以三个相同的字段开头（可能填写了不同的值）。这些字段中的一个 "Content Type" 有一个字节，列出所有这三个字段以及它们的长度。
   
   |帧| SSL Record Type| Content Type| Version| Length|
   |:---:|:---:|:---:|:---:|:---:|
   |159|Client Hello|Handshake (22)|TLSv1.2|298|
   |162|Server Hello|Handshake (22)|TLSv1.2|150|
   |166|Certificate|Handshake (22)|TLSv1.2|680|
   |170|Server Key Exchange|Handshake (22)|TLSv1.2|392|
   |171|Server Hello Done|Handshake (22)|TLSv1.2|63|
   |174|`Client Key Exchange`、`Change Cipher Spec`、`Encrypted Handshake Message`|`Handshake(22)`、`Change Cipher Spec(20)`、`Handshake(22)`|TLSv1.2|180|
   |176|Change Cipher Spec|Change Cipher Spec|TLSv1.2|60|
   |177|Encrypted Handshake Message|Handshake (22)|TLSv1.2|99|
   |181|http-over-tls|Application Data(23)|TLSv1.2|383|
   |185|http-over-tls|Application Data(23)|TLSv1.2|443|
   |187|http-over-tls|Application Data(23)|TLSv1.2|385|
   
* **ClientHello Record:**

   * 3、展开 ClientHello 记录项 (如果你的轨迹有多个帧包含 ClientHello，展开第一个帧)，content type 的值是什么？
      * ![](https://github.com/YangXiaoHei/Networking/blob/master/08%20计算机网络中的安全/image/wl_ssl_2.png)
      
   * 4、这个 ClientHello 是否包含一个不重数？如果是，这个不重数的 16 进制表达是什么？
      * 随机数如下图所示
      * ![](https://github.com/YangXiaoHei/Networking/blob/master/08%20计算机网络中的安全/image/wl_ssl_3.png)
      
   * 5、这个 ClientHello 记录项是否告知了它支持的加密算法？如果是，在第一个展开的套件中，公开密钥算法是什么？对称密钥算法是什么？hash 算法是什么？
      * ![](https://github.com/YangXiaoHei/Networking/blob/master/08%20计算机网络中的安全/image/wl_ssl_4.png)
      
* **ServerHello Record:**

   * 6、定位 ServerHello SSL 记录项，这个记录项是否选择了一个加密算法套件？它选择的加密算法套件是什么?
      * 服务器选择加密算法如下图所示
      * ![](https://github.com/YangXiaoHei/Networking/blob/master/08%20计算机网络中的安全/image/wl_ssl_5.png)

   * 7、这个记录项是否包含一个不重数？如果包含的话，有多少字节长？在 SSL 中客户端和服务器包含不重数的目的是什么？
      * 在 ServerHello 中包含了一个不重数，如下图所示
      * ![](https://github.com/YangXiaoHei/Networking/blob/master/08%20计算机网络中的安全/image/wl_ssl_6.png)
      * 这个不重数总共有 32 字节，使用不重数的目的是防止 “连接重放”。

   * 8、这个记录项是否包含一个 Session ID？使用 Session ID 的目的是什么？
      * 包含了 Session ID，如下图所示
      * ![](https://github.com/YangXiaoHei/Networking/blob/master/08%20计算机网络中的安全/image/wl_ssl_7.png)
      * Session ID 的目的是恢复会话，它减少了用来创建一个新的 Session ID 而耗费时间的握手步骤的数量。如果在 ClientHello 记录项中的 Session ID 不是 0，那么这代表客户端想要继续先前已经建立的会话，如果是 0，那么代表客户端想要与服务器建立一个新的会话。
      
   * 9、这个记录项是否包含一个证书？或者证书包含在一个单独的记录项中？证书是否能被完整的塞到一个单独的以太网帧中?
      * ![](https://github.com/YangXiaoHei/Networking/blob/master/08%20计算机网络中的安全/image/wl_ssl_8.png)
      * 从上图可以看出，这个证书包含在一个单独的记录项中，这个证书不能被完整的塞带一个以太网帧中，而是被分为三个 TCP 报文段来承载。

⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️        
⚠️ 自己电脑捕获的 SSL 分组的加密算法看不懂，下面开始使用教材提供的轨迹 ⚠️
⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️⚠️

* **Client Key Exchange Record:**
   * 10、定位 Client Key Exchange 记录项，这个记录项是否包含一个 PMS？这个密钥是用来干嘛的？这个密钥是否被加密？如果是，如何加密的？这个被加密的密钥有多长？
      * 从下图可以看出，包含一个 PMS，该密钥用来让客户端和服务器共同导出 4 个密钥。这个 PMS 被服务器的公钥加密，总共有 128 字节长。 
      * ![](https://github.com/YangXiaoHei/Networking/blob/master/08%20计算机网络中的安全/image/wl_ssl_9.png)
     
* **Change Cipher Spec Record (sent by client) and Encrypted Handshake Record:**     

   * 11、Change Cipher Sepc 记录项的目的是啥？在你的分组轨迹中该记录项有多少字节？
      * 目的是通知对方接下来要使用加密的方式传输报文咯，在我的分组轨迹中，这个记录项有 1 字节长。

   * 12、在加密的握手报文记录项中，被加密的是什么？如何加密的？
      * Encrypted Handshake Record 被加密的是所有客户端和服务器的所有握手报文，使用 MD5 和 SHA 两个摘要算法将各自计算出的哈希值连在一块，该加密报文用来验证 Key Exchange 和认证阶段是成功的。

   * 13、服务器是否也发送了一个 Change Cipher Record 和一个 Encrypted Handshake Record 给客户端？这些记录项和客户端发送的有什么不同？
      * 是的，服务器同样给客户端发送了 Change Cipher Record 和 Encrypted Handshake Record，这些报文只有发送者不同，一会是客户端，一会是服务器。
      
* **Application Data**
  * 14、application data 如何被加密？这些包含 application data 的记录项是否包含 MAC，Wireshake 能分辨出加密应用数据和 MAC 吗？
     * 应用数据被服务器所选择的加密套件中的特定加密算法所加密，在这个分组轨迹中，使用的是 CBC AES 加密应用数据和 MAC，而 MAC 使用 SHA 算法生成。但是 Wireshake 并不能分辨出应用数据和 MAC。 
    
  * 15、注释以及解释你在轨迹中发现的好玩的地方。

      

   