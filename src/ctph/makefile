# compilation flags
CXX_FLAGS=-std=c++11 -O3 -Wall -Wextra -g
CFLAGS=-O3 -Wall -std=c99 -g
CC=gcc

# main executables 
EXECS=newscan.x pscan.x
# executables not using threads (and therefore not needing the thread library)
EXECS_NT=newscanNT.x unparsz

# targets not producing a file declared phony
.PHONY: all clean

all: $(EXECS) $(EXECS_NT)

newscanNT.x: newscan.cpp malloc_count.o utils.o
	$(CXX) $(CXX_FLAGS) -o $@ $^ -ldl -DNOTHREADS

newscan.x: newscan.cpp newscan.hpp malloc_count.o utils.o xerrors.o 
	$(CXX) $(CXX_FLAGS) -o $@ newscan.cpp malloc_count.o utils.o xerrors.o -ldl -pthread

pscan.x: pscan.cpp pscan.hpp malloc_count.o utils.o xerrors.o 
	$(CXX) $(CXX_FLAGS) -o $@ pscan.cpp malloc_count.o utils.o xerrors.o -ldl -pthread

unparsz: unparsz.c utils.o malloc_count.o
	$(CC) $(CFLAGS) -o $@ $^ -ldl

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(EXECS) $(EXECS_NT) *.o
