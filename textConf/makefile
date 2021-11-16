CFLAGS := -g -Wall -Werror
LDFLAGS = -pthread
CC=gcc
all: server client
utils.o: utils.c utils.h
server: server.o utils.o
client: client.o utils.o
clean:
	rm -f *.o server client utils
