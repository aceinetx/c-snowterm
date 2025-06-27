CC = cc
CFLAGS = -std=c90 -Wall -Wextra -Wpedantic -ggdb
LDFLAGS = -lncurses

all: c-snowterm
clean:
	rm -rf c-snowterm *.o

c-snowterm: c-snowterm.o
	$(CC) $(LDFLAGS) -o $@ $< 

c-snowterm.o: c-snowterm.c
	$(CC) $(CFLAGS) -c -o $@ $< 
