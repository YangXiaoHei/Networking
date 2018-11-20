#include "comm.h"

#define ARRSIZE(_arr_) (sizeof(_arr_) / sizeof(_arr_[0]))

int sockfd;
struct packet_t my_packets[10];
struct packet_t packetbuf;

unsigned short sender_base = 0;
unsigned short next_seqnum = 0;

int yytimeout(void);
void init_data(void);
void start_timer(void);
int  rdt_send(int seq);
int  udt_send(struct packet_t *packet);
void rdt_recv(struct packet_t *packet, ssize_t offset);

void init_data(void)
{
    for (int i = 0; i < ARRSIZE(my_packets); i++)
    {
        my_packets[i].seq = i;
        my_packets[i].isACK = 0;
        my_packets[i].isACKed = 0;
        my_packets[i].isTransmited = 0;
        unsigned long data = 0x1234567887654321;
        memcpy(&my_packets[i].data, &data, sizeof(data));
        my_packets[i].timeout_timestamp = 0;
        my_packets[i].checksum = calculate_checksum(my_packets[i].data, sizeof(my_packets[i].data));
    }
}

int rdt_send(int seq)
{
    /* 经由不可靠信道传输 */
    my_packets[seq].seq = seq;
    my_packets[seq].timeout_timestamp = curtime_us() + TIMEOUT_INTERVAL;
    my_packets[seq].isTransmited = 1;
    my_packets[seq].checksum = calculate_checksum((char *)&my_packets[seq].data, sizeof(my_packets[seq].data));
    return udt_send(&my_packets[seq]);
}

int udt_send(struct packet_t *packet)
{
    int retcode = 0;
    
    /* 用不发包来当作丢包效果 */
    if (probability(0.2)) {
        retcode = -1;
        goto dismiss;
    }

    /* 产生 1 比特的差错 */
    if (probability(0.3)) {
        gen_one_bit_error((char *)packet->data, sizeof(packet->data));
        retcode = -2;
        goto biterror;
    }

biterror:
    /* 经由可靠信道传输 */
    TCP_send(sockfd, (char *)packet, sizeof(struct packet_t));
dismiss:
    return retcode;
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

int check_all_finished(void)
{
    for (int i = 0; i < ARRSIZE(my_packets); i++)
        if (!my_packets[i].isACKed)
            return 0;
    return 1;
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
    ssize_t nread;
    while (1) 
    {
        received_one = 0;

        /* 如果所有发送分组都已被确认，那么跳出循环 */
        if (check_all_finished())
        {
            LOG("all packet transmit finished!");
            break;
        }

        /* 先读 4 个字节，看读不读得到 */
        setflags(sockfd, O_NONBLOCK);
        if ((nread = read(sockfd, &packetbuf, 4)) > 0)
        {
            /* 如果能读到，就把完整的包读出来 */
            rdt_recv(&packetbuf, nread);
            received_one = 1;
        }
        clrflags(sockfd, O_NONBLOCK);

        /* 如果收到一个损坏的包，或者是对分组 1 的应答，那么啥都不做 */
        if (received_one)
        {
            if (corrupt(&packetbuf))
                LOG("receive a corrupt ACK while waiting ACK %d!", sender_base);
            else
            {
                if (packetbuf.seq >= sender_base)
                {
                    for (int i = sender_base; i <= packetbuf.seq; i++)
                    {
                        my_packets[i].isACKed= 1;
                        my_packets[i].timeout_timestamp = 0;
                        my_packets[i].isTransmited = 0;
                    }
                    sender_base = packetbuf.seq + 1;
                    LOG("receive a valid ACK %d, move window base to %d ✅", packetbuf.seq, sender_base);
                }
                else
                    LOG("ignore ACK %d, [base=%d][nextseq=%d]",packetbuf.seq, sender_base, next_seqnum);
            }
        }

        /* 判断是否应该重传 */   
        for (int i = sender_base; i <= min(sender_base + SENDER_WIN_SZ, ARRSIZE(my_packets)); i++)
        {
            if (my_packets[i].isTransmited && curtime_us() > my_packets[i].timeout_timestamp)
            {
                int retcode = rdt_send(i);
                switch (retcode)
                {
                    case -1:
                    {
                        LOG("[timeout]!! retransmit packet %d dismiss!! [base=%d][nextseq=%d] ❌", i, sender_base, next_seqnum);
                    } break;
                    case -2:
                    {
                        LOG("[timeout]!! retransmit packet %d biterror!! [base=%d][nextseq=%d] ❌", i, sender_base, next_seqnum);
                    } break;
                    default:
                    {
                        LOG("[timeout]!! retransmit packet %d [base=%d][nextseq=%d]", i, sender_base, next_seqnum);
                    } break;
                }
            }
        }

        if (next_seqnum >= ARRSIZE(my_packets))
            continue;
    
        while (next_seqnum < min(sender_base + SENDER_WIN_SZ, ARRSIZE(my_packets)))
        {
            int retcode = rdt_send(next_seqnum++);
            switch (retcode)
            {
                case -1:
                {
                    LOG("send packet %d dismiss!! [base=%d][nextseq=%d] ❌", next_seqnum - 1, sender_base, next_seqnum);
                } break;
                case -2:
                {
                    LOG("send packet %d biterror!! [base=%d][nextseq=%d] ❌", next_seqnum - 1, sender_base, next_seqnum);
                } break;
                default:
                {
                    LOG("send packet %d [base=%d][nextseq=%d]", next_seqnum - 1, sender_base, next_seqnum);
                } break;
            }
        }
    }
    
    return 0;
}
