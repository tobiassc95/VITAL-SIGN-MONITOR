#ifndef I2S_H
#define I2S_H

#include <stdint.h>
#include "stdbool.h"

//#define NDATA 12//4
#define NDATA 576

typedef enum {
	I2S_8K = 8000,
	I2S_11K025 = 11025,
	I2S_12K = 12000,
	I2S_16K = 16000,
	I2S_22K05 = 22050,
	I2S_24K = 24000,
	I2S_32K = 32000,
	I2S_44K1 = 44100,
	I2S_48K = 48000
}I2S_SAMPRATE;

void I2Sconfigure (I2S_SAMPRATE samplerate);
//void I2Senable (void);
void I2Stransmition(uint16_t data);
void I2StransmitionDMA(uint16_t* data);
bool I2Sdatasent(void);
#endif //I2S_H
