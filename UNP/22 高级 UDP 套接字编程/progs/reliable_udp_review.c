#include <sys/time.h>
#include "reliable_udp_review.h"
#include "../../tool/TimeTool.h"

#define RTT_RTOCALC(ptr) \
    ((ptr)->rtt_srtt + 4 * (ptr)->rtt_rttvar)

float rtt_minmax(float rto)
{
    if (rto < RTT_MIN)
        return RTT_MIN;
    if (rto > RTT_MAX)
        return RTT_MAX;
    return rto; 
}

int rtt_start(struct rtt_info *rtt)
{
    return (int)(rtt->rtt_rto + 0.5);
}
void rtt_newpack(struct rtt_info *rtt)
{
    rtt->rtt_nretrans = 0;
}
int rtt_timeout(struct rtt_info *rtt)
{
    rtt->rtt_rto *= 2;
    logx("timeout occurr, rto backoff -> %.2f, cur retrans = %d", rtt->rtt_rto, rtt->rtt_nretrans);
    if (++rtt->rtt_nretrans > RTT_MAXNTRANS)
        return -1;
    return 0;
}
void rtt_init(struct rtt_info *rtt)
{
    rtt->rtt_rtt = 0;
    rtt->rtt_srtt = 0;
    rtt->rtt_rttvar = 0.75;
    rtt->rtt_rto = rtt_minmax(RTT_RTOCALC(rtt));
    rtt->rtt_nretrans = 0;
    
    struct timeval tv;
    gettimeofday(&tv, NULL);
    rtt->rtt_base = tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
void rtt_stop(struct rtt_info *rtt, long ms)
{
    double delta;
    
    rtt->rtt_rtt = ms / 1000;
    delta = rtt->rtt_rtt - rtt->rtt_srtt;
    rtt->rtt_srtt += delta / 8;

    if (delta < 0)
        delta =  -delta;

    rtt->rtt_rttvar += (delta - rtt->rtt_rttvar) / 4;
    rtt->rtt_rto = rtt_minmax(RTT_RTOCALC(rtt));
    logx("new rto = %.2f", rtt->rtt_rto);
}

long rtt_ts(struct rtt_info *rtt)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000) - rtt->rtt_base;
}
