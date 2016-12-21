CC = gcc
CFLAGS = -Wall -Ofast -lm -lpthread -std=c11

all:
	$(CC) $(CFLAGS) -o ray-trace fuentes/main.c

clean:
	$(RM) ray-trace