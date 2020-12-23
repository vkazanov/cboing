CC = gcc
CFLAGS = -g -Wall -Wextra `sdl2-config --cflags --libs`
LOADLIBES=-lSDL2
TARGET = cboing

all: $(TARGET)


.PHONY: clean
clean:
	rm -vf *.o cboing
