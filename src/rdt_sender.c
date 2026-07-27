#include <string.h>
#include "../include/rdt_sender.h"

enum RDT_SENDER_STATE {
    WAIT_SEQ_0,
    WAIT_SEQ_1,
    WAIT_ACK_0,
    WAIT_ACK_1
};

int rdt_send(FILE* data, const char* filename) {
    if (data) {
        struct rdt_packet sndpkt;
        struct rdt_packet rcvpkt;
        enum RDT_SENDER_STATE state;
        int is_the_last_ack_received, timeout, is_fileheader_send;
        size_t pkt_size;

        state = WAIT_SEQ_0;
        is_the_last_ack_received = timeout = is_fileheader_send = 0;
        while (!is_the_last_ack_received) {
            switch (state) {
                case WAIT_SEQ_0: {
                    if (!is_fileheader_send && filename != NULL) {
                        sndpkt = make_packet(NULL);
                        sndpkt.payload_size = strlen(filename);
                        memcpy(sndpkt.data, filename, strlen(filename));
                        sndpkt.data[strlen(filename)] = '\0';
                        is_fileheader_send = 1;
                    } else {
                        sndpkt = make_packet(data);
                    }
                    
                    sndpkt.seq_num = 0;
                    pkt_size = sizeof(struct rdt_packet) - PKT_PAYLOAD_SIZE + sndpkt.payload_size;
                    sndpkt.checksum = make_checksum(&sndpkt, pkt_size);
                    udt_send(&sndpkt);

                    state = WAIT_ACK_0;
                    break;
                }

                case WAIT_ACK_0: {
                    timeout = udt_rcv(&rcvpkt);
                    
                    if (timeout) udt_send(&sndpkt);
                    // Check errors or sender's segment restransmission
                    else if (is_corrupt(&rcvpkt) || has_ack(&rcvpkt, 1));
                    // Success
                    else { 
                        if (sndpkt.is_last_chunk) is_the_last_ack_received = 1;
                        state = WAIT_SEQ_1;
                    }

                    break;
                }

                case WAIT_SEQ_1: {
                    sndpkt = make_packet(data);
                    sndpkt.seq_num = 1;
                    pkt_size = sizeof(struct rdt_packet) - PKT_PAYLOAD_SIZE + sndpkt.payload_size;
                    sndpkt.checksum = make_checksum(&sndpkt, pkt_size);
                    udt_send(&sndpkt);

                    state = WAIT_ACK_1;
                    break;
                }

                case WAIT_ACK_1: {
                    timeout = udt_rcv(&rcvpkt);
                    
                    if (timeout) udt_send(&sndpkt);
                    // Check errors or sender's segment retransmission
                    else if (is_corrupt(&rcvpkt) || has_ack(&rcvpkt, 0));
                    // Success
                    else { 
                        if (sndpkt.is_last_chunk) is_the_last_ack_received = 1;
                        state = WAIT_SEQ_0;
                    }

                    break;
                }
            }
        }

        return 0;
  }

  return 1;
}