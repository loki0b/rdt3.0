#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include "../include/rdt.h"

int  socket_fd;
struct sockaddr_in app_addr;
struct sockaddr_in remote_host_addr;

int main(int argc, char* argv[]) {
    int port;

    if (argc < 2) {
        fprintf(stderr, "Argument PORT is required\n");
    } else if (argc == 2) {
        port = atoi(argv[1]);

        if (port > UINT16_MAX) {
            fprintf(stderr, "PORT needs to be a 16 bit number\n");
            exit(EXIT_FAILURE);
        }
    }
    
    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd < 0) {
        perror("(socket)");
        exit(EXIT_FAILURE);
    }

    app_addr.sin_family = AF_INET,
    app_addr.sin_addr.s_addr = htonl(INADDR_ANY), // Set dinamically
    app_addr.sin_port = htons(port);

    if(bind(socket_fd, (struct sockaddr*)&app_addr, sizeof(app_addr)) < 0) {
        perror("(bind)");
        exit(EXIT_FAILURE);
    }

    return 0;
}