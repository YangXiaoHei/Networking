* [TimeoutConnectClientBySignal.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/TimeoutConnectClientBySignal.c) 使用 SIGALRM 给 connect 设置超时有两个缺陷:
  * 1、使用 SIGALRM 只能缩短超时时间，因为 connect 有一个内置的超时 75 s。
  * 2、信号与多线程的交互很困难，因此只在单线程化的程序中使用该技术。
  * 3、覆盖了原有的 alarm 设置，这一点在习题 [Practise_02_client.c](https://github.com/YangXiaoHei/Networking/blob/master/UNP/14%20高级%20IO%20函数/progs/Practise_02_client.c) 已经修复。

* ⚠️⚠️⚠️ UNPv1 第 3 版书中所说的套接字选项 `IP_RECVDSTADDR` 在 linux 内核 `GNU/Linux 4.15.0-42-generic x86_64` 中不存在，取而代之的是 `IP_RECVORIGDSTADDR ` 选项，切记！

* 对于套接字描述符，不要搭配标准 IO 库一起使用，会遇到缓冲问题（其实可以解决）。