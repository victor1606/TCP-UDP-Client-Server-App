#ifndef _UDPTPS_H_
#define _UDPTPS_H_

#include "structures.h"

void udp_INT_case(UDP_message *udp_msg, message *msg);
void udp_SHORT_REAL_case(UDP_message *udp_msg, message *msg);
void udp_FLOAT_case(UDP_message *udp_msg, message *msg);

#endif