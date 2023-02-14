#include <arpa/inet.h>
#include "stdio.h"

#include "udp_types.h"
#include "structures.h"

void udp_INT_case(UDP_message *udp_msg, message *msg) {
    int num = ntohl(*(uint32_t *)(udp_msg->data + 1));
    strcpy(msg->type, "INT");

    if (udp_msg->data[0] != 0) {
        num = num * (-1);
    }

    sprintf(msg->message_value, "%d", num);
}

void udp_SHORT_REAL_case(UDP_message *udp_msg, message *msg) {
    double num = ntohs(*(uint16_t *)(udp_msg->data));
    strcpy(msg->type, "SHORT_REAL");

    num = num / 100;

    sprintf(msg->message_value, "%.2f", num);
}

void udp_FLOAT_case(UDP_message *udp_msg, message *msg) {
    double num = ntohl(*(uint32_t *)(udp_msg->data + 1));
    strcpy(msg->type, "FLOAT");

    num = num / pow(10, udp_msg->data[5]);

    if (udp_msg->data[0] == 0) {
        num = num * (-1);
    }

    sprintf(msg->message_value, "%lf", num);
}
