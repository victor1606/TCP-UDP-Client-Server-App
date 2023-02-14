#include "structures.h"
#include "utils.h"

#include "udp_types.h"

using namespace std;

int main(int argc, char *argv[]) {
	setvbuf(stdout, NULL, _IONBF, BUFSIZ); // disable buffering

	int udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	DIE(udp_socket < 0, "socket_udp");
	int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	DIE(tcp_socket < 0, "socket_tcp");

	int port = atoi(argv[1]);

	struct sockaddr_in addr_udp;
	socklen_t socklen = sizeof(addr_udp);
	addr_udp.sin_family = AF_INET;
	addr_udp.sin_port = htons(port);
	addr_udp.sin_addr.s_addr = INADDR_ANY;

	int ret = bind(udp_socket, (struct sockaddr *)&addr_udp, sizeof(addr_udp));
	DIE(ret < 0, "bind udp");
	
	struct sockaddr_in addr_tcp;

	memset((char *)&addr_tcp, 0, sizeof(addr_tcp)); // set address
	addr_tcp.sin_family = AF_INET;
	addr_tcp.sin_port = htons(port);
	addr_tcp.sin_addr.s_addr = INADDR_ANY;

	ret = bind(tcp_socket, (struct sockaddr *)&addr_tcp, sizeof(struct sockaddr_in));
	DIE(ret < 0, "bind tcp");

	int ok = 1;
	DIE(setsockopt(tcp_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&ok, sizeof(int)) < 0, "turn off");

	ret = listen(tcp_socket, MAX_CL);
	DIE(ret == -1, "tcp listen failed");

	fd_set temp_fd, read_fd; // set file descriptors
	FD_ZERO(&read_fd);
	FD_ZERO(&temp_fd);
	
	int fdmax = tcp_socket;
	
	// map socket->id and viceversa
	unordered_map<int, string> map_socket_id;
	unordered_map<string, int> map_id_socket;

	unordered_map<string, vector<subscribers>> subscr_by_topic;

	vector<string> curr_clients;
	subscribers subscr;

	while (1) {   
		char buffer[BUFLEN];
		FD_SET(tcp_socket, &read_fd);
		FD_SET(udp_socket, &read_fd);
		FD_SET(0, &read_fd);

		temp_fd = read_fd;
		int rc = select(fdmax + 1, &temp_fd, NULL, NULL, NULL);
		DIE(rc == -1, "fd select failed");

		for (int i = 0; i < fdmax + 1; ++i) {
			message msg;
			if (FD_ISSET(i, &temp_fd)) {
				if (i == udp_socket) {
					// udp message received
					int ret = recvfrom(udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr_udp, &socklen);
					DIE(ret < 0, "udp receive failed");

					char *aux_ip = inet_ntoa(addr_udp.sin_addr);
					uint16_t aux_port = ntohs(addr_udp.sin_port);

					strcpy(msg.IP_UDP, aux_ip);	// set ip
					msg.udp_port = aux_port;	// set port

					UDP_message *udp_msg = (UDP_message *)(buffer);

					strncpy(msg.topic, udp_msg->topic, MAX_TOPIC);

					msg.topic[MAX_TOPIC] = '\0';
					if (udp_msg->type == 0) {
						udp_INT_case(udp_msg, &msg);
					} else if (udp_msg->type == 1) {
						udp_SHORT_REAL_case(udp_msg, &msg);
					} else if (udp_msg->type == 2) {
						udp_FLOAT_case(udp_msg, &msg);
					} else if (udp_msg->type == 3) {
						strcpy(msg.type, "STRING");
						strcpy(msg.message_value, udp_msg->data);
					}

					for (const auto &tp : subscr_by_topic) {
						if (!tp.first.compare(udp_msg->topic)) {
							for (int j = 0; j < subscr_by_topic[udp_msg->topic].size(); ++j) {
								string aux_str = (subscr_by_topic[udp_msg->topic])[j].id;

								vector<string>::iterator it = curr_clients.begin();

								while (it != curr_clients.end()) {
									if (*it == aux_str) {
										unordered_map<string, int>::iterator id_sock = map_id_socket.find(aux_str);
										int aux_fd = id_sock->second;
										msg.topic[MAX_TOPIC] = '\0'; // add limiter
										ret = send(aux_fd, &msg, sizeof(msg), 0);
										DIE(ret == -1, "udp send failed");
									}
								}
							}
						}
					}
				} else if (i == STDIN) {   
					fgets(buffer, sizeof(buffer) - 1, stdin);

					char cmd[BUFLEN];
					sscanf(buffer, "%s ", cmd);

					if (!strcmp(cmd, "exit")) {
						for (int ct = 0; ct < fdmax + 1; ++ct) {
							if (ct != tcp_socket && ct != udp_socket ) {
								if (FD_ISSET(ct, &read_fd)) {
									close(ct);
								}
							}
						}
						close(udp_socket);
						close(tcp_socket);
						return 0;
					}
				} else if (i == tcp_socket) {
					socklen_t clilen_tcp = sizeof(addr_tcp);
					
					int new_socket = accept(tcp_socket, (struct sockaddr *)&addr_tcp, &clilen_tcp);
					DIE(new_socket == -1, "tcp accept failed");

					char id[MAX_ID];
					ret = recv(new_socket, id, MAX_ID, 0); // receive id
					DIE(ret == -1, "id recv failed");

					vector<string>::iterator it = curr_clients.begin();
					int found = 0;
					for (auto elem : curr_clients) {
						if (!((*it).compare(id))) {
							found++;
							break;
						}
					}

					if (found == 0) {
						FD_SET(new_socket, &read_fd);
						curr_clients.push_back(id); // insert new client

						// map socket -> id and viceversa
						map_socket_id[new_socket] = id;
						map_id_socket[id] = new_socket;

						setsockopt(new_socket, IPPROTO_TCP, TCP_NODELAY,
											(char *)&ok, sizeof(int)); // Nagle
						
						fdmax = max(fdmax, new_socket);

						// new client connected
						char *new_client_sin_addr = inet_ntoa(addr_tcp.sin_addr);
						uint16_t new_client_sin_port = ntohs(addr_tcp.sin_port);
						printf("New client %s connected from %s:%hu.\n", id, new_client_sin_addr, new_client_sin_port);
					} else {
						strcpy(msg.message_value, "exit\n");
						close(new_socket);
						cout << "Client " << id << " already connected." << endl;
					}
				} else {
					memset(buffer, 0, BUFLEN);

					ret = recv(i, buffer, sizeof(buffer), 0);
					if (ret == 0) {
						vector<string>::iterator it;
						for (int k = 0; k < curr_clients.size(); ++k) {
							if (!curr_clients[k].compare(map_socket_id[i])) {
								it = curr_clients.begin() + k;
								break;
							}
						}
						curr_clients.erase(it);

						printf("Client %s disconnected.\n", const_cast<char *>(map_socket_id[i].c_str()));
						map_id_socket.erase(map_socket_id[i]);
						close(i);
						FD_CLR(i, &read_fd);
					} else {
						char cmd[BUFLEN], topic[MAX_TOPIC];
						int sf;
                    	sscanf(buffer, "%s %s %d", cmd, topic, &sf);

						if (!strcmp(cmd, "subscribe")) {
							strcpy(subscr.id, const_cast<char *>(map_socket_id[i].c_str()));
							subscr.sf = sf;
							
							int found = 0;
							for (int k = 0; k < subscr_by_topic[topic].size(); ++k) {
								string aux_id = subscr_by_topic[topic][k].id;
								
								if (aux_id == map_socket_id[i]) {
									found++;
									subscr_by_topic[topic][k].sf = subscr.sf;
									break;
								}
							}
							// add to map if a client subscribes to another topic
							if (found == 0) {
								subscr_by_topic[topic].push_back(subscr);
							}
						} else if (!strcmp(cmd, "unsubscribe")) {
							int found = 0;
							for (const auto &p : subscr_by_topic) {
								if (!p.first.compare(topic)) { // find topic that was unsubscribed from
									for (int k = 0; k < p.second.size(); ++k) {
										if (p.second.at(k).id == map_socket_id[i]) {
											found++;
											vector<subscribers> aux = p.second;
											aux.erase(aux.begin() + k);
											break;
										}
									}
								}

								if (found == 1) {
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	close(tcp_socket);
	return 0;
}