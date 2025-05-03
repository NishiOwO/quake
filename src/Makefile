TARGET = unix
DEBUG = -O2

ifeq ($(TARGET),unix)
OPENGL_CFLAGS = `pkg-config --cflags gl`
OPENGL_LIBS = `pkg-config --libs gl` `pkg-config --libs glu`
else ifeq ($(TARGET),windows)
OPENGL_CFLAGS =
OPENGL_LIBS = -lopengl32 -lglu32
endif

EXEC =

CC = $(GCC_PREFIX)gcc
WINDRES = $(GCC_PREFIX)windres
CFLAGS = $(DEBUG) -fomit-frame-pointer -funroll-loops -fcommon -DGL_EXT_SHARED -DGLQUAKE -Iinclude -Wno-error=implicit-function-declaration $(OPENGL_CFLAGS)
LDFLAGS =
LIBS = $(OPENGL_LIBS)

.PHONY: all clean
.SUFFIXES: .c .o .rc .res

OBJS = $(shell cat list)

ifdef QUAKE2
CFLAGS += -DQUAKE2
endif

ifeq ($(TARGET),unix)
OBJS += net_udp.o net_bsd.o sys.o
CFLAGS += `pkg-config --cflags x11`
LIBS += -lm -lpthread
else ifeq ($(TARGET),windows)
OBJS += net_udp.o net_bsd.o sys_win.o conproc.o quake.res
CFLAGS += -mwindows
LIBS += -lgdi32 -lwsock32
EXEC = .exe
endif

all:
ifdef QUAKE2
	@echo "Quake2: Enabled"
else
	@echo "Quake2: Disabled"
endif
	@echo " Target: $(TARGET)"
	@echo "     CC: $(CC)"
	@echo " CFLAGS: $(CFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "   LIBS: $(LIBS)"
	$(MAKE) quake$(EXEC)

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

.rc.res:
	@echo RC $@
	@$(WINDRES) -I include -O coff $< $@

clean:
	rm -f quake quake.exe *.o *.res
