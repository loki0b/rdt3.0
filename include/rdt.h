#include <stdint.h>
#include <stdio.h>
#include "../include/defs.h"

// __attribute__((packed))
struct rdt_packet {
    unsigned char seq_num;
    unsigned char ack_num;
    unsigned short checksum;
    unsigned short payload_size;
    unsigned char  is_last_chunk;
    unsigned char data[BUFFER_SIZE];
};

void rdt_send(FILE* data, char* filename);
void udt_send(const struct rdt_packet*);
void rdt_rcv(struct rdt_packet*);

struct rdt_packet make_packet(FILE* data);
unsigned short make_checksum(unsigned char* buffer, size_t size);

int is_corrupt(struct rdt_packet* pkt);
int has_ack(struct rdt_packet* pkt, unsigned char x);
int has_seq(struct rdt_packet* pkt, unsigned char x);

void extract(struct rdt_packet* pkt);
void deliver_data();