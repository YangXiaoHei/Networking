unsigned short checksum(unsigned short *addr, int len)
{
    unsigned int sum = 0;
    unsigned short anwser = 0;
    unsigned short *w = addr;

    /* 16 位累加，每次用掉两个字节 */
    while(len > 1) {
        sum += *w++;
        len -= 2;
    }

    /* 如果 len 是奇数，那么把最后一个字节加上去 */
    if (len == 1) {
        *(unsigned char *)&anwser = *(unsigned char *)w;
        sum += anwser;
    }

    /* 把高 16 位和低 16 位相加 */
    sum = (sum >> 16) + (sum & 0xFFFF);

    /* 把进位加上来 */
    sum += (sum >> 16);

    /* 取反码 */
    anwser = ~sum;
    return anwser;
}