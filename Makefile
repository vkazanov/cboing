CC = gcc
CFLAGS = -g -Wall -Wextra `sdl2-config --cflags`
LOADLIBES=`sdl2-config --libs` -lSDL2_image
TARGET = cboing

all: $(TARGET)


.PHONY: clean
clean:
	rm -vf *.o cboing
