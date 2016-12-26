CC = gcc
CFLAGS = -Wall -Ofast -lm -lpthread -std=gnu11
OGLFLAGS = -framework OpenGL -Wno-deprecated-declarations

all:
	$(CC) $(CFLAGS) -o ray-trace fuentes/main.c
#compilar Linux
ogl:
	$(CC) $(CFLAGS) $(OGLFLAGS) -lOpenGL -DOPENGL -o ray-trace fuentes/main.c
#compilar Mac
oglm:
	$(CC) $(CFLAGS) $(OGLFLAGS) -framework GLUT -DOPENGL -o ray-trace fuentes/main.c

clean:
	$(RM) ray-trace