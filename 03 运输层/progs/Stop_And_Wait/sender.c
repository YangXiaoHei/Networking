#include "comm.h"

int sockfd;
struct packet_t packetbuf;
unsigned long start_timestamp;

enum sender_state current_state = sender_wait_0;

int timeout(void);
void start_timer(void);
void rdt_send(char *data, size_t datalen, int seq);
void udt_send(struct packet_t *packet);
void rdt_recv(struct packet_t *packet, ssize_t offset);

int timeout(void)
{
    return (curtime_us() - start_timestamp) >= TIMEOUT_INTERVAL;
}

void start_timer(void)
{
    start_timestamp = curtime_us();
}

void rdt_send(char *data, size_t datalen, int seq)
{
    /* 清空缓冲区内容 */
    bzero(&packetbuf, sizeof(packetbuf));

    /* 将数据打包 */
    struct packet_t *packet = &packetbuf;
    packet->isACK = 0;
    packet->seq = seq;
    size_t maxlen = sizeof(packet->data);
    size_t validlen = datalen > maxlen ? maxlen : datalen;
    memcpy(packet->data, data, validlen);
    packet->checksum = calculate_checksum(packet->data, sizeof(packet->data));

    /* 经由不可靠信道传输 */
    udt_send(packet);
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

int main(int argc, char *argv[]) {

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

    long data = 0x1234567887654321L;  /* 8 字节 */

    while (1)
    {
        switch (current_state) 
        {
            /* wait 0 */
            case sender_wait_0:
            {
                LOG("-------- wait 0 begin -------");
                sleep(yh_random(3, 6));

                LOG("sender send packet 0");
                rdt_send((char *)&data, sizeof(data), 0);

                current_state = sender_wait_ACK_0;
                start_timer();
                LOG("-------- wait 0 end -------\n");
            } 
            break;

            /* wait ACK 0 */
            case sender_wait_ACK_0:
            {
                LOG("-------- wait ACK 0 begin -------");
                ssize_t nread = 0;

            wait_ACK_0_timeout_again:
                while (!timeout())
                {
                    /* 先读 4 个字节，看读不读得到 */
                    setflags(sockfd, O_NONBLOCK);
                    if ((nread = read(sockfd, &packetbuf, 4)) <= 0)
                    {
                        /* do nothing, wait timeout */
                        clrflags(sockfd, O_NONBLOCK);
                        continue;
                    }

                    /* 如果能读到，就把完整的包读出来 */
                    rdt_recv(&packetbuf, nread);

                    /* 如果收到一个损坏的包，或者是对分组 1 的应答，那么啥都不做 */
                    int wrong = 0;
                    if ((wrong = corrupt(&packetbuf)) || (packetbuf.isACK && packetbuf.seq == 1))
                    {
                        
                        if (wrong)
                            LOG("receive a corrupt ACK while waiting ACK 0!");
                        else
                            LOG("receive ACK 1 while waiting ACK 0!");

                        /* do nothing, wait timeout */
                        continue;
                    }
                    else
                    {
                        /* 收到对分组 0 的确认，进入下一状态 */
                        LOG("receive a valid ACK 0");
                        current_state = sender_wait_1;
                        goto wait_ACK_0_end;
                    }
                }
                LOG("[timeout]!! retransmit packet 0");
                rdt_send((char *)&data, sizeof(data), 0);
                start_timer();
                goto wait_ACK_0_timeout_again;

            wait_ACK_0_end:
                LOG("-------- wait ACK 0 end -------\n");
            }
            break;

            /* wait 1 */
            case sender_wait_1:
            {
                LOG("-------- wait 1 begin -------");
                sleep(yh_random(3, 6));

                LOG("sender send packet 1");
                rdt_send((char *)&data, sizeof(data), 1);

                current_state = sender_wait_ACK_1;
                start_timer();
                LOG("-------- wait 1 end -------\n");
            }
            break;

            /* wait ACK 1 */
            case sender_wait_ACK_1:
            {
                LOG("-------- wait ACK 1 begin -------");
                ssize_t nread = 0;

            wait_ACK_1_timeout_again:
                while (!timeout())
                {
                   /* 先读 4 个字节，看读不读得到 */
                    setflags(sockfd, O_NONBLOCK);
                    if ((nread = read(sockfd, &packetbuf, 4)) <= 0)
                    {
                        /* do nothing, wait timeout */
                        clrflags(sockfd, O_NONBLOCK);
                        continue;
                    }

                    /* 如果能读到，就把完整的包读出来 */
                    rdt_recv(&packetbuf, nread);

                    /* 如果收到一个损坏的包，或者是对分组 0 的应答，那么啥都不做 */
                    int wrong = 0;
                    if ((wrong = corrupt(&packetbuf)) || (packetbuf.isACK && packetbuf.seq == 0))
                    {
                        /* do nothing, wait timeout */
                        if (wrong)
                            LOG("receive a corrupt ACK while waiting ACK 1!");
                        else
                            LOG("receive ACK 0 while waiting ACK 1!");

                        /* do nothing, wait timeout */
                        continue;
                    }
                    else
                    {
                        /* 收到对分组 1 的确认，进入下一状态 */
                        LOG("receive a valid ACK 1");
                        current_state = sender_wait_0;
                        goto wait_ACK_1_end;
                    }
                }
                LOG("[timeout]!! retransmit packet 1");
                rdt_send((char *)&data, sizeof(data), 1);
                start_timer();
                goto wait_ACK_1_timeout_again;

            wait_ACK_1_end:
                LOG("------ wait ACK 1 end ---------\n");
            } 
            break;

            default:
            {
                LOG("unexpected state!");
                exit(1);
            }
        }
    }

    return 0;
}