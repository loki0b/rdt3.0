#include "../include/rdt_sender.h"

enum RDT_SENDER_STATE {
    WAIT_SEQ_0,
    WAIT_SEQ_1,
    WAIT_ACK_0,
    WAIT_ACK_1
};

int rdt_send(FILE* data) {
    if (data) {
        struct rdt_packet sndpkt;
        struct rdt_packet rcvpkt;
        enum RDT_SENDER_STATE state;
        int is_the_last_ack_received, timeout;
        size_t pkt_size;

        state = WAIT_SEQ_0;
        is_the_last_ack_received = timeout = 0;
        while (!is_the_last_ack_received) {
            switch (state) {
                case WAIT_SEQ_0: {
                    sndpkt = make_packet(data);
                    sndpkt.seq_num = 0;
                    pkt_size = sizeof(struct rdt_packet) - PKT_PAYLOAD_SIZE + sndpkt.payload_size;
                    sndpkt.checksum = make_checksum(&sndpkt, pkt_size);
                    udt_send(&sndpkt);

                    state = WAIT_ACK_0;
                    break;
                }

                case WAIT_ACK_0: {
                    timeout = rdt_rcv(&rcvpkt);
                    
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
                    timeout = rdt_rcv(&rcvpkt);
                    
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