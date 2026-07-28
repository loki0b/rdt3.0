#ifndef RDT_RECV_H
#define RDT_RECV_H

#include "../include/rdt_common.h"

void deliver_data(const struct rdt_packet *pkt, FILE *file);
FILE* extract(const struct rdt_packet *pkt);
int rdt_rcv();

#endif