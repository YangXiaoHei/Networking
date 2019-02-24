#ifndef _RELIABLE_UDP_REVIEW_H_
#define _RELIABLE_UDP_REVIEW_H_

struct rtt_info {
    float rtt_rtt; /* 当次测量的RTT */ 
    float rtt_srtt; /* 平滑过渡的估算 RTT */
    float rtt_rttvar; /* 波动 */
    float rtt_rto; /* 计算得到的本次超时估计 */
    long rtt_base; /* 发送一次 udp 报文的开始时间戳 */
    int rtt_nretrans; /* 重传的次数 */
};

#define RTT_MIN 2
#define RTT_MAX 60
#define RTT_MAXNTRANS 4

int rtt_start(struct rtt_info *);
void rtt_newpack(struct rtt_info *);
int rtt_timeout(struct rtt_info *);
void rtt_init(struct rtt_info *);
void rtt_stop(struct rtt_info *, long ms);
long rtt_ts(struct rtt_info *);

#endif
