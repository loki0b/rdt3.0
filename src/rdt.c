#include "../include/rdt.h"
#include <netinet/in.h>
#include <stdlib.h>

void udt_send(const struct rdt_packet* pkt) {
    int bytes_read;

    bytes_read = sendto(socket_fd, pkt->data, BUFFER_SIZE, 0, (struct sockaddr*)&app_addr, sizeof(app_addr));
    if (bytes_read < 0) {
        perror("(sendto)");
        exit(EXIT_FAILURE);
    }
}