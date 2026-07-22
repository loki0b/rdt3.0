#include "../include/rdt.h"
#include "../include/defs.h"
#include <netinet/in.h>
#include <stdlib.h>

void rdt_send(FILE* data, char* filename) {
    if (data) {
        struct rdt_packet pkt;

        while (!feof(data)) {
            pkt = make_packet(data);
            pkt.seq_num = 0;
            pkt.ack_num = 0;

            udt_send(&pkt);
        }
    }
}

struct rdt_packet make_packet(FILE* data) {  
    struct rdt_packet pkt;

    size_t bytes_read = fread(pkt.data, sizeof(char), BUFFER_SIZE, data);
    if (bytes_read > 0) {
        pkt.payload_size = bytes_read;
        pkt.is_last_chunk = feof(data) ? 1 : 0;
        pkt.checksum = make_checksum(pkt.data, bytes_read);
    }

    return pkt;
}

unsigned short make_checksum(unsigned char* buffer, size_t size) {
    return 0;
}

void udt_send(const struct rdt_packet* pkt) {
    int bytes_read;

    bytes_read = sendto(socket_fd, pkt->data, BUFFER_SIZE, 0, (struct sockaddr*)&app_addr, sizeof(app_addr));
    if (bytes_read < 0) {
        perror("(sendto)");
        exit(EXIT_FAILURE);
    }
}

// Validate checksum
int is_corrupt(struct rdt_packet* pkt) {
    if (pkt == NULL) return -1;

    return (pkt->checksum == make_checksum(pkt->data, pkt->payload_size)) ? 1 : 0;
}

// Check if ack_num is x
int has_ack(struct rdt_packet* pkt, unsigned char x) {
    if (pkt == NULL) return -1;

    return (pkt->ack_num == x) ? 1 : 0;
}

// Check if seq_num is x
int has_seq(struct rdt_packet* pkt, unsigned char x) {
    if (pkt == NULL) return -1;

    return (pkt->seq_num == x) ? 1 : 0;
}