CFLAGS := -g -Wall -Werror
CC=gcc
all: server deliver
utils.o: utils.c utils.h
packet.o: packet.c packet.h
server: server.o packet.o utils.o
deliver: deliver.o packet.o utils.o
clean:
	rm -f *.o server deliver packet utils
