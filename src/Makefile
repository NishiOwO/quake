CC = cc
CFLAGS = -g -fomit-frame-pointer -funroll-loops -fcommon -DGL_EXT_SHARED -DGLQUAKE -Iinclude `pkg-config --cflags x11` `pkg-config --cflags gl`
LDFLAGS =
LIBS = `pkg-config --libs x11` `pkg-config --libs gl` -lm

.PHONY: all clean
.SUFFIXES: .c .o

all: quake

OBJS = $(shell cat list)

quake: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f quake *.o
