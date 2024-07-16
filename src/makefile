# compilation flags
CFLAGS=-g -Wall -std=c99
CC=gcc 

# executables in this directory
EXECS=procdicN iprocdicN idespairN ipostprocN postprocN

# targets not producing a file declared phony
.PHONY: all ctph repair large_repair largeb_repair clean

all: $(EXECS) ctph repair large_repair largeb_repair

# general rule for the targets in this directory
# note: 0x78000000 = 2**31-2**27 = 2013265920
%: %.c
	gcc $(CFLAGS) -o $@ $< -D Unique=0x78000000

ctph:
	make -C ctph

repair:
	make -C repair

large_repair:
	make -C large_repair

largeb_repair:
	make -C largeb_repair

clean:
	rm -f $(EXECS)
	make -C ctph clean
	make -C repair clean
	make -C large_repair clean
	make -C largeb_repair clean


