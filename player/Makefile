CC=gcc
CFLAGS=-g -Wall -Wextra -O2

PLAYER_OBJS=player.o
EXECUTABLES=player

all: $(EXECUTABLES)

player: $(PLAYER_OBJS)
	$(CC) $(LDFLAGS) -o $@ $(PLAYER_OBJS) $(LDLIBS)

clean:
	rm -f *.o

distclean: clean
	rm -f $(EXECUTABLES)

.PHONY: all clean distclean
