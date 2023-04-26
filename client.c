#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 51511
#define BUF_SIZE 500

int main(int argc, char **argv) {
    int client_fd;
    struct sockaddr_in server_addr;
    char buffer[BUF_SIZE];
    int bytes_received, bytes_sent;
    char file_name[BUF_SIZE];
    FILE *file;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_ip> <file_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // create socket
  client_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_fd == -1) {
      perror("socket");
      exit(EXIT_FAILURE);
  }

  // connect to server
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) != 1) {
      fprintf(stderr, "Invalid address: %s\n", argv[1]);
      exit(EXIT_FAILURE);
  }
  server_addr.sin_port = htons(PORT);
  if (connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
      perror("connect");
      exit(EXIT_FAILURE);
  }

  // select file to send
  snprintf(buffer, BUF_SIZE, "select file %s", argv[2]);
  bytes_sent = send(client_fd, buffer, strlen(buffer), 0);
  if (bytes_sent == -1) {
      perror("send");
      exit(EXIT_FAILURE);
  }

  // receive file from server
  bytes_received = 0;
  file = fopen(argv[2], "wb");
  if (file == NULL) {
      fprintf(stderr, "Could not create file: %s\n", argv[2]);
      exit(EXIT_FAILURE);
  }
  while (1) {
      bytes_received = recv(client_fd, buffer, BUF_SIZE, 0);
      if (bytes_received == -1) {
          perror("recv");
          exit(EXIT_FAILURE);
      }
      if (bytes_received == 0) {
          break;
      }
      fwrite(buffer, 1, bytes_received, file);
  }
  fclose(file);

  // receive confirmation from server
  bytes_received = recv(client_fd, buffer, BUF_SIZE, 0);
  if (bytes_received == -1) {
      perror("recv");
      exit(EXIT_FAILURE);
  }
  printf("%.*s\n", bytes_received, buffer);

  // close connection
  snprintf(buffer, BUF_SIZE, "exit");
  bytes_sent = send(client_fd, buffer, strlen(buffer), 0);
  if (bytes_sent == -1) {
      perror("send");
      exit(EXIT_FAILURE);
  }
  close(client_fd);

  return 0;

}
