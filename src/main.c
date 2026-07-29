#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <unistd.h>
#include "../include/rdt_sender.h"
#include "../include/rdt_receiver.h"

#define BUFFER_SIZE 1024

void setup_local_socket(char *host, int port);
void setup_remote_socket(char *host, int port);

int  socket_fd;
struct sockaddr_in local_host_addr;
struct sockaddr_in remote_host_addr;

int main(int argc, char* argv[]) {
    int localport, remoteport;
    char *localhost, remotehost[BUFFER_SIZE];

    if (argc < 2) {
        fprintf(stderr, "Argument PORT is required\n");
    } else if (argc >= 2) {
        localhost = argv[1];
        localport = atoi(argv[2]);

        if (localport < 0 || localport > UINT16_MAX) {
            fprintf(stderr, "Port number must be in [0, 2^16 - 1]\n");
            exit(EXIT_FAILURE);
        }

        setup_local_socket(localhost, localport);

        printf("Enter the remote host ip and port\n");
        scanf("%s %i", remotehost, &remoteport);
        setup_remote_socket(remotehost, remoteport);
    }

    int op;
    printf("Choose an option:\n\t1) Send messages\n\t2) Send a file\n\t3) Receive files/message\n");
    scanf("%i", &op);

    switch (op) {
        case 1: {
            char filename[] = "/tmp/output.txt";
            FILE* stream = fopen(filename, "wb+");

            int c;
            while (1) {
                while ((c = getchar()) != EOF) {
                    fputc(c, stream);

                    if (c == '\n') break;
                }

                rewind(stream);
                rdt_send(stream, filename);
                ftruncate(fileno(stream), 0);
            }
            
            break;
        }

        case 2: {
            char filename[BUFFER_SIZE];
            printf("Type the file's path:\n");
            scanf("%s", filename);

            int filefd = open(filename, O_RDONLY);
            FILE* stream = fdopen(filefd, "rb");
            rdt_send(stream, filename);

            fclose(stream);
            break;
        }

        case 3: {
            while (1) {
                rdt_rcv();
            }

            break;
        }
    }

    return 0;
}

void setup_local_socket(char *host, int port) {
    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_fd < 0) {
        perror("(socket)");
        exit(EXIT_FAILURE);
    }

    local_host_addr.sin_family = AF_INET;
    int ret = inet_pton(AF_INET, host, &local_host_addr.sin_addr);
    if (ret == 0) {
        fprintf(stderr, "Invalid network address.\n");
        exit(EXIT_FAILURE);
    } else if (ret == -1) {
        perror("(inet_pton)");
        exit(EXIT_FAILURE);
    }
    local_host_addr.sin_port = htons(port);

    if(bind(socket_fd, (struct sockaddr*)&local_host_addr, sizeof(local_host_addr)) < 0) {
        perror("(bind)");
        exit(EXIT_FAILURE);
    }
}

void setup_remote_socket(char *host, int port) {
    remote_host_addr.sin_family = AF_INET;
    
    int ret = inet_pton(AF_INET, host, &remote_host_addr.sin_addr);
    if (ret == 0) {
        fprintf(stderr, "Invalid network address.\n");
        exit(EXIT_FAILURE);
    } else if (ret == -1) {
        perror("(inet_pton)");
        exit(EXIT_FAILURE);
    }
    remote_host_addr.sin_port = htons(port);
}