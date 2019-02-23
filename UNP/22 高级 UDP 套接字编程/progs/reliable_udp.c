#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "reliable_udp.h"
#include "../../tool/TimeTool.h"

#define RTT_RTOCALC(ptr) ((ptr)->rtt_srtt + 4.0 * (ptr)->rtt_rttvar)

static int rtt_minmax(float rto)
{
    if (rto > RTT_RXTMAX)
        return RTT_RXTMAX;
    if (rto < RTT_RXTMIN)
        return RTT_RXTMIN;
    return rto;
}

/*
 * 初始化控制信息
 */
void rtt_init(struct rtt_info *ptr)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ptr->rtt_base = tv.tv_sec;     /* seconds since 1970 */
    ptr->rtt_rtt = 0;
    ptr->rtt_srtt = 0;
    ptr->rtt_rttvar = 0.75;
    ptr->rtt_rto = rtt_minmax(RTT_RTOCALC(ptr));
}

/*
 * 将重传次数置为 0
 */
void rtt_newpack(struct rtt_info *ptr)
{
    ptr->rtt_nretrans = 0;
}
/*
 * 将 rto 向上舍入到整数
 */
int rtt_start(struct rtt_info *ptr)
{
    return (int)(ptr->rtt_rto + 0.5);
}
/*
 * 结束一次收发，重新计算 rtt, rttvar rto
 */
void rtt_stop(struct rtt_info *ptr, long ms)
{
    double delta;

    ptr->rtt_rtt = ms / 1000;
    
    delta = ptr->rtt_rtt - ptr->rtt_srtt;
    ptr->rtt_srtt += delta / 8;

    if (delta < 0.0)
        delta = -delta;

    ptr->rtt_rttvar += (delta - ptr->rtt_rttvar) / 4;

    ptr->rtt_rto = rtt_minmax(RTT_RTOCALC(ptr));

    logx("new rto = %.2f", ptr->rtt_rto);
}
/*
 * 当发生超时时，该函数负责执行指数退避，累加重传次数，若达到阈值则报错
 */
int rtt_timeout(struct rtt_info *ptr)
{
    ptr->rtt_rto *= 2;
    logx("timeout occur, backoff -- new rto = %.2f", ptr->rtt_rto);
    if (++ptr->rtt_nretrans > RTT_MAXNRETRANS) 
        return -1;
    return 0;
}
/*
 * 返回本次 udp 发送的相对时间戳
 */
long rtt_ts(struct rtt_info *ptr)
{
    long ts;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    ts = (tv.tv_sec - ptr->rtt_base) * 1000 + (tv.tv_usec / 1000);
    return ts;
}
