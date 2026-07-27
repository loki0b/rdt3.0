#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include "../include/rdt_common.h"

struct rdt_packet make_packet(FILE* data) {
    struct rdt_packet pkt;
    memset(&pkt, 0, sizeof(pkt));

    if (data == NULL) {
        pkt.payload_size = 0;
    } else {
        size_t bytes_read = fread(pkt.data, sizeof(char), PKT_PAYLOAD_SIZE, data);

        if (ferror(data) != 0) {
            perror("(fread)");
            exit(EXIT_FAILURE);
        }

        pkt.payload_size = bytes_read;
        pkt.is_last_chunk = feof(data) ? 1 : 0;
    }

    return pkt;
}

int udt_rcv(struct rdt_packet* pkt) {
    int bytes_to_read;
    socklen_t remote_addr_len;

    memset(pkt, 0, sizeof(*pkt));

    if (start_timer()) {
        fprintf(stderr, "(start_timer): failed");
        exit(EXIT_FAILURE);
    }

    remote_addr_len = sizeof(remote_host_addr);
    bytes_to_read = recvfrom(socket_fd, pkt, sizeof(struct rdt_packet), 0, (struct sockaddr*)&remote_host_addr, &remote_addr_len);
    if (bytes_to_read < 0) {
        // Timeout
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINPROGRESS) return 1;
        else {
            perror("(recvfrom)");
            exit(EXIT_FAILURE);
        }
    }

    if (stop_timer()) {
        fprintf(stderr, "(start_timer): failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void udt_send(const struct rdt_packet* pkt) {
    ssize_t bytes_to_read;
    size_t bytes_to_send;

    bytes_to_send = sizeof(struct rdt_packet) - PKT_PAYLOAD_SIZE + pkt->payload_size;
    bytes_to_read = sendto(socket_fd, pkt, bytes_to_send, 0, (struct sockaddr*)&remote_host_addr, sizeof(remote_host_addr));
    if (bytes_to_read < 0) {
        perror("(sendto)");
        exit(EXIT_FAILURE);
    }
}

/*
* Internet Checksum RFC 1071
*/
unsigned short make_checksum(const struct rdt_packet* pkt, size_t size) {
    size_t bytes_left = sizeof(struct rdt_packet) - PKT_PAYLOAD_SIZE + pkt->payload_size;
    const uint16_t* ptr = (const uint16_t*)pkt;
    uint32_t sum  = 0;

    while (bytes_left > 1) {
        sum += *ptr++;

        bytes_left -= 2;
    }

    if (bytes_left == 1) {
        uint16_t last_byte = 0;
        *(uint8_t*)(&last_byte)= *(const uint8_t*)ptr;
        sum += last_byte;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (unsigned short)~sum;
}

// Validate checksum
int is_corrupt(struct rdt_packet* pkt) {
    if (pkt == NULL) return -1;
    
    size_t pkt_size = sizeof(struct rdt_packet) - PKT_PAYLOAD_SIZE + pkt->payload_size;
    int ret = (pkt->checksum != make_checksum(pkt, pkt_size)) ? 1 : 0;

    return ret;
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

// Set a timer for ACK response from receiver
// default 500ms
// TODO: dynamic interval based on RTT: TimeoutInterval = EstimatedRTT + 4 × DevRTT
int start_timer() {
    struct timeval time = {
        .tv_sec = 0,
        .tv_usec = 500000
    };

    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time)) < 0) {
        perror("(setsockopt)");
        
        return 1;
    }

    return 0;
}

// Stop the timer
int stop_timer() {
    struct timeval time = {
        .tv_sec = 0,
        .tv_usec = 0
    };

    if (setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time)) < 0) {
        perror("(setsockopt)");
        
        return 1;
    }

    return 0;
}