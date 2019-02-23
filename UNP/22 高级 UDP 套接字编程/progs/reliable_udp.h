#ifndef _RELIABLE_UDP_H_
#define _RELIABLE_UDP_H_

struct rtt_info {
	float rtt_rtt;  /* 测量得出的往返时间 */
	float rtt_srtt; /* 估算的往返时间 */
	float rtt_rttvar; /* 波动 */
	float rtt_rto; /* 计算得出的超时估计值 rtt_rto = rtt_srtt + 4  * rtt_rttvar */
	int rtt_nretrans; /* 重传的次数 */
	long rtt_base;  /* 开始的时间点 */
};

#define RTT_RXTMIN 2
#define RTT_RXTMAX 60
#define RTT_MAXNRETRANS 4
/*
 * 初始化控制信息
 */
void rtt_init(struct rtt_info *);

/*
 * 将重传次数置为 0
 */
void rtt_newpack(struct rtt_info *);
/*
 * 将 rto 向上舍入到整数
 */
int rtt_start(struct rtt_info *);
/*
 * 结束一次收发，重新计算 rtt, rttvar rto
 */
void rtt_stop(struct rtt_info *, long);
/*
 * 当发生超时时，该函数负责执行指数退避，累加重传次数，若达到阈值则报错
 */
int rtt_timeout(struct rtt_info *);
/*
 * 返回本次 udp 发送的相对时间戳
 */
long rtt_ts(struct rtt_info *);

#endif
