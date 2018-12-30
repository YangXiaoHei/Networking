
#ifndef _TOOL_H_
#define _TOOL_H_

void init_rand(void);
char *to_bin(unsigned long num);
int yh_random(int n, int m);
int probability(double pro);
void gen_one_bit_error(char *msg, size_t msg_len);
void clrflags(int fd, int flag);
void setflags(int fd, int flag);
unsigned long curtime_us(void);
short _calculate_checksum(short num1, short num2);
short calculate_checksum(char *data, size_t datalen);

#endif
