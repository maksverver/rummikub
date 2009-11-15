CC=gcc
CFLAGS=-g -Wall -Wextra -O2

# cross-compile for server:
CFLAGS+=-m32 -march=i686

PLAYER_OBJS=calc.o data.o main.o play.o
EXECUTABLES=player

all: $(EXECUTABLES)

player: $(PLAYER_OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(PLAYER_OBJS) $(LDLIBS)

clean:
	rm -f *.o

distclean: clean
	rm -f $(EXECUTABLES)

.PHONY: all clean distclean
