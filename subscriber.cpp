#include "structures.h"
#include "utils.h"

using namespace std;

void usage(char *file) {
	fprintf(stderr, "Usage: %s client_id server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        usage(argv[0]);
    }

    int tcp_socket, n, ret;
    struct sockaddr_in sv_addr;
    char buffer[BUFLEN];
    
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);

    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    DIE(tcp_socket < 0, "tcp socket failed to open");

    sv_addr.sin_family = AF_INET;
    sv_addr.sin_port = htons(atoi(argv[3]));
    ret = inet_aton(argv[2], &sv_addr.sin_addr);
    DIE(ret == 0, "inet_aton failed");

    ret = connect(tcp_socket, (struct sockaddr *)&sv_addr, sizeof(sv_addr));
    DIE(ret < 0, "connect failed");

    ret = send(tcp_socket, argv[1], 256, 0);
    DIE(ret == -1, "id send failed");

    fd_set read_fd, temp_fd;
    FD_ZERO(&read_fd);
    FD_ZERO(&temp_fd);

    while (1) {
        temp_fd = read_fd;
        FD_SET(tcp_socket, &read_fd);
        FD_SET(STDIN_FILENO, &read_fd);

        FD_SET(0, &temp_fd);
        FD_SET(tcp_socket, &temp_fd);

        int rc = select(tcp_socket + 1, &temp_fd, NULL, NULL, NULL);
        DIE(rc < 0, "select failed");

        for (int i = 0; i < tcp_socket + 1; ++i) {
            if (FD_ISSET(i, &temp_fd)) {
                if (i == STDIN) {
                    memset(buffer, 0, BUFLEN);
                    ret = read(0, buffer, sizeof(buffer) - 1);
			        DIE(ret < 0, "read invalid");
                
                    char cmd[BUFLEN];
                    sscanf(buffer, "%s ", cmd);

                    if (!strcmp(cmd, "exit")) {
                        close(tcp_socket);
                        return 0;
                    }

                    if (!strcmp(cmd, "subscribe")) {
                        n = send(tcp_socket, buffer, strlen(buffer), 0);
                        DIE(n < 0, "send (subscribe) failed");
                        cout << "Subscribed to topic.\n";
                    } else if (!strcmp(cmd, "unsubscribe")) {
                        ret = send(tcp_socket, buffer, strlen(buffer), 0);
                        DIE(ret < 0, "send (unsubscribe) failed");
                        cout << "Unsubscribed from topic.\n";
                    }
                } else if (i == tcp_socket) {
                    char buffer[BUFLEN];
                    memset(buffer, 0, BUFLEN);

                    int received = recv(tcp_socket, buffer, sizeof(message), 0);
                    DIE(received < 0, "receive failed");

                    if (received > 0) {
                        for (int ct = received; ct < sizeof(message); ++ct) { // check if message is fully sent
                            received = recv(tcp_socket, buffer + ct, sizeof(message) - ct, 0);
                        }

                        message *msg = (message *)buffer;
                        msg->topic[MAX_TOPIC] = '\0';
                        
                        string aux_udp_ip = msg->IP_UDP;
                        string aux_udp_port = to_string(msg->udp_port);
                        string aux = aux_udp_ip + ":" + aux_udp_port + " - " + msg->topic + " - " + 
                                    msg->type + " - " + msg->message_value;
                        cout << aux;
                    } else if (received == 0) {
                        close(tcp_socket);
                        return 0;
                    }
                }
            }
        }
    }

    close(tcp_socket);
    return 0;
}
