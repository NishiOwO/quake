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
#include <SDL.h>
#include <SDL_audio.h>

#include "quakedef.h"

int snd_inited;

int total = 0;
#define BUFFER_SIZE		4096*4
unsigned char dma_buffer[BUFFER_SIZE];

SDL_AudioSpec spec;
SDL_AudioDeviceID dev;

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

	shm = &sn;
	shm->splitbuffer = 0;

	Con_Printf("16 bit stereo sound initialized\n");

	spec.freq = 44100;
	spec.format = AUDIO_S16;
	spec.channels = 2;
	spec.samples = 4096;
	spec.callback = NULL;
	spec.userdata = NULL;

	SDL_InitSubSystem(SDL_INIT_AUDIO);
	dev = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);

	shm->samplebits = 16;
	shm->channels = 2;
	shm->speed = spec.freq;

	shm->soundalive = true;
	shm->samples = sizeof(dma_buffer) / (shm->samplebits/8);
	shm->samplepos = 0;
	shm->submission_chunk = 1;
	shm->buffer = (unsigned char *)dma_buffer;

	snd_inited = 1;

	SDL_PauseAudioDevice(dev, 0);

	return 1;
}

int SNDDMA_GetDMAPos(void)
{
	return 0;
}

int SNDDMA_GetSamples(void)
{
	return total;
}

void SNDDMA_Shutdown(void)
{
	if (snd_inited) {
		SDL_PauseAudioDevice(dev, 1);
		SDL_ClearQueuedAudio(dev);
		SDL_CloseAudioDevice(dev);
		snd_inited = 0;
	}
}

/*
==============
SNDDMA_Submit

Send sound to device if buffer isn't really the dma buffer
===============
*/

int oldtime = 0;
void SNDDMA_Submit(void)
{
	int t1 = paintedtime;
	int t2 = oldtime;
	int sz;
	if(SDL_GetQueuedAudioSize(dev)) return;
	oldtime = paintedtime;
	sz = (t1 - t2) * 4;
	SDL_QueueAudio(dev, dma_buffer, sz);
	total += sz / 4;
}

