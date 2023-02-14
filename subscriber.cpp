#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"

void usage(char *file)
{
	fprintf(stderr, "Usage: %s id_client server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];

	if (argc < 4) {
		usage(argv[0]);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "invalid_socket");

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));

	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton failed");

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect failed");

	int fdmax = sockfd;

	while (1) {
		fd_set read_set;
		FD_SET(STDIN_FILENO, &read_set);
		FD_SET(sockfd, &read_set);

		int rc = select(fdmax + 1, &read_set, NULL, NULL, NULL);
		DIE(rc < 0, "select");

		if(FD_ISSET(STDIN_FILENO, &read_set)) {
			// se citeste de la stdin
			memset(buffer, 0, sizeof(buffer));

			n = read(0, buffer, sizeof(buffer) - 1);
			DIE(n < 0, "read invalid");

			if (strncmp(buffer, "subscribe", 9) == 0) {
				ret = send(sockfd, buffer, strlen(buffer), 0);
				DIE(ret < 0, "subscribe command failed");
				printf("Subscribed to topic.\n");
			}

			if (strncmp(buffer, "unsubscribe", 11) == 0) {
				ret = send(sockfd, buffer, strlen(buffer), 0);
                DIE(ret < 0, "unsubscribe command failed");
                printf("Unsubscribed from topic.\n");
			}

			if (strncmp(buffer, "exit", 4) == 0) {
				break;
			}

			// se trimite mesaj la server
			n = send(sockfd, buffer, strlen(buffer), 0);
			DIE(n < 0, "send");
		} else {
			// Socket
			int rc = recv(sockfd, buffer, sizeof(buffer), 0);
			DIE(rc < 0, "recv");

			if(rc == 0) {
				printf("Connection closed\n");
				break;
			}

			int wc = write(STDOUT_FILENO, buffer, rc);
			DIE(wc < 0, "write failed");
		}
  		
	}

	close(sockfd);

	return 0;
}
