#include "comm.h"

#define ARRSIZE(_arr_) (sizeof(_arr_) / sizeof(_arr_[0]))

int sockfd;
struct packet_t my_packets[20];
unsigned long start_timestamp;

struct packet_t packetbuf;

unsigned short sender_base = 0;
unsigned short next_seqnum = 0;

int timeout(void);
void init_data(void);
void start_timer(void);
void rdt_send(int seq);
void udt_send(struct packet_t *packet);
void rdt_recv(struct packet_t *packet, ssize_t offset);

void init_data(void)
{
    for (int i = 0; i < ARRSIZE(my_packets); i++)
    {
        my_packets[i].seq = i;
        my_packets[i].isACK = 0;
        my_packets[i].isACKed = 0;
        my_packets[i].data = 0x1234567887654321;
        my_packets[i].checksum = calculate_checksum(my_packets[i].data, sizeof(my_packets[i].data));
    }
}

int timeout(void)
{
    return (curtime_us() - start_timestamp) >= TIMEOUT_INTERVAL;
}

void start_timer(void)
{
    start_timestamp = curtime_us();
}

void rdt_send(int seq)
{
    /* 经由不可靠信道传输 */
    udt_send(&my_packets[seq]);
}

void udt_send(struct packet_t *packet)
{
    /* 为了简化该仿真程序，
       直接用不发包来当作丢包效果 */
    if (probability(0.2))
        return;

    /* 产生 1 比特的差错 */
    if (probability(0.3))
        gen_one_bit_error((char *)packet->data, sizeof(packet->data));

    /* 经由可靠信道传输 */
    TCP_send(sockfd, (char *)packet, sizeof(struct packet_t));
}

void rdt_recv(struct packet_t *packet, ssize_t offset)
{   
    /* 如果没有收到一个完整的包, 当作损坏 */
    if (TCP_recv(sockfd, (char *)packet + offset, sizeof(struct packet_t) - offset) != sizeof(struct packet_t) - offset)
    {
        LOG("incomplete packet! exit directly!");
        exit(1); /* 直接退出 */
    }
}

int corrupt(struct packet_t *packet)
{
    /* 如果校验和不通过, 当作损坏 */
    char tmp[sizeof(packet->data) + 2];
    memcpy(tmp, packet->data, sizeof(packet->data));
    memcpy(tmp + sizeof(packet->data), &packet->checksum, 2);
    if (calculate_checksum(tmp, sizeof(tmp)) != 0)
        return 1;

    /* 无损坏 */
    return 0;
}


int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        LOG("usage : %s <#receiver_ip> <#receiver_port>", argv[0]);
        exit(1);
    }

    /* 随机数 */
    init_rand();

    /* 与接收方建立连接 */
    const char *receiver_ip = argv[1];
    unsigned short receiver_port = atoi(argv[2]);
    sockfd = TCP_connect(receiver_ip, receiver_port);
    LOG("connect to receiver succ!");

    int received_one = 0;
    while (1) 
    {
        received_one = 0;

        /* 先读 4 个字节，看读不读得到 */
        setflags(sockfd, O_NONBLOCK);
        if ((nread = read(sockfd, &packetbuf, 4)) > 0)
        {
            /* 如果能读到，就把完整的包读出来 */
            rdt_recv(&packetbuf, nread);
            clrflags(sockfd, O_NONBLOCK);
            received_one = 1;
        }

        /* 如果收到一个损坏的包，或者是对分组 1 的应答，那么啥都不做 */
        if (received_one)
        {
            if (corrupt(&packetbuf))
            {   
                LOG("receive a corrupt ACK while waiting ACK %d!", sender_base);
            }
            else
            {
                sender_base = packetbuf.seq + 1;
                LOG("receive a valid ACK for packet %d", packetbuf.seq);
                LOG("move window base to %d", sender_base);
            }
        }

        /* 这里应该要有超时判断 */   
    
        /* 不发送 */
        if (next_seqnum > sender_base + SENDER_WIN_SZ)
        {
            LOG("there are avaliable packets to send [base=%d][nextseq=%d]", sender_base, next_seqnum);
        }
        else
        {
            // for (int i = next_seqnum; i <= sender_base + SENDER_WIN_SZ; i++)
            // {
            //     rdt_send(i);
            //     LOG("send packet %d [base=%d][nextseq=%d]", i, sender_base, next_seqnum);
            // }
        }
    }
    
    return 0;
}