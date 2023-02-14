#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include <bits/stdc++.h>

#define BUFLEN 1551
#define MAX_CL 3000
#define MAX_ID 256
#define MAX_TOPIC 50
#define MAX_LEN 1600
#define STDIN 0
#define IP_LENGTH 16
#define ID_TYPE_LENGTH 11
#define MSG_LEN 2000

struct message {
    uint16_t udp_port;
    char topic[MAX_TOPIC];
    char type[ID_TYPE_LENGTH];
    char message_value[MSG_LEN];
    char IP_UDP[IP_LENGTH];
    
};

struct UDP_message {
    uint8_t type;
    char topic[MAX_TOPIC];
    char data[BUFLEN];
};

struct subscribers {
    int sf;
    char id[ID_TYPE_LENGTH];
};

#endif