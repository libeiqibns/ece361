#ifndef CLIENT_H
#define CLIENT_H

#define DEFAULT_HOST "0.0.0.0"
#define DEFAULT_SERVER_PORT "5000"    // the port users will be connecting to
#define DEFAULT_PACKET_COUNT 5
#define DEFAULT_MESSAGE_LEN 5
#define MAX_BUF_LEN 2048
#define MAX_STR_LEN 100
#define MAX_PACKET_COUNT 25
#define INT_INF 99999999
#define BITS_PER_BYTE 8
#define MICROSECS_PER_SEC 1000000
#define MAX(x,y) (x>y) ? x:y
#define MIN(x,y) (x<y) ? x:y

void rand_str_generator(int len, char *rand_str);

#endif //client.h