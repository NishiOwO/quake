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
	@echo CC $@
ifdef DEV
	@rm -f $@.log
	@$(CC) $(CFLAGS) -c -o $@ $< >$@.log 2>&1
	@./proc.sh $@ $<
	@rm -f $@.log
else
	@$(CC) $(CFLAGS) -c -o $@ $<
endif

clean:
	rm -f quake *.o
