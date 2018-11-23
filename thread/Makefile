#
# Makefile for File Transfer Socket
# Heoju
#

CC = gcc
CFLAGS = 

server : server.c
	$(CC) $(CFLAGS) -o server server.c

client : client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	-rm server
	-rm client

all : server client
