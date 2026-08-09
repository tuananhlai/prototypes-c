
FLAGS = -Wall -pthread
INCLUDES = ../include

.PHONY: all clean

all: t0.out t1.out

clean:
	rm -f t0.out t1.out

t0.out: t0.c
	gcc -I $(INCLUDES) -o t0.out t0.c $(FLAGS)

t1.out: t1.c
	gcc -I $(INCLUDES) -o t1.out t1.c $(FLAGS)
