CFLAGS = -g -Wall

PORT_NR = 2550
IP = 127.0.0.1

all: server subscriber 

server: server.cpp udp_types.o
	g++ server.cpp udp_types.o -o server $(CFLAGS)

subscriber: subscriber.cpp
	g++ subscriber.cpp -o subscriber $(CFLAGS)
	
run_server:
	./server ${PORT}

run_client:
	./subscriber ${ID_CLIENT} ${IP} ${PORT_NR}

.PHONY: clean run_server run_client

clean:
	rm -f server subscriber udp_types.o
