OPENGL_CFLAGS = `pkg-config --cflags gl`
OPENGL_LIBS = `pkg-config --libs gl`

EXEC =

CC = cc
CFLAGS = -g -fomit-frame-pointer -funroll-loops -fcommon -DGL_EXT_SHARED -DGLQUAKE -Iinclude `pkg-config --cflags x11` $(OPENGL_CFLAGS) -Wno-error=implicit-function-declaration
LDFLAGS =
LIBS = -lm -lpthread $(OPENGL_LIBS)

.PHONY: all clean
.SUFFIXES: .c .o

OBJS = $(shell cat list)

TARGET = unix
ifeq ($(TARGET),unix)
OBJS += net_udp.o net_bsd.o sys.o
else ifeq ($(TARGET),windows)
OBJS += net_udp.o net_bsd.o sys_win.o conproc.o
CFLAGS = -mwindows -fcommon -DGL_EXT_SHARED -DGLQUAKE -Iinclude
LIBS = -lopengl32 -lglu32 -lgdi32 -lwsock32
EXEC = .exe
endif

all: quake$(EXEC)

quake$(EXEC): $(OBJS)
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
	rm -f quake quake.exe *.o
