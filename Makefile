TARGET = windows
DEBUG = -O2
SND = ma

detected_OS := $(shell uname 2>/dev/null || echo Unknown)

ifeq ($(detected_OS),Darwin)        # Mac OS X
	TARGET = macos
endif
ifneq (,$(filter $(detected_OS),Linux NetBSD FreeBSD OpenBSD))
	TARGET = unix
endif

ifneq (,$(filter $(detected_OS),NetBSD))
	INCLUDES += -I/usr/X11R7/include
	LIBRARIES += -L/usr/X11R7/lib
endif

ifeq ($(TARGET),unix)
OPENGL_CFLAGS = 
OPENGL_LIBS = -lGL -lGLU 
else ifeq ($(TARGET),windows)
OPENGL_CFLAGS =
OPENGL_LIBS = -lopengl32 -lglu32
else ifeq ($(TARGET),macos)
OPENGL_CFLAGS =
OPENGL_LIBS = -framework OpenGL
endif



EXEC =

CC = $(GCC_PREFIX)gcc
WINDRES = $(GCC_PREFIX)windres
CFLAGS = $(DEBUG) -fomit-frame-pointer -funroll-loops -fcommon -DGL_EXT_SHARED -DGLQUAKE -Iinclude -Wno-error=implicit-function-declaration $(INCLUDES) $(OPENGL_CFLAGS) -std=c99 -D_DEFAULT_SOURCE
LDFLAGS = $(LIBRARIES)
LIBS = $(OPENGL_LIBS)

.PHONY: all clean
.SUFFIXES: .c .o .rc .res

OBJS = $(shell cat list) src/snd_$(SND).o

ifdef QUAKE2
CFLAGS += -DQUAKE2
endif

ifeq ($(TARGET),unix)
OBJS += src/net_udp.o src/net_bsd.o src/sys.o
CFLAGS += 
LIBS += -lm -lpthread -lX11 -lXrandr
else ifeq ($(TARGET),macos)
OBJS += src/net_udp.o src/net_bsd.o src/sys.o
LIBS += -lm -lpthread -framework CoreVideo -framework Cocoa -framework IOKit
else ifeq ($(TARGET),windows)
OBJS += src/net_udp.o src/net_bsd.o src/sys_win.o src/conproc.o src/quake.res
CFLAGS += -mwindows
LIBS += -lgdi32 -lwsock32
EXEC = .exe
endif

ifeq ($(SND),sdl2)
CFLAGS += `sdl2-config --cflags`
LIBS += `sdl2-config --libs`
else ifeq ($(SND),ma)
OBJS += src/miniaudio.o
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

src/.c.o:
	@echo CC $@
ifdef DEV
	@rm -f $@.log
	@$(CC) $(CFLAGS) -c -o $@ $< >$@.log 2>&1
	@./proc.sh $@ $<
	@rm -f $@.log
else
	@$(CC) $(CFLAGS) -c -o $@ $<
endif

src/quake.res: src/quake.rc 
	@echo RC $@
	@$(WINDRES) -I include -O coff $< $@

clean:
	rm -f quake quake.exe res/*.res src/*.o
