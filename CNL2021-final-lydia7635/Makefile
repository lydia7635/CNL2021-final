CC = g++
LIBCURL = -L/usr/include/curl/lib -lcurl

BROKER = src/broker.cpp src/broker_connection.cpp src/broker_clients.cpp src/broker_error_handling.cpp

BROKER_EXEC = broker

all: broker

broker: $(BROKER)
	$(CC) $(BROKER) -o $(BROKER_EXEC) $(LIBCURL)

.PHONY: clean

clean:
	rm $(BROKER_EXEC)