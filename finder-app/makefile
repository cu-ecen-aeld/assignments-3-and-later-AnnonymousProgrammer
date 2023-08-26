.PHONY: clean

CC := aarch64-none-linux-gnu-gcc
# CXX :=gcc
CFLAGS := -c -g -Wall
objs := writer.o

all: writer

writer: $(objs)
	$(CC) -o $@ $^

%.o: %.c
	$(CC) -c -g -Wall $<

clean:
	rm -f $(objs)

