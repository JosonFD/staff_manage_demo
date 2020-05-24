all:server client
OB = server client 
CFLAGS= -Wall -c -g
CC = gcc

server:server.c
	$(CC) $(CFLGS) -o $@ $^ -lsqlite3 -lpthread
client:client.c operation.c
	$(CC) $(CFLGS) -o $@ $^


.PHONY:clean
clean:
	rm  client server
