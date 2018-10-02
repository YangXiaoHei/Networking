
#ifndef _TOOL_H_
#define _TOOL_H_

void init_rand();
char *to_bin(unsigned long num);
int yh_random(int n, int m);
int probability(double pro);
void gen_one_bit_error(unsigned char *msg, size_t msg_len);
unsigned long curtime_us();
short _calculate_checksum(short num1, short num2);
short calculate_checksum(char *data, size_t datalen);

#endif