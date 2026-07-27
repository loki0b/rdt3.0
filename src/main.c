#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <unistd.h>
#include "../include/rdt_sender.h"
#include "../include/rdt_receiver.h"

#define BUFFER_SIZE 1024

int  socket_fd;
struct sockaddr_in local_host_addr;
struct sockaddr_in remote_host_addr;

int main(int argc, char* argv[]) {
    int localport, remoteport;

    if (argc < 2) {
        fprintf(stderr, "Argument PORT is required\n");
    } else if (argc >= 2) {
        localport = atoi(argv[1]);
        remoteport = atoi(argv[2]);

        if ((localport < 0 || localport > UINT16_MAX) || (remoteport < 0 || remoteport > UINT16_MAX)) {
            fprintf(stderr, "Port number must be in [0, 2^16 - 1]\n");
            exit(EXIT_FAILURE);
        }
    }
    
    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd < 0) {
        perror("(socket)");
        exit(EXIT_FAILURE);
    }

    local_host_addr.sin_family = AF_INET;
    local_host_addr.sin_addr.s_addr = htonl(INADDR_ANY), // Set dinamically
    local_host_addr.sin_port = htons(localport);

    if(bind(socket_fd, (struct sockaddr*)&local_host_addr, sizeof(local_host_addr)) < 0) {
        perror("(bind)");
        exit(EXIT_FAILURE);
    }

    remote_host_addr.sin_family = AF_INET;
    remote_host_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    remote_host_addr.sin_port = htons(remoteport);
  
    if (localport == 8000) {
        const char filename[] = "/tmp/output.txt";
        FILE* stream = fopen(filename, "wb+");

        while (1) {
            int c;
            while ((c = getchar()) != EOF) {
                fputc(c, stream);

                if (c == '\n') break;
            }

            rewind(stream);
            rdt_send(stream, filename);
            ftruncate(fileno(stream), 0);
        }
        
    } else if (localport == 3000) {
        while (1) {
            rdt_rcv();
        }
    }
    

    return 0;
}