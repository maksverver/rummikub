GXX=g++
CXXFLAGS=-g -Wall -Wextra -O2
LDFLAGS=-Wl,--as-needed
LDLIBS=-lcurl

COMMON_OBJS=rummikub.o rummikub-io.o rummikub-rpc.o
ARBITER_OBJS=$(COMMON_OBJS) arbiter.o
EXECUTABLES=arbiter

all: $(EXECUTABLES)

arbiter: $(ARBITER_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(ARBITER_OBJS) $(LDLIBS)

clean:
	rm -f *.o

distclean: clean
	rm -f $(EXECUTABLES)

.PHONY: all clean distclean
