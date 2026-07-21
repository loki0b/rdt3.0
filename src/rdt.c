#include "../include/rdt.h"
#include "../include/defs.h"
#include <netinet/in.h>
#include <stdlib.h>

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

short make_checksum(unsigned char* buffer, size_t size) {
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