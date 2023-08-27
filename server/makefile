.PHONY: clean

CC := gcc
# aarch64-none-linux-gnu-gcc
# CXX :=gcc
CFLAGS := -c -g -Wall
objs := aesdsocket.o

all: aesdsocket

aesdsocket: $(objs)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) -c -g -Wall $<

clean:
	rm -f $(objs)

