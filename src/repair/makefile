
all: irepair despair idespair

CFLAGS = -Wall -g -O9


# targets not producing a file declared phony
.PHONY: all clean

irepair: irepair.o array.o hash.o heap.o records.o basics.o makefile
	gcc $(CFLAGS) -o irepair irepair.o array.o hash.o heap.o records.o basics.o

irepair.o: irepair.c array.h hash.h heap.h records.h basics.h makefile
	gcc $(CFLAGS) -c irepair.c

array.o: array.c array.h hash.h heap.h records.h basics.h makefile
	gcc $(CFLAGS) -c array.c

hash.o: hash.c array.h hash.h heap.h records.h basics.h makefile
	gcc $(CFLAGS) -c hash.c

heap.o: heap.c array.h hash.h heap.h records.h basics.h makefile
	gcc $(CFLAGS) -c heap.c

records.o: records.c array.h hash.h heap.h records.h basics.h makefile
	gcc $(CFLAGS) -c records.c

basics.o: basics.c basics.h makefile
	gcc $(CFLAGS) -c basics.c

clean:
	rm -f irepair despair idespair *.o
