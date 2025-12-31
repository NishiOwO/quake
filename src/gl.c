#ifdef _WIN32
#define OEMRESOURCE
#include <windows.h>
#else
#define RGFW_USE_XDL
#endif
#define RGFW_OPENGL
#define RGFW_IMPLEMENTATION
/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include <stdarg.h>
#include <stdio.h>
#ifndef _WIN32
#include <signal.h>
#endif

#include "quakedef.h"

#include <RGFW.h>

#define WARP_WIDTH              320
#define WARP_HEIGHT             200

static RGFW_window* win;


unsigned short	d_8to16table[256];
unsigned		d_8to24table[256];
unsigned char	d_15to8table[65536];

static qboolean        mouse_avail;
static qboolean        mouse_active;
static int   mx, my;
static int	old_mouse_x, old_mouse_y;

qboolean dgamouse = qfalse;
qboolean vidmode_ext = qfalse;

static int scr_width, scr_height;

static int default_dotclock_vidmode;
static int num_vidmodes;
static qboolean vidmode_active = qfalse;

/*-----------------------------------------------------------------------*/

//int		texture_mode = GL_NEAREST;
//int		texture_mode = GL_NEAREST_MIPMAP_NEAREST;
//int		texture_mode = GL_NEAREST_MIPMAP_LINEAR;
int		texture_mode = GL_LINEAR;
//int		texture_mode = GL_LINEAR_MIPMAP_NEAREST;
//int		texture_mode = GL_LINEAR_MIPMAP_LINEAR;

int		texture_extension_number = 1;

float		gldepthmin, gldepthmax;

const char *gl_vendor;
const char *gl_renderer;
const char *gl_version;
const char *gl_extensions;

void (*qglColorTableEXT) (int, int, int, int, int, const void*);
void (*qgl3DfxSetPaletteEXT) (GLuint *);

static float vid_gamma = 1.0;

qboolean is8bit = qfalse;
qboolean isPermedia = qfalse;
qboolean gl_mtexable = qfalse;

/*-----------------------------------------------------------------------*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}

void D_EndDirectRect (int x, int y, int width, int height)
{
}

static int RGFWToQuakeKey(u32 keycode)
{
	int key;
	char buf[64];

	key = 0;

	switch(keycode)
	{
		case RGFW_pageUp:	 key = K_PGUP; break;

		case RGFW_pageDown:	 key = K_PGDN; break;

		case RGFW_home:	 key = K_HOME; break;

		case RGFW_end:	 key = K_END; break;

		case RGFW_left:	 key = K_LEFTARROW; break;

		case RGFW_right:	key = K_RIGHTARROW;		break;

		case RGFW_down:	 key = K_DOWNARROW; break;

		case RGFW_up:		 key = K_UPARROW;	 break;

		case RGFW_escape: key = K_ESCAPE;		break;

		case RGFW_return: key = K_ENTER;		 break;

		case RGFW_tab:		key = K_TAB;			 break;

		case RGFW_F1:		 key = K_F1;				break;

		case RGFW_F2:		 key = K_F2;				break;

		case RGFW_F3:		 key = K_F3;				break;

		case RGFW_F4:		 key = K_F4;				break;

		case RGFW_F5:		 key = K_F5;				break;

		case RGFW_F6:		 key = K_F6;				break;

		case RGFW_F7:		 key = K_F7;				break;

		case RGFW_F8:		 key = K_F8;				break;

		case RGFW_F9:		 key = K_F9;				break;

		case RGFW_F10:		key = K_F10;			 break;

		case RGFW_F11:		key = K_F11;			 break;

		case RGFW_F12:		key = K_F12;			 break;

		case RGFW_backSpace: key = K_BACKSPACE; break;

		case RGFW_delete: key = K_DEL; break;

		case RGFW_shiftL:
		case RGFW_shiftR:	key = K_SHIFT;		break;

		case RGFW_controlL:
		case RGFW_controlR:	key = K_CTRL;		 break;

		case RGFW_altL:
		case RGFW_altR: key = K_ALT;			break;

		case RGFW_insert:key = K_INS; break;

		default:
			key = keycode;
			break;
	}

	return key;
}

static void install_grabs(void)
{
	i32 width, height;
	RGFW_window_getSize(win, &width, &height);

	RGFW_window_showMouse(win, 0);
	RGFW_window_holdMouse(win);
	mouse_active = qtrue;
}

static void uninstall_grabs(void)
{
	if (!win)
		return;

	/* TODO */

	RGFW_window_unholdMouse(win);
	mouse_active = qfalse;
}

void focusfunc(RGFW_window* win, u8 inFocus) {
	if (inFocus == RGFW_FALSE) {
		RGFW_window_unholdMouse(win);
	} else {
		if (mouse_active) {
			i32 width, height;
			RGFW_window_getSize(win, &width, &height);

			RGFW_window_unholdMouse(win);
			RGFW_window_holdMouse(win);
		}
	}
}

void mouseposfunc(RGFW_window* win, i32 x, i32 y, float vecX, float vecY) {
	if (mouse_active) {
		mx += vecX * 2;
		my += vecY * 2;
	}
}

void keyfunc(RGFW_window* win, RGFW_key key, u8 keyChar, RGFW_keymod keyMod, RGFW_bool repeat, RGFW_bool pressed) {
   	Key_Event(RGFWToQuakeKey(key), pressed);
}

void mousebuttonfunc(RGFW_window* win, u8 button, u8 pressed) {
	int b = -1;
	if (button== RGFW_mouseLeft)
		b = 0;
	else if (button == RGFW_mouseMiddle)
		b = 2;
	else if (button== RGFW_mouseRight)
		b = 1;
	if (b >= 0)
		Key_Event(K_MOUSE1 + b, pressed);
}

void scrollfunc(RGFW_window* win, float x, float y) {
	if (y > 0 ) {
		Key_Event(K_MWHEELUP, qtrue);
		Key_Event(K_MWHEELUP, qfalse);
	} else if (y < 0) {
		Key_Event(K_MWHEELDOWN, qtrue);
		Key_Event(K_MWHEELDOWN, qfalse);
	}
}

static void HandleEvents(void)
{
	int b;
	qboolean dowarp = qfalse;
	int mwx = vid.width/2;
	int mwy = vid.height/2;

	RGFW_pollEvents();

	if (dowarp) {
		/* move the mouse to the window center again */
	}

}

static void IN_DeactivateMouse( void )
{
	if (!mouse_avail || !win)
		return;

	if (mouse_active) {
		uninstall_grabs();
		mouse_active = qfalse;
	}
}

static void IN_ActivateMouse( void )
{
	if (!mouse_avail || !win)
		return;

	if (!mouse_active) {
		mx = my = 0; // don't spazz
		install_grabs();
		mouse_active = qtrue;
	}
}


void VID_Shutdown(void)
{
	IN_DeactivateMouse();
	if (win) {
		RGFW_window_close(win);
	}
	vidmode_active = qfalse;
	win = NULL;
}

void signal_handler(int sig)
{
	printf("Received signal %d, exiting...\n", sig);
	Sys_Quit();
	exit(0);
}

void InitSig(void)
{
#ifndef _WIN32
	signal(SIGHUP, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);
	signal(SIGILL, signal_handler);
	signal(SIGTRAP, signal_handler);
	signal(SIGIOT, signal_handler);
	signal(SIGBUS, signal_handler);
	signal(SIGFPE, signal_handler);
	signal(SIGSEGV, signal_handler);
	signal(SIGTERM, signal_handler);
#endif
}

void VID_ShiftPalette(unsigned char *p)
{
//	VID_SetPalette(p);
}

void	VID_SetPalette (unsigned char *palette)
{
	byte	*pal;
	unsigned r,g,b;
	unsigned v;
	int     r1,g1,b1;
	int		j,k,l,m;
	unsigned short i;
	unsigned	*table;
	FILE *f;
	char s[255];
	int dist, bestdist;

//
// 8 8 8 encoding
//
	pal = palette;
	table = d_8to24table;
	for (i=0 ; i<256 ; i++)
	{
		r = pal[0];
		g = pal[1];
		b = pal[2];
		pal += 3;

		v = (255<<24) + (r<<0) + (g<<8) + (b<<16);
		*table++ = v;
	}
	d_8to24table[255] &= 0xffffff;	// 255 is transparent

	for (i=0; i < (1<<15); i++) {
		/* Maps
		000000000000000
		000000000011111 = Red  = 0x1F
		000001111100000 = Blue = 0x03E0
		111110000000000 = Grn  = 0x7C00
		*/
		r = ((i & 0x1F) << 3)+4;
		g = ((i & 0x03E0) >> 2)+4;
		b = ((i & 0x7C00) >> 7)+4;
		pal = (unsigned char *)d_8to24table;
		for (v=0,k=0,bestdist=10000*10000; v<256; v++,pal+=4) {
			r1 = (int)r - (int)pal[0];
			g1 = (int)g - (int)pal[1];
			b1 = (int)b - (int)pal[2];
			dist = (r1*r1)+(g1*g1)+(b1*b1);
			if (dist < bestdist) {
				k=v;
				bestdist = dist;
			}
		}
		d_15to8table[i]=k;
	}
}

/*
===============
GL_Init
===============
*/
void GL_Init (void)
{
	gl_vendor = glGetString (GL_VENDOR);
	Con_Printf ("GL_VENDOR: %s\n", gl_vendor);
	gl_renderer = glGetString (GL_RENDERER);
	Con_Printf ("GL_RENDERER: %s\n", gl_renderer);

	gl_version = glGetString (GL_VERSION);
	Con_Printf ("GL_VERSION: %s\n", gl_version);
	gl_extensions = glGetString (GL_EXTENSIONS);
//	Con_Printf ("GL_EXTENSIONS: %s\n", gl_extensions);

//	Con_Printf ("%s %s\n", gl_renderer, gl_version);

	glClearColor (1,0,0,0);
	glCullFace(GL_FRONT);
	glEnable(GL_TEXTURE_2D);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.666);

	glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	glShadeModel (GL_FLAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

/*
=================
GL_BeginRendering

=================
*/
void GL_BeginRendering (int *x, int *y, int *width, int *height)
{
	extern cvar_t gl_clear;

	*x = *y = 0;
	*width = scr_width;
	*height = scr_height;

//    if (!wglMakeCurrent( maindc, baseRC ))
//		Sys_Error ("wglMakeCurrent failed");

//	glViewport (*x, *y, *width, *height);
}


void GL_EndRendering (void)
{
	glFlush();
	RGFW_window_swapBuffers_OpenGL(win);
}

qboolean VID_Is8bit(void)
{
	return is8bit;
}

static void Check_Gamma (unsigned char *pal)
{
	float	f, inf;
	unsigned char	palette[768];
	int		i;

	if ((i = COM_CheckParm("-gamma")) == 0) {
		if ((gl_renderer && strstr(gl_renderer, "Voodoo")) ||
			(gl_vendor && strstr(gl_vendor, "3Dfx")))
			vid_gamma = 1;
		else
			vid_gamma = 0.7; // default to 0.7 on non-3dfx hardware
	} else
		vid_gamma = Q_atof(com_argv[i+1]);

	for (i=0 ; i<768 ; i++)
	{
		f = pow ( (pal[i]+1)/256.0 , vid_gamma );
		inf = f*255 + 0.5;
		if (inf < 0)
			inf = 0;
		if (inf > 255)
			inf = 255;
		palette[i] = inf;
	}

	memcpy (pal, palette, sizeof(palette));
}

void VID_Init(unsigned char *palette)
{
	int i;
	char	gldir[MAX_OSPATH];
	int width = 640, height = 480;
	unsigned long mask;
	qboolean fullscreen = qtrue;
	int MajorVersion, MinorVersion;
	int actualWidth, actualHeight;

	mouse_avail = 1;

	vid.maxwarpwidth = WARP_WIDTH;
	vid.maxwarpheight = WARP_HEIGHT;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));

// interpret command-line params

// set vid parameters
	if ((i = COM_CheckParm("-window")) != 0)
		fullscreen = qfalse;

	if ((i = COM_CheckParm("-width")) != 0)
		width = atoi(com_argv[i+1]);

	if ((i = COM_CheckParm("-height")) != 0)
		height = atoi(com_argv[i+1]);

	if ((i = COM_CheckParm("-conwidth")) != 0)
		vid.conwidth = Q_atoi(com_argv[i+1]);
	else
		vid.conwidth = 640;

	vid.conwidth &= 0xfff8; // make it a multiple of eight

	if (vid.conwidth < 320)
		vid.conwidth = 320;

	// pick a conheight that matches with correct aspect
	vid.conheight = vid.conwidth*3 / 4;

	if ((i = COM_CheckParm("-conheight")) != 0)
		vid.conheight = Q_atoi(com_argv[i+1]);
	if (vid.conheight < 200)
		vid.conheight = 200;

	win = RGFW_createWindow("Quake", 0, 0, width, height, RGFW_windowCenter | RGFW_windowOpenGL);
	RGFW_window_makeCurrentWindow_OpenGL(win);

	RGFW_setMousePosCallback(mouseposfunc);
	RGFW_setMouseScrollCallback(scrollfunc);
	RGFW_setFocusCallback(focusfunc);
	RGFW_setKeyCallback(keyfunc);
	RGFW_setMouseButtonCallback(mousebuttonfunc);

	if(fullscreen){
		RGFW_monitor mon = RGFW_window_getMonitor(win);
		width = mon.mode.w;
		height = mon.mode.h;

		RGFW_window_resize(win, width, height);
		RGFW_window_setFullscreen(win, 1);
	}

	scr_width = width;
	scr_height = height;

	if (vid.conheight > height)
		vid.conheight = height;
	if (vid.conwidth > width)
		vid.conwidth = width;
	vid.width = vid.conwidth;
	vid.height = vid.conheight;

	vid.aspect = ((float)vid.height / (float)vid.width) * (320.0 / 240.0);
	vid.numpages = 2;

	InitSig(); // trap evil signals

	GL_Init();

	sprintf (gldir, "%s/glquake", com_gamedir);
	Sys_mkdir (gldir);

	VID_SetPalette(palette);

	Con_SafePrintf ("Video mode %dx%d initialized.\n", width, height);

	vid.recalc_refdef = 1;				// force a surface cache flush
}

void Sys_SendKeyEvents(void)
{
	if(win == NULL) return;
	HandleEvents();
}

void Force_CenterView_f (void)
{
	cl.viewangles[PITCH] = 0;
}

void IN_Init(void)
{
}

void IN_Shutdown(void)
{
}

/*
===========
IN_Commands
===========
*/
void IN_Commands (void)
{
	if (!win)
		return;

	if (vidmode_active || key_dest == key_game)
		IN_ActivateMouse();
	else
		IN_DeactivateMouse ();
}

/*
===========
IN_Move
===========
*/
void IN_MouseMove (usercmd_t *cmd)
{
	if (!mouse_avail)
		return;

	if (1)
	{
		mx = (mx + old_mouse_x) * 0.5;
		my = (my + old_mouse_y) * 0.5;
	}
	old_mouse_x = mx;
	old_mouse_y = my;

	mx *= sensitivity.value;
	my *= sensitivity.value;

// add mouse X/Y movement to cmd
	if ( (in_strafe.state & 1) || (lookstrafe.value && (in_mlook.state & 1) ))
		cmd->sidemove += m_side.value * mx;
	else
		cl.viewangles[YAW] -= m_yaw.value * mx;

	if (in_mlook.state & 1)
		V_StopPitchDrift ();

	if ( (in_mlook.state & 1) && !(in_strafe.state & 1))
	{
		cl.viewangles[PITCH] += m_pitch.value * my;
		if (cl.viewangles[PITCH] > 80)
			cl.viewangles[PITCH] = 80;
		if (cl.viewangles[PITCH] < -70)
			cl.viewangles[PITCH] = -70;
	}
	else
	{
		if ((in_strafe.state & 1) && noclip_anglehack)
			cmd->upmove -= m_forward.value * my;
		else
			cmd->forwardmove -= m_forward.value * my;
	}
	mx = my = 0;
}

void IN_Move (usercmd_t *cmd)
{
	IN_MouseMove(cmd);
}


