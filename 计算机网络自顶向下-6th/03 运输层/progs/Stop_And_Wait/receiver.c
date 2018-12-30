#include "comm.h"

int sockfd;
struct packet_t packetbuf;
char sender_ip[128];
unsigned short sender_port;

enum receiver_state current_state = receiver_state_wait_0;

int  rdt_send(int seq);
int  udt_send(struct packet_t *packet);
void rdt_recv(struct packet_t *packet);

int rdt_send(int seq)
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
    return udt_send(packet);
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

void rdt_recv(struct packet_t *packet)
{   
    /* 如果没有收到一个完整的包, 直接退出 */
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

            reentry_wait_0:
                rdt_recv(&packetbuf);

                int wrong = 0;
                if ((wrong = corrupt(&packetbuf)) || packetbuf.seq == 1)
                {
                    if (wrong)
                        LOG("receive a corrupt packet while waiting 0!");
                    else
                        LOG("receive packet 1 while waiting 0!");

                    int retcode = rdt_send(1);
                    switch (retcode)
                    {
                        case -1:
                        {
                            LOG("retransmit ACK 1 for sender dismiss!!  ❌");
                        } break;
                        case -2:
                        {
                            LOG("retransmit ACK 1 for sender biterror!! ❌");
                        } break;
                        default:
                        {
                            LOG("retransmit ACK 1 for sender ✅");
                        } break;
                    }
                    goto reentry_wait_0;
                }
                else
                {
                    int retcode = rdt_send(0);
                    switch (retcode)
                    {
                        case -1:
                        {
                            LOG("receive a valid packet %d, send ACK 0 for sender dismiss!! ❌", packetbuf.seq);
                        } break;
                        case -2:
                        {
                            LOG("receive a valid packet %d, send ACK 0 for sender biterror!! ❌", packetbuf.seq);
                        } break;
                        default:
                        {
                            LOG("receive a valid packet %d, send ACK 0 for sender ✅", packetbuf.seq);
                        } break;
                    }
                    current_state = receiver_state_wait_1;
                }
                LOG("------- wait packet 0 end --------\n");

            } break;

            case receiver_state_wait_1 :
            {
                LOG("------- wait packet 1 begin --------");

            reentry_wait_1:
                rdt_recv(&packetbuf);

                int wrong = 0;
                if ((wrong = corrupt(&packetbuf)) || packetbuf.seq == 0)
                {
                    if (wrong)
                        LOG("receive a corrupt packet while waiting 1!");
                    else
                        LOG("receive packet 0 while waiting 1!");

                    int retcode = rdt_send(0);
                    switch (retcode)
                    {
                        case -1:
                        {
                            LOG("retransmit ACK 0 for sender dismiss!!  ❌");
                        } break;
                        case -2:
                        {
                            LOG("retransmit ACK 0 for sender biterror!! ❌");
                        } break;
                        default:
                        {
                            LOG("retransmit ACK 0 for sender ✅");
                        } break;
                    }
                    goto reentry_wait_1;
                }
                else
                {
                    int retcode = rdt_send(1);
                    switch (retcode)
                    {
                        case -1:
                        {
                            LOG("receive a valid packet %d, send ACK 1 for sender dismiss!! ❌", packetbuf.seq);
                        } break;
                        case -2:
                        {
                            LOG("receive a valid packet %d, send ACK 1 for sender biterror!! ❌", packetbuf.seq);
                        } break;
                        default:
                        {
                            LOG("receive a valid packet %d, send ACK 1 for sender ✅", packetbuf.seq);
                        } break;
                    }
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