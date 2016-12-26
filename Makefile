CC = gcc
CFLAGS = -Wall -Ofast -lm -lpthread -std=gnu11
OGLFLAGS = -framework OpenGL -framework GLUT -Wno-deprecated-declarations

all:
	$(CC) $(CFLAGS) -o ray-trace fuentes/main.c

ogl:
	$(CC) $(CFLAGS) $(OGLFLAGS) -DOPENGL -o ray-trace fuentes/main.c

clean:
	$(RM) ray-trace