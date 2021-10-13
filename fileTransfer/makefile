CFLAGS := -g -Wall -Werror
CC=gcc
all: server deliver
packet.o: packet.c packet.h
server: server.o packet.o
deliver: deliver.o packet.o
clean:
	rm -f *.o server deliver packet
