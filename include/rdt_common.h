#ifndef RDT_COMMON_H
#define RDT_COMMON_H

#include <stddef.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024
#define PKT_PAYLOAD_SIZE 1024

extern int  socket_fd;
extern struct sockaddr_in app_addr;
extern struct sockaddr_in remote_host_addr;

// __attribute__((packed))
struct rdt_packet {
    unsigned char seq_num;
    unsigned char ack_num;
    unsigned short checksum;
    unsigned short payload_size;
    unsigned char  is_last_chunk;
    unsigned char data[PKT_PAYLOAD_SIZE];
};

void udt_send(const struct rdt_packet* pkt);
unsigned short make_checksum(unsigned char* buffer, size_t size);
int is_corrupt(struct rdt_packet* pkt);
int has_seq(struct rdt_packet* pkt, unsigned char x);
int has_ack(struct rdt_packet* pkt, unsigned char x);

#endif