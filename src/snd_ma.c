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

#include "quakedef.h"

int snd_inited;

ma_device device;
ma_device_config config;

int total = 0;
int tbuf = 0;
int quit = 0;

#define BUFFER_SIZE		8192
unsigned char dma_buffer[BUFFER_SIZE];

ma_event ev;

void data_callback(ma_device* device, void* out, const void* in, ma_uint32 frame){
	int sz = 0;
	memset(out, 0, frame * 4);

	ma_event_wait(&ev);
	if(quit) return;

	if(tbuf == 0){
		tbuf = BUFFER_SIZE;
	}
	sz = tbuf > (frame * 4) ? (frame * 4) : tbuf;
	memcpy(out, dma_buffer + (BUFFER_SIZE - tbuf), sz);
	tbuf -= sz;
	total += sz / 2;
	if(tbuf == 0 && sz > 0){
		tbuf = 0;
	}
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

	shm = &sn;
	shm->splitbuffer = 0;

	config = ma_device_config_init(ma_device_type_playback);
	config.playback.format = ma_format_s16;
	config.playback.channels = 2;
	config.sampleRate = 11025;
	config.sampleRate = 44100;
	config.dataCallback = data_callback;
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
	if (!snd_inited)
		return (0);

	return (BUFFER_SIZE - tbuf) % BUFFER_SIZE;
}

int SNDDMA_GetSamples(void)
{
	if (!snd_inited)
		return (0);

	return total;
}

void SNDDMA_Shutdown(void)
{
	if (snd_inited) {
		quit = 1;
		ma_event_signal(&ev);
		ma_device_uninit(&device);
		snd_inited = 0;
		ma_event_uninit(&ev);
	}
}

/*
==============
SNDDMA_Submit

Send sound to device if buffer isn't really the dma buffer
===============
*/
void SNDDMA_Submit(void)
{
	ma_event_signal(&ev);
}

