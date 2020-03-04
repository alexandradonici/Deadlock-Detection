CC=gcc
CFLAGS= -c -Wall -Wextra -pedantic -pthread
OBJECTS= main1_mutex.o lockcheck.o
LIBS = -lpthread


# --- targets
all:    hello

hello:   $(OBJECTS)
		$(CC)  -o hello  $(OBJECTS) $(LIBS)

main1_mutex.o: main1_mutex.c
		$(CC) $(CFLAGS)  main1_mutex.c $(LIBS)

lockdep.o: lockcheck.c
		$(CC) $(CFLAGS)  lockcheck.c $(LIBS)


# --- remove binary and executable files
clean:
	rm -rf main1_mutex $(OBJECTS)
