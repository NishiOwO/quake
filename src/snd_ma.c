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
#include <miniaudio.h>
#define NEW_WAY

#include "quakedef.h"
#ifdef NEW_WAY
#include <stb_ds.h>
#endif

int snd_inited;

ma_device device;
ma_device_config config;

int total = 0;
int tbuf = 0;
int quit = 0;
#define BUFFER_SIZE		4096*4
unsigned char dma_buffer[BUFFER_SIZE];

ma_event ev;
ma_mutex mx;

ma_bool8 ready = MA_FALSE;

#ifdef NEW_WAY
typedef struct buffer_t {
	int pos;
	int size;
	unsigned char* buffer;
} buffer_t;
int buffer_size = 0;
buffer_t* buffer_list = NULL;
#endif

void data_callback(ma_device* device, void* out, const void* in, ma_uint32 frame){
    int bsz = 0;
    memset(out, 0, frame * 4);

    if(quit) return;
 
    if (ready == MA_FALSE) {
        return;
    }

    ma_mutex_lock(&mx);
#ifdef NEW_WAY
    if(arrlen(buffer_list) > 0){
	buffer_t* buf = &buffer_list[0];
	int fsz = frame * 4;
	while(fsz > 0){
		int sz = 0;
		int i;
		if(fsz > (buf->size - buf->pos)){
			sz = buf->size - buf->pos;
		}else{
			sz = fsz;
		}
		memcpy(bsz + (unsigned char*)out, buf->buffer + buf->pos, sz);
		buf->pos += sz;
		fsz -= sz;
		bsz += sz;
		total += sz / 4;
		if(buf->pos >= buf->size){
			free(buf->buffer);
			arrdel(buffer_list, 0);
			buf = &buffer_list[0];
		}
		if(arrlen(buffer_list) == 0) break;
	}
    }
#else
	if(tbuf == 0){
		tbuf = BUFFER_SIZE;
	}
	bsz = tbuf > (frame * 4) ? (frame * 4) : tbuf;
	memcpy(out, dma_buffer + (BUFFER_SIZE - tbuf), bsz);
	tbuf -= bsz;

    if (tbuf <= 0) {
        ready = MA_FALSE;
    }

    total += bsz / 2;
#endif

	ma_mutex_unlock(&mx);
}

qboolean SNDDMA_Init(void)
{
	int rc;
	int fmt;
	int tmp;
	int i;
	char *s;
	int caps;

	if (snd_inited) {
		printf("Sound already init'd\n");
		return 0;
	}

	ma_event_init(&ev);
	ma_mutex_init(&mx);

	shm = &sn;
	shm->splitbuffer = 0;

	config = ma_device_config_init(ma_device_type_playback);
	config.playback.format = ma_format_s16;
	config.playback.channels = 2;
	//config.sampleRate = 8000;
	//config.sampleRate = 22050;
	config.sampleRate = 44100;
	config.dataCallback = data_callback;
	config.periodSizeInFrames = 4096;
	if(ma_device_init(NULL, &config, &device) != MA_SUCCESS){
		return 0;
	}
	Con_Printf("16 bit stereo sound initialized\n");
    ma_device_start(&device);

	shm->samplebits = 16;
	shm->channels = config.playback.channels;
	shm->speed = config.sampleRate;

	shm->soundalive = true;
	shm->samples = sizeof(dma_buffer) / (shm->samplebits/8);
	shm->samplepos = 0;
	shm->submission_chunk = 1;
	shm->buffer = (unsigned char *)dma_buffer;

	snd_inited = 1;

	return 1;
}

int SNDDMA_GetDMAPos(void)
{
#ifdef NEW_WAY
	return 0;
#else
	if (!snd_inited)
		return (0);

	return (BUFFER_SIZE - tbuf) % BUFFER_SIZE;
#endif
}

int SNDDMA_GetSamples(void)
{
#ifdef NEW_WAY
	return total;
#else
	if (!snd_inited)
		return (0);

	return total;
#endif
}

void SNDDMA_Shutdown(void)
{
	if (snd_inited) {
		quit = 1;
		ma_event_signal(&ev);
		ma_device_uninit(&device);
		snd_inited = 0;
		ma_event_uninit(&ev);
		ma_mutex_uninit(&mx);
	}
}

/*
==============
SNDDMA_Submit

Send sound to device if buffer isn't really the dma buffer
===============
*/

#ifdef NEW_WAY
int oldtime = 0;
#endif
void SNDDMA_Submit(void)
{
#ifdef NEW_WAY
	int t1 = paintedtime;
	int t2 = oldtime;
	buffer_t buf;
	buf.size = (t1 - t2) * 4;
	buf.pos = 0;
	if(buf.size == 0) return;
	oldtime = paintedtime;

	buf.buffer = malloc(buf.size);
	ma_mutex_lock(&mx);
	memcpy(buf.buffer, dma_buffer, buf.size);
	arrput(buffer_list, buf);
	ma_mutex_unlock(&mx);
#else
	ma_event_signal(&ev);
#endif
	ready = MA_TRUE;
}

