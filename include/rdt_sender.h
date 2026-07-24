#ifndef RDT_SND_H
#define RDT_SND_H

#include <stdio.h>
#include "../include/rdt_common.h"

int rdt_send(FILE* data, char* filename);
int rdt_rcv(struct rdt_packet* pkt);
struct rdt_packet make_packet(FILE* data);



int start_timer();
int stop_timer();

#endif