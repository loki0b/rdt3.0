#include "../include/rdt_receiver.h"

enum RDT_RECEIVER_STATE {
  WAIT_SEQ_0,
  WAIT_SEQ_1
};

int wip() {
  struct rdt_packet sndpkt;
  struct rdt_packet rcvpkt;
  enum RDT_RECEIVER_STATE state;
  int is_the_last_ack_received, timeout;
  size_t pkt_size;

  state = WAIT_SEQ_0;
  is_the_last_ack_received = timeout = 0;
  while (!is_the_last_ack_received) {
    switch (state) {
      case WAIT_SEQ_0: {
        timeout = rdt_rcv(&rcvpkt);
        sndpkt = make_packet(NULL);
        pkt_size = sizeof(struct rdt_packet) - PKT_PAYLOAD_SIZE + sndpkt.payload_size;
        
        // Success
        if (!timeout && !is_corrupt(&rcvpkt) && has_seq(&rcvpkt, 0)) {
          extract(&rcvpkt); //data
          deliver_data();

          sndpkt.ack_num = 0;
          sndpkt.checksum = make_checksum(&sndpkt, pkt_size);
          udt_send(&sndpkt);

          state = WAIT_SEQ_1;
        }
        // Error or receive sender's retransmission
        else if (!timeout && (is_corrupt(&rcvpkt) || has_seq(&rcvpkt, 1))) {
          sndpkt.ack_num = 1;  
          sndpkt.checksum = make_checksum(&sndpkt, pkt_size);
          udt_send(&sndpkt);
        }

        break;
      }

      case WAIT_SEQ_1: {
        timeout = rdt_rcv(&rcvpkt);
        sndpkt = make_packet(NULL);
        pkt_size = sizeof(struct rdt_packet) - PKT_PAYLOAD_SIZE + sndpkt.payload_size;
        
        // Success
        if (!timeout && !is_corrupt(&rcvpkt) && has_seq(&rcvpkt, 1)) {
          extract(&rcvpkt); //data
          deliver_data();

          sndpkt.ack_num = 1;
          sndpkt.checksum = make_checksum(&sndpkt, pkt_size);
          udt_send(&sndpkt);

          state = WAIT_SEQ_0;
        }
        // Error or receive sender's retransmission
        else if (!timeout && (is_corrupt(&rcvpkt) || has_seq(&rcvpkt, 0))) {
          sndpkt.ack_num = 0;  
          sndpkt.checksum = make_checksum(&sndpkt, pkt_size);
          udt_send(&sndpkt);
        }

        break;
      }
    }

    return 0;
  }

  return 1;
}