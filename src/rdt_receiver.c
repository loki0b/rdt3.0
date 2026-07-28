#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "../include/rdt_receiver.h"

enum RDT_RECEIVER_STATE {
  WAIT_SEQ_0,
  WAIT_SEQ_1
};

// "server_"
static const char prefix[] = {0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x5f, 0x00};

// return 1 if error;
int rdt_rcv() {
  struct rdt_packet sndpkt;
  struct rdt_packet rcvpkt;
  enum RDT_RECEIVER_STATE state;
  int is_the_last_ack_received, timeout, is_fileheader_send;
  size_t pkt_size;
  FILE* file;

  state = WAIT_SEQ_0;
  is_the_last_ack_received = timeout = is_fileheader_send = 0;
  file = NULL;
  while (!is_the_last_ack_received) {
    switch (state) {
      case WAIT_SEQ_0: {
        timeout = udt_rcv(&rcvpkt);
        sndpkt = make_packet(NULL);
        pkt_size = sizeof(struct rdt_packet) - PKT_PAYLOAD_SIZE + sndpkt.payload_size;
        
        // Success
        if (!timeout && !is_corrupt(&rcvpkt) && has_seq(&rcvpkt, 0)) {
          if (!is_fileheader_send) {
            file = extract(&rcvpkt);
            is_fileheader_send = 1;
          } else {
            deliver_data(&rcvpkt, file);
          }

          sndpkt.ack_num = 0;
          sndpkt.checksum = make_checksum(&sndpkt, pkt_size);
          udt_send(&sndpkt);

          if (rcvpkt.is_last_chunk) is_the_last_ack_received = 1;
          else state = WAIT_SEQ_1;
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
        timeout = udt_rcv(&rcvpkt);
        sndpkt = make_packet(NULL);
        pkt_size = sizeof(struct rdt_packet) - PKT_PAYLOAD_SIZE + sndpkt.payload_size;
        
        // Success
        if (!timeout && !is_corrupt(&rcvpkt) && has_seq(&rcvpkt, 1)) {
          
          // Concat data to to stream
          if (fwrite(&rcvpkt.data, sizeof(char), rcvpkt.payload_size, file) > 0) {
              ;
          }

          sndpkt.ack_num = 1;
          sndpkt.checksum = make_checksum(&sndpkt, pkt_size);
          udt_send(&sndpkt);

          if (rcvpkt.is_last_chunk) is_the_last_ack_received = 1;
          else state = WAIT_SEQ_0;
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

  }

  if (fclose(file) < 0) {
    perror("(fclose)");
    exit(EXIT_FAILURE);
  }

  return 0;
}

FILE* extract(const struct rdt_packet *pkt) {
  char *filename_separator, *output_file, *filename = NULL;
  FILE *file;

  filename = strdup((char*)&pkt->data);

  // Find filename - path
  filename_separator = strrchr(filename, '/');
  if (filename_separator != NULL) filename_separator++;
  else filename_separator = filename;

  size_t name_len = strlen(prefix) + strlen(filename_separator) + 1;
  output_file = malloc(name_len);
  if (output_file == NULL) {
    perror("(malloc)");
    exit(EXIT_FAILURE);
    
  }

  strcpy(output_file, prefix);
  strcat(output_file, filename_separator);

  printf("%s\n", output_file);

  // Create the new file in receiver
  file = fopen(output_file, "a");
  if (file == NULL) {
    perror("(fopen - deliver_data)");
    exit(EXIT_FAILURE);
  }

  free(filename);
  free(output_file);

  return file;
}

void deliver_data(const struct rdt_packet *pkt, FILE *file) {
  if (fwrite(&pkt->data, sizeof(char), pkt->payload_size, file) > 0) {
    ;
  }
}