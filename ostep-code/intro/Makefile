
.PHONY: all clean

all: cpu.out mem.out threads.out io.out

clean:
	rm -f cpu.out mem.out threads.out io.out

cpu.out: cpu.c common.h
	gcc -o cpu.out cpu.c -Wall

mem.out: mem.c common.h
	gcc -o mem.out mem.c -Wall

threads.out: threads.c common.h common_threads.h
	gcc -o threads.out threads.c -Wall -pthread

io.out: io.c common.h
	gcc -o io.out io.c -Wall

