#include <stdint.h>
#include <stdio.h>
#include "../include/defs.h"

// __attribute__((packed))
struct rdt_packet {
    short seq_num;
    short ack_num;
    short checksum;
    short payload_size;
    char  is_last_chunk;
    unsigned char data[BUFFER_SIZE];
};

void rdt_send(FILE* data, char* filename);
struct rdt_packet make_packet(FILE* data);
short make_checksum(unsigned char* buffer, size_t size);
void udt_send(const struct rdt_packet*);