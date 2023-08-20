.PHONY: clean

CROSS_COMPILE := aarch64-none-linux-gnu-
CXX :=gcc
CXXFLAGS := -c -g -Wall
objs := writer.o

all: writer

writer: $(objs)
	$(CROSS_COMPILE)$(CXX) -o $@ $^

%.o: %.c
	$(CROSS_COMPILE)$(CXX) $(CXXFLAGS) $<

clean:
	rm -f $(objs)

