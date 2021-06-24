CC = g++
LIBCURL = -L/usr/include/curl/lib -lcurl

BROKER = src/broker.cpp src/broker_connection.cpp src/broker_clients.cpp src/broker_error_handling.cpp src/updates_manager.cpp
CLIENT = src/client.cpp

BROKER_EXEC = broker
CLIENT_EXEC = client

all: broker client

broker: $(BROKER)
	$(CC) $(BROKER)  -pthread -o $(BROKER_EXEC) $(LIBCURL)

client: $(CLIENT)
	$(CC) $(CLIENT) -o $(CLIENT_EXEC)

.PHONY: clean

clean:
	rm -f $(BROKER_EXEC) $(CLIENT_EXEC) HackMD_cache.json request.txt updates.txt source_code.txt
