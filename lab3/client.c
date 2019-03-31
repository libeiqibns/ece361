/*
** client.c -- a datagram "client" demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "client.h"

void rand_str_generator(int len, char *rand_str){
    memset(rand_str, 0, MAX_STR_LEN);
    int r;
    for (int i = 0; i < len; i++){
        r = rand() % 95 + 32; //generate random char
        rand_str[i] = r;
    }
    rand_str[len] = '\0';
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes, str_len, packet_count;
    int host_arg_idx = -1, message_len_arg_idx = -1, count_arg_idx = -1, port_arg_idx = -1, log_arg_idx = -1;
    char buf[MAX_BUF_LEN], rand_str[MAX_STR_LEN];
    char *host_name, *port;
    struct timeval start_time[MAX_PACKET_COUNT], end_time[MAX_PACKET_COUNT];
    int max_time = 0, min_time = INT_INF, avg_time_1 = 0, avg_time_2 = 0, time_interval[MAX_PACKET_COUNT];
    double time_dev = 0.0;
    bool silent = 0;
    FILE *fd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    srand(time(NULL));

    for (int i = 1; i < argc; ++i){
        if (strcmp("-h", argv[i]) == 0){
            host_arg_idx = ++i;
        }
        else if (strcmp("-l", argv[i]) == 0){
            message_len_arg_idx = ++i;
        }
        else if (strcmp("-c", argv[i]) == 0){
            count_arg_idx = ++i;
        }
        else if (strcmp("-p", argv[i]) == 0){
            port_arg_idx = ++i;  
        }
        else if (strcmp("-log", argv[i]) == 0){
            log_arg_idx = ++i;
        }
        else if (strcmp("-silent", argv[i]) == 0){
            silent = 1;
        }
        else {
            fprintf(stderr,"usage: client [-h hostname] [-l message_length] [-c packet_count] [-p port] [-log file_name] [-silent]\n");
            exit(1);
        }
    }

    if (host_arg_idx == -1) host_name = DEFAULT_HOST;
    else host_name = argv[host_arg_idx];

    if (message_len_arg_idx == -1) str_len = DEFAULT_MESSAGE_LEN;
    else str_len = atoi(argv[message_len_arg_idx]);
    assert(str_len > 0 || strcmp(argv[message_len_arg_idx], "0") == 0);
    if (str_len > MAX_STR_LEN){
        fprintf(stderr, "error: message length should not be greater than %d bytes\n", MAX_STR_LEN);
        exit(2);
    }

    if (count_arg_idx == -1) packet_count = DEFAULT_PACKET_COUNT;
    else packet_count = atoi(argv[count_arg_idx]);
    assert(packet_count > 0);
    if (packet_count > MAX_PACKET_COUNT){
        fprintf(stderr, "error: cannot send more than %d packets\n", MAX_PACKET_COUNT);
        exit(2);
    }

    if (log_arg_idx != -1){
        fd = fopen(argv[log_arg_idx], "w");
        if (fd == NULL){
            fprintf(stderr, "error: couldn't open file %s\n", argv[log_arg_idx]);
            exit(3);
        }
    }

    if (port_arg_idx == -1) port = DEFAULT_SERVER_PORT;
    else port = argv[port_arg_idx];




    if ((rv = getaddrinfo(host_name, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
    }

    //first trial: generate 5 random character strings of length (str_len)
    //send the strings and wait for response
    //find average delay
    if (!silent) printf("\nStart first trial\n");

    for (int i = 0; i < packet_count; i++){
        assert(!gettimeofday(&start_time[i], NULL));

        rand_str_generator(str_len, rand_str);
        if ((numbytes = sendto(sockfd, rand_str, str_len, 0,
                p->ai_addr, p->ai_addrlen)) == -1) {
            perror("talker: sendto");
            exit(1);
        }

        if ((numbytes = recvfrom(sockfd, buf, MAX_BUF_LEN-1 , 0,
            p->ai_addr, &p->ai_addrlen)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        assert(!gettimeofday(&end_time[i], NULL));
        time_interval[i] = (end_time[i].tv_sec * 1000000 + end_time[i].tv_usec) 
                        - (start_time[i].tv_sec * 1000000 + start_time[i].tv_usec);

        max_time = MAX(max_time, time_interval[i]);
        min_time = MIN(min_time, time_interval[i]);
        avg_time_1 += time_interval[i];

        if (!silent) printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
        if (!silent) printf("listener: packet is %d bytes long\n", numbytes);
        buf[numbytes] = '\0';
        if (!silent) printf("listener: packet contains \"%s\"\n", buf);
        
    }

    avg_time_1 /= packet_count;
    
    for (int i = 0; i < packet_count; i++){
        time_dev += (time_interval[i] - avg_time_1) * (time_interval[i] - avg_time_1);
    }
    
    time_dev = sqrt(time_dev/(double)packet_count);

    if (!silent) printf("\n"
        "num of bytes       = %10d bytes\n"
        "max delay          = %10d us\n"
        "min delay          = %10d us\n"
        "average delay      = %10d us\n"
        "standard deviation = %10d us\n",
        str_len, max_time, min_time, avg_time_1, (int)time_dev);


    //second trial: generate 5 random character strings of length (str_len+1)
    //send the strings and wait for response
    //find average delay

    ++str_len;
    max_time = 0;
    min_time = INT_INF;

    if (!silent) printf("\nStart second trial\n");
    
    for (int i = 0; i < packet_count; i++){
        assert(!gettimeofday(&start_time[i], NULL));

        rand_str_generator(str_len, rand_str);
        if ((numbytes = sendto(sockfd, rand_str, str_len, 0,
                p->ai_addr, p->ai_addrlen)) == -1) {
            perror("talker: sendto");
            exit(1);
        }

        if ((numbytes = recvfrom(sockfd, buf, MAX_BUF_LEN-1 , 0,
            p->ai_addr, &p->ai_addrlen)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        assert(!gettimeofday(&end_time[i], NULL));
        time_interval[i] = (end_time[i].tv_sec * 1000000 + end_time[i].tv_usec) 
                        - (start_time[i].tv_sec * 1000000 + start_time[i].tv_usec);

        max_time = MAX(max_time, time_interval[i]);
        min_time = MIN(min_time, time_interval[i]);
        avg_time_2 += time_interval[i];

        if (!silent) printf("talker: sent %d bytes to %s\n", numbytes, argv[1]);
        if (!silent) printf("listener: packet is %d bytes long\n", numbytes);
        buf[numbytes] = '\0';
        if (!silent) printf("listener: packet contains \"%s\"\n", buf);
        
    }

    avg_time_2 /= packet_count;
    
    for (int i = 0; i < packet_count; i++){
        time_dev += (time_interval[i] - avg_time_2) * (time_interval[i] - avg_time_2);
    }
    
    time_dev = sqrt(time_dev/(double)packet_count);

    if (!silent) printf("\n"
        "num of bytes       = %10d bytes\n"
        "max delay          = %10d us\n"
        "min delay          = %10d us\n"
        "average delay      = %10d us\n"
        "standard deviation = %10d us\n",
        str_len, max_time, min_time, avg_time_2, (int)time_dev);

    //print final result of transmission rate and t_prop
    int trans_time, t_prop;
    double trans_rate;

    trans_time = avg_time_2 - avg_time_1;
    trans_rate = 2 * BITS_PER_BYTE * MICROSECS_PER_SEC / (double) trans_time;
    t_prop = (avg_time_2 - str_len * trans_time) / 2;

    printf("\n"
        "transmission rate  = %10d bits/s\n"
        "t_prop             = %10d us\n\n",
        (int)trans_rate, t_prop);

    if (log_arg_idx != -1){
        fprintf(fd,
        "transmission rate  = %10d bits/s\n"
        "t_prop             = %10d us\n",
        (int)trans_rate, t_prop);

        fclose(fd);
    }

    freeaddrinfo(servinfo);
    close(sockfd);

    return 0;
}