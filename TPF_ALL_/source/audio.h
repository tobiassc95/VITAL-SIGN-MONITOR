#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

typedef enum {
//	AUDIO_THOMAS,
//	AUDIO_SONG,
	AUDIO_NONE,
	AUDIO_CARDIO,
	AUDIO_OXI,
	AUDIO_TEMP
} AUDIO_MP3FILE;

typedef enum {
	AUDIO_SINE,
	AUDIO_SQUARE
} AUDIO_WAVE;

void audioInit(void);
bool audioPlaying(void);
int audioPlay(AUDIO_MP3FILE file);
void audioPlayWave(AUDIO_WAVE wave);
void audioShutDown(void);

#endif //AUDIO_H
