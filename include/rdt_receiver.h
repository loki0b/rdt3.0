#ifndef RDT_RECV_H
#define RDT_RECV_H

#include "../include/rdt_common.h"

void extract(struct rdt_packet* pkt);
void deliver_data();

#endif