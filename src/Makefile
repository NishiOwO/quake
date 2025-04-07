CC = cc
CFLAGS = -DGLQUAKE -Iinclude `pkg-config --cflags x11` `pkg-config --cflags gl`
LDFLAGS =
LIBS = `pkg-config --libs x11` `pkg-config --libs gl`

.PHONY: all clean
.SUFFIXES: .c .o

all: quake

OBJS = cl_cam.o cl_demo.o cl_ents.o cl_input.o cl_main.o cl_parse.o cl_pred.o cl_tent.o cmd.o common.o console.o crc.o cvar.o gl_draw.o gl_mesh.o gl_model.o gl_ngraph.o gl_refrag.o gl_rlight.o gl_rmain.o gl_rmisc.o gl_rsurf.o gl_screen.o gl_vidlinuxglx.o gl_warp.o keys.o mathlib.o md4.o menu.o net_chan.o net_udp.o nonintel.o pmove.o pmovetst.o r_part.o sbar.o screen.o skin.o snd_dma.o snd_linux.o snd_mem.o snd_mix.o sys_linux.o view.o wad.o zone.o 

quake: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f quake *.o
