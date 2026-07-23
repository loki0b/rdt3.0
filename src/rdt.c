#include "../include/rdt.h"
#include "../include/defs.h"
#include <asm-generic/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>

static struct sockaddr_in receiver_addr;
socklen_t receiver_addr_len;

enum RDT_STATE {
    WAIT_SEQ_0,
    WAIT_SEQ_1,
    WAIT_ACK_0,
    WAIT_ACK_1
};

void rdt_send(FILE* data, char* filename) {
    if (data) {
        struct rdt_packet pkt;
        struct rdt_packet rcv_pkt;
        enum RDT_STATE state;
        int is_the_last_ack_received;

        state = WAIT_SEQ_0;
        is_the_last_ack_received = 0;
        while (!is_the_last_ack_received) {
            switch (state) {
                case WAIT_SEQ_0: {
                    pkt = make_packet(data);
                    pkt.seq_num = 0;
                    udt_send(&pkt);

                    state = WAIT_ACK_0;
                    break;
                }

                case WAIT_ACK_0: {
                    // Timeout
                    if (rdt_rcv(&rcv_pkt)) udt_send(&pkt);
                    
                    // Check errors
                    else if (is_corrupt(&rcv_pkt) || has_ack(&rcv_pkt, 1));

                    // Success
                    else { 
                        if (pkt.is_last_chunk) is_the_last_ack_received = 1;
                        state = WAIT_SEQ_1;
                    }

                    break;
                }

                case WAIT_SEQ_1: {
                    pkt = make_packet(data);
                    pkt.seq_num = 1;
                    udt_send(&pkt);

                    state = WAIT_ACK_1;
                    break;
                }

                case WAIT_ACK_1: {
                    // Timeout
                    if (rdt_rcv(&rcv_pkt)) udt_send(&pkt);

                    // Check errors
                    else if (is_corrupt(&rcv_pkt) || has_ack(&rcv_pkt, 0));

                    // Success
                    else { 
                        if (pkt.is_last_chunk) is_the_last_ack_received = 1;
                        state = WAIT_SEQ_0;
                    }

                    break;
                }
            }
        }
    }
}

int rdt_rcv(struct rdt_packet* pkt) {
    int bytes_read;
    
    memset(pkt, 0, sizeof(*pkt));

    if (start_timer()) {
        fprintf(stderr, "(start_timer): failed");
        exit(EXIT_FAILURE);
    }

    receiver_addr_len = sizeof(receiver_addr);
    bytes_read = recvfrom(socket_fd, pkt, sizeof(struct rdt_packet), 0, (struct sockaddr*)&receiver_addr, &receiver_addr_len);
    if (bytes_read < 0) {
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

struct rdt_packet make_packet(FILE* data) {  
    struct rdt_packet pkt;
    memset(&pkt, 0, sizeof(pkt));

    size_t bytes_read = fread(pkt.data, sizeof(char), BUFFER_SIZE, data);

    if (ferror(data) != 0) {
        perror("(fread)");
        exit(EXIT_FAILURE);
    }

    pkt.payload_size = bytes_read;
    pkt.is_last_chunk = feof(data) ? 1 : 0;
    if (bytes_read > 0) {
        pkt.checksum = make_checksum(pkt.data, bytes_read);
    }

    return pkt;
}

unsigned short make_checksum(unsigned char* buffer, size_t size) {
    return 0;
}

void udt_send(const struct rdt_packet* pkt) {
    ssize_t bytes_to_read;
    size_t bytes_to_send;

    bytes_to_send = sizeof(struct rdt_packet) - BUFFER_SIZE + pkt->payload_size;
    bytes_to_read = sendto(socket_fd, pkt, bytes_to_send, 0, (struct sockaddr*)&remote_host_addr, sizeof(remote_host_addr));
    if (bytes_to_read < 0) {
        perror("(sendto)");
        exit(EXIT_FAILURE);
    }
}

// Validate checksum
int is_corrupt(struct rdt_packet* pkt) {
    if (pkt == NULL) return -1;

    return (pkt->checksum != make_checksum(pkt->data, pkt->payload_size)) ? 1 : 0;
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

// Set a timer for 500ms
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