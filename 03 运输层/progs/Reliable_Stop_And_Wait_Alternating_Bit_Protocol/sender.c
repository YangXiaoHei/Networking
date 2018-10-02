#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "TCP.h"
#include "log.h"
#include "tool.h"
#include "common.h"

struct packet_t packetbuf;
int sockfd;
unsigned long start_timestamp;

enum sender_state current_state = sender_wait_0;

int timeout(void);
void start_timer(void);
void rdt_send(char *data, size_t datalen, int seq);
void udt_send(struct packet_t *packet);
int rdt_recv(struct packet_t *packet);

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
    struct packet_t *packet = &packetbuf;

    /* 获取待发送数据长度 */
    size_t maxlen = sizeof(packet->data);
    size_t validlen = datalen > maxlen ? maxlen : datalen;

    /* 将数据打包 */
    packet->isACK = 0;
    packet->seq = seq;
    packet->checksum = calculate_checksum(packet->data, validlen);
    memcpy(packet->data, data, validlen);

    /* 经由不可靠信道传输 */
    udt_send(packet);
}

void udt_send(struct packet_t *packet)
{
    /* 0.7 概率损坏，产生 1 比特的差错 */
    if (probability(0.7)) 
        gen_one_bit_error((unsigned char *)packet->data, sizeof(packet->data));

    /* 0.5 概率丢包，为了简化该仿真程序，
       直接用不发包来当作丢包效果 */
    if (probability(0.5)) 
        return;

    /* 经由可靠信道传输 */
    TCP_send(sockfd, (char *)packet, sizeof(struct packet_t));
}

void rdt_recv(struct packet_t *packet)
{   
    /* 如果没有收到一个完整的包, 当作损坏 */
    if (TCP_recv(sockfd, (char *)packet, sizeof(struct packet_t)) != sizeof(struct packet_t))
    {
        LOG("收到不完整的 packet 包! 为了简化判断，禁止这种事发生...");
        exit(1); /* 直接退出 */
    }
}

int checkcorrupt(struct packet_t *packet)
{
    /* 如果校验和不通过, 当作损坏 */
    char tmp[sizeof(packet->data) + 2];
    memcpy(tmp, packet->data, sizeof(packet->data));
    memcpy(tmp + sizeof(packet->data), &packet->checksum, 2);
    if (calculate_checksum(tmp, sizeof(tmp)) != 0)
        return 0;

    /* 无损坏 */
    return 1;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        LOG("usage : %s <#receiver_ip> <#receiver_port>", argv[0]);
        exit(1);
    }

    init_rand();

    /* 与接收方建立连接 */
    const char *receiver_ip = argv[1];
    unsigned short receiver_port = atoi(argv[2]);
    sockfd = TCP_connect(receiver_ip, receiver_port);

    long data = 0x1234567887654321L;

    while (1)
    {
        switch (current_state) 
        {
            /* wait 0 */
            case sender_wait_0:
            {
                LOG("sender enter wait 0...");
                sleep(yh_random(1, 3));

                LOG("sender send packet 0");
                rdt_send((char *)&data, sizeof(data), 0);

                current_state = sender_wait_ACK_0;
            } 
            break;

            /* wait ACK 0 */
            case sender_wait_ACK_0:
            {
                LOG("sender enter wait ACK 0...");

            wait_ACK_0_timeout_again:
                while (!timeout())
                {
                    /* TODO 将 TCP_recv 中的接收 n 字节才退出的逻辑删除，把忙轮询裸露在此处 */

                    if (!rdt_recv(&packetbuf) || (packetbuf.isACK && packetbuf.seq == 1))
                    {
                        /* do nothing, wait timeout */
                        if (packetbuf.isACK && packetbuf.seq == 1)
                            LOG("sender receive a ACK 1");

                        if ()
                    }
                    else
                    {
                        current_state = sender_wait_1;
                        goto wait_ACK_0_end;
                    }
                }
                rdt_send((char *)&data, sizeof(data), 0);
                start_timer();
                current_state = sender_wait_ACK_0;
                goto wait_ACK_0_timeout_again;

            wait_ACK_0_end:
                LOG("haha");
            }; 
            break;

            /* wait 1 */
            case sender_wait_1:
            {
                LOG("sender enter wait 1...");
                sleep(yh_random(1, 3));

                rdt_send((char *)&data, sizeof(data), 1);

                current_state = sender_wait_ACK_1;
            }; 
            break;

            /* wait ACK 1 */
            case sender_wait_ACK_1:
            {
                LOG("sender enter wait ACK 1...");

            wait_ACK_1_timeout_again:
                while (!timeout())
                {
                    if (!rdt_recv(&packetbuf) || packetbuf.seq == 0)
                    {
                        /* do nothing, wait timeout */
                    }
                    else
                    {
                        current_state = sender_wait_0;
                        goto wait_ACK_1_end;
                    }
                }
                rdt_send((char *)&data, sizeof(data), 0);
                start_timer();
                current_state = sender_wait_ACK_1;
                goto wait_ACK_1_timeout_again;

            wait_ACK_1_end:
                LOG("haha");
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