OPENGL_CFLAGS = `pkg-config --cflags gl`
OPENGL_LIBS = `pkg-config --libs gl`

CC = cc
CFLAGS = -g -fomit-frame-pointer -funroll-loops -fcommon -DGL_EXT_SHARED -DGLQUAKE -Iinclude `pkg-config --cflags x11` $(OPENGL_CFLAGS) -Wno-error=implicit-function-declaration
LDFLAGS =
LIBS = -lm -lpthread $(OPENGL_LIBS)

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
