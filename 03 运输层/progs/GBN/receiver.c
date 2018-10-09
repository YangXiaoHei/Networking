#include "comm.h"

#define ARRSIZE(_arr_) (sizeof(_arr_) / sizeof(_arr_[0]))

int sockfd;
struct packet_t packetbuf;
struct packet_t my_packets[10];
int expected_seq = 0;
char sender_ip[128];
unsigned short sender_port;

void rdt_send(int seq);
void udt_send(struct packet_t *packet);
void rdt_recv(struct packet_t *packet, ssize_t offset);
int corrupt(struct packet_t *packet);

void rdt_send(int seq)
{
    bzero(&packetbuf, sizeof(packetbuf));
    packetbuf.seq = seq;
    packetbuf.isACK = 1;
    packetbuf.checksum = calculate_checksum((char *)&packetbuf.data, sizeof(packetbuf.data));

    /* 经由不可靠信道传输 */
    udt_send(&packetbuf);
}

void udt_send(struct packet_t *packet)
{
    /* 用不发包来当作丢包效果 */
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
    /* 如果没有收到一个完整的包, 直接退出 */
    if (TCP_recv(sockfd, (char *)packet + offset, sizeof(struct packet_t) - offset) != sizeof(struct packet_t) - offset)
    {
        LOG("incomplete packet! exit directly!");
        exit(1); 
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

    ssize_t nread;
    while (1)
    {
        setflags(sockfd, O_NONBLOCK);
        if ((nread = read(sockfd, &packetbuf, 4)) <= 0)
            continue;
        clrflags(sockfd, O_NONBLOCK);

        rdt_recv(&packetbuf, nread);

        int wrong = 0;
        if ((wrong = corrupt(&packetbuf)) || packetbuf.seq != expected_seq)
        {
            if (wrong)
                LOG("receive a corrupt packet");
            else
                LOG("ignore inorder packet %d [expected=%d]", packetbuf.seq, expected_seq);

            LOG("retransmit ACK %d", expected_seq - 1);
            rdt_send(expected_seq - 1);

            continue;
        }

        LOG("receive valid packet %d [exptected=%d]", packetbuf.seq, expected_seq);
        memcpy(&my_packets[expected_seq].data, packetbuf.data, sizeof(packetbuf.data));
        LOG("Acked packet %d", expected_seq);
        rdt_send(expected_seq);
        expected_seq++;
    }

    return 0;
}