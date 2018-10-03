#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "TCP.h"
#include "log.h"
#include "common.h"
#include "tool.h"
#include <arpa/inet.h>

int sockfd;
struct packet_t packetbuf;
char sender_ip[128];
unsigned short sender_port;

enum receiver_state current_state = receiver_state_wait_0;

void rdt_send(int seq);
void udt_send(struct packet_t *packet);
void rdt_recv(struct packet_t *packet);

void rdt_send(int seq)
{
    /* 清空缓冲区内容 */
    bzero(&packetbuf, sizeof(packetbuf));

    /* 将数据打包 */
    struct packet_t *packet = &packetbuf;
    packet->isACK = 1;
    packet->seq = seq;
    bzero(packet->data, sizeof(packet->data));
    packet->checksum = calculate_checksum(packet->data, sizeof(packet->data));
    
    /* 经由不可靠信道传输 */
    udt_send(packet);
}

void udt_send(struct packet_t *packet)
{
    /* 为了简化该仿真程序，
       直接用不发包来当作丢包效果 */
    if (probability(0.2))
    {
        LOG("ACK %d is lost!", packet->seq);
        return;
    } 

    /* 产生 1 比特的差错 */
    if (probability(0.3)) 
    {
        gen_one_bit_error((char *)packet->data, sizeof(packet->data));
        LOG("ACK %d is corrupt!", packet->seq);
    }

    /* 经由可靠信道传输 */
    TCP_send(sockfd, (char *)packet, sizeof(struct packet_t));
}

void rdt_recv(struct packet_t *packet)
{   
    /* 如果没有收到一个完整的包, 当作损坏 */
    if (TCP_recv(sockfd, (char *)packet, sizeof(struct packet_t)) != sizeof(struct packet_t))
    {
        LOG("incomplete packet! exit directly");
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
    if (argc != 2)
    {
        LOG("usage : %s <#bind_port>", argv[0]);
        exit(1);
    }
    
    unsigned short bind_port = atoi(argv[1]);

    int listenfd = TCP_service_init(bind_port);
    struct sockaddr_in senderaddr;
    bzero(&senderaddr, sizeof(senderaddr));
    socklen_t addlen = sizeof(senderaddr);
    if ((sockfd = accept(listenfd, (struct sockaddr *)&senderaddr, &addlen)) < 0)
    {
        ERRLOG("accept error!");
        exit(1);
    }
    if (inet_ntop(AF_INET, &senderaddr.sin_addr, sender_ip, sizeof(sender_ip)) == NULL)
    {
        ERRLOG("inet_ntop error");
        exit(1);
    }
    sender_port = ntohs(senderaddr.sin_port);
    LOG("accept conn from [%s:%d] succ!", sender_ip, sender_port);

    while (1)
    {
        switch (current_state)
        {
            case receiver_state_wait_0 :
            {
                LOG("------- wait packet 0 begin --------");

            reentry_current_wait_0:
                rdt_recv(&packetbuf);

                if (corrupt(&packetbuf) || packetbuf.seq == 1)
                {
                    if (packetbuf.seq == 1)
                        LOG("receive a packet 1 at wait 0!");
                    else
                        LOG("receive a corrupt packet at wait 0!");

                    LOG("retransmit ACK 1 for sender");
                    rdt_send(1);
                    goto reentry_current_wait_0;
                }
                else
                {
                    LOG("receive a valid packet %d", packetbuf.seq);
                    LOG("send ACK 0 for sender");
                    rdt_send(0);
                    current_state = receiver_state_wait_1;
                }

                LOG("------- wait packet 0 end --------\n");

            } break;

            case receiver_state_wait_1 :
            {
                LOG("------- wait packet 1 begin --------");

            reentry_current_wait_1:
                rdt_recv(&packetbuf);

                if (corrupt(&packetbuf) || packetbuf.seq == 0)
                {
                    if (packetbuf.seq == 0)
                        LOG("receive a packet 0 at wait 1!");
                    else
                        LOG("receive a corrupt packet at wait 1!");

                    LOG("retransmit ACK 0 for sender");
                    rdt_send(0);
                    goto reentry_current_wait_1;
                }
                else
                {
                    LOG("receive a valid packet %d", packetbuf.seq);
                    LOG("send ACK 1 for sender");
                    rdt_send(1);
                    current_state = receiver_state_wait_0;
                }

                LOG("------- wait packet 1 end --------\n");

            } break;

            default :
            {
                LOG("unexpected stat!");
                exit(1);
            }
        }
    }

    return 0;
}