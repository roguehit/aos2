CFLAGS=-Wall -g -lssl -lpthread
CC=gcc

client: src/client.c
	$(CC) $(CFLAGS) src/client.c -o bin/client

server: src/server.c
	$(CC) $(CFLAGS) src/server.c -o bin/server

phony: clean
 
clean:
	@rm bin/*
	@echo Cleaned
	
