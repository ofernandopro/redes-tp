#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 51511
#define BUF_SIZE 500

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUF_SIZE];
    int bytes_received, bytes_sent;
    char file_name[BUF_SIZE];
    FILE *file;

    // create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // bind socket to port
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // listen for incoming connections
    if (listen(server_fd, 1) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // accept client connection
    socklen_t client_addr_len = sizeof(client_addr);
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // receive file from client
    bytes_received = recv(client_fd, buffer, BUF_SIZE, 0);
    if (bytes_received == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    if (sscanf(buffer, "select file %[^\n]", file_name) != 1) {
        fprintf(stderr, "Invalid command from client: %s\n", buffer);
        exit(EXIT_FAILURE);
    }
    file = fopen(file_name, "rb");
    if (file == NULL) {
        fprintf(stderr, "Could not open file: %s\n", file_name);
        exit(EXIT_FAILURE);
    }
    while ((bytes_received = fread(buffer, 1, BUF_SIZE, file)) > 0) {
        bytes_sent = send(client_fd, buffer, bytes_received, 0);
        if (bytes_sent == -1) {
            perror("send");
            exit(EXIT_FAILURE);
        }
    }
    fclose(file);

    // send confirmation to client
    snprintf(buffer, BUF_SIZE, "file %s received", file_name);
    bytes_sent = send(client_fd, buffer, strlen(buffer), 0);
    if (bytes_sent == -1) {
        perror("send");
        exit(EXIT_FAILURE);
    }

    // close connection
    close(client_fd);
    printf("Connection closed.\n");

    return 0;
}
