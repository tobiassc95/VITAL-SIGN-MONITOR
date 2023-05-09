#include "audio.h"
#include "uda1380.h"
#include "i2s.h"
#include "flash.h"
#include "lib/helix/pub/mp3dec.h"
#include <stddef.h>
//#define _USE_MATH_DEFINES
#include <math.h>
#include "cardio_array.h"
#include "oxi_array.h"
#include "temp_array.h"

#define BYTES2READ 1000///

static HMP3Decoder hMP3dec;
static uint16_t buff2send[NDATA];
static bool playing = false;
static uint8_t *cardio_ptr;
static uint8_t *oxi_ptr;
static uint8_t *temp_ptr;

void audioInit(void) {
	uint16_t i;
	flashWrite(cardio_array, CARDIO_ARRAY_SIZE, &cardio_ptr);
	flashWrite(oxi_array, OXI_ARRAY_SIZE, &oxi_ptr);
	flashWrite(temp_array, TEMP_ARRAY_SIZE, &temp_ptr);

//	for (i = 0; i < CARDIO_ARRAY_SIZE; i++)
//		if(cardio_ptr[i] != cardio_array[i])
//			return;
//	for (i = 0; i < OXI_ARRAY_SIZE; i++)
//		if(oxi_ptr[i] != oxi_array[i])
//			return;
//	for (i = 0; i < TEMP_ARRAY_SIZE; i++)
//		if(temp_ptr[i] != temp_array[i])
//			return;

	UDA1380Init();
	hMP3dec = MP3InitDecoder();
//	if (hMP3dec == NULL) {
//		return;
//	}
	I2Sconfigure(I2S_8K); //i2s config
	//I2Senable();
}

bool audioPlaying(void) {
	return playing;
}

int audioPlay(AUDIO_MP3FILE file) {
	//DIR directory; /* Directory object */
	//FILINFO fileInformation;
	int8_t err;
	static uint8_t mp3buff[BYTES2READ], *mp3ptr, *mp3array; //unsigned char
	//int16_t decbuff[/*MAX_NGRAN * */MAX_NCHAN * MAX_NSAMP];
	MP3FrameInfo mp3frameinfo;
	uint16_t offset;
	static uint16_t bytesleft, bytesdec;
	static bool playing_ = false;
	uint16_t i;

	if (file == AUDIO_NONE && !playing)
		return 0;

	if (!playing) {
		switch(file) {
		case AUDIO_CARDIO:
			mp3ptr = cardio_ptr;
			bytesleft = CARDIO_ARRAY_SIZE;
			break;
		case AUDIO_OXI:
			mp3ptr = oxi_ptr;
			bytesleft = OXI_ARRAY_SIZE;
			break;
		case AUDIO_TEMP:
			mp3ptr = temp_ptr;
			bytesleft = TEMP_ARRAY_SIZE;
			break;
		default:
			return -1;
		}
		playing = true;
		playing_ = true;
		bytesdec = BYTES2READ;
	}

	if (I2Sdatasent()) {
		if (playing_) {
			if(bytesleft == 0) {
				playing_ = false;
				for (i = 0; i < NDATA; i++) {
					buff2send[i] = 0;
				}
				I2StransmitionDMA(buff2send);
			}
			else {
				//memcpy(&mp3buff[BYTES2READ - bytesdec], mp3array, bytesdec); //Read the whole file.
				//mp3ptr = mp3buff;

				bytesdec = bytesleft;
				/* find start of next MP3 frame - assume EOF if no sync found */
				offset = MP3FindSyncWord(mp3ptr, bytesleft);
				if (offset == -1) { ///if there isn't a sync word, then EOF (we read the whole file).
					playing_ = false;
					for (i = 0; i < NDATA; i++) {
						buff2send[i] = 0;
					}
					I2StransmitionDMA(buff2send);
				}
				else {
					mp3ptr += offset; ///update variables...
					bytesleft -= offset;

					/* decode one MP3 frame - if offset < 0 then bytesLeft was less than a full frame */ ///or EOF.
					err = MP3Decode(hMP3dec, &mp3ptr, &bytesleft, buff2send, 0);
					if (err == ERR_MP3_NONE) { /* no error */
						MP3GetLastFrameInfo(hMP3dec, &mp3frameinfo); ///mp3frameinfo is fill with the frame information (?).
						bytesdec -= bytesleft;
						//memmove(mp3buff, &mp3buff[bytesdec], BYTES2READ - bytesdec);
						//mp3array = &mp3array[bytesdec];
						//I2StransmitionDMA(buff2send, mp3frameinfo.outputSamps);
						I2StransmitionDMA(buff2send);
					}
					else {
						playing_ = false;
						for (i = 0; i < NDATA; i++) {
							buff2send[i] = 0;
						}
						I2StransmitionDMA(buff2send);
					}
				}
			}
		}
		else
			playing = false;
	}
}

int audioPlayBLKG(AUDIO_MP3FILE file) {
	int8_t err;
	uint8_t mp3buff[BYTES2READ], *mp3ptr; //unsigned char
	//int16_t decbuff[/*MAX_NGRAN * */MAX_NCHAN * MAX_NSAMP];
	MP3FrameInfo mp3frameinfo;
	uint16_t bytesleft, bytesread, bytesdec, offset;

	if (file == AUDIO_NONE && !playing)
		return 0;

	if (!playing) {
		switch(file) {
		case AUDIO_CARDIO:
			mp3ptr = cardio_ptr;
			bytesleft = CARDIO_ARRAY_SIZE;
			break;
		case AUDIO_OXI:
			mp3ptr = oxi_ptr;
			bytesleft = OXI_ARRAY_SIZE;
			break;
		case AUDIO_TEMP:
			mp3ptr = temp_ptr;
			bytesleft = TEMP_ARRAY_SIZE;
			break;
		default:
			return -1;
		}
		playing = true;
		bytesdec = BYTES2READ;
	}

	while (1) {
		//memcpy(&mp3buff[BYTES2READ - bytesdec], mp3array, bytesdec); //Read the whole file.
		//mp3ptr = mp3buff;

		bytesdec = bytesleft;
		/* find start of next MP3 frame - assume EOF if no sync found */
		offset = MP3FindSyncWord(mp3ptr, bytesleft);
		if (offset == -1) ///if there isn't a sync word, then EOF (we read the whole file).
			break;

		mp3ptr += offset; ///update variables...
		bytesleft -= offset;

		/* decode one MP3 frame - if offset < 0 then bytesLeft was less than a full frame */ ///or EOF.
		err = MP3Decode(hMP3dec, &mp3ptr, &bytesleft, buff2send, 0);
		if (err == ERR_MP3_NONE) { /* no error */
			MP3GetLastFrameInfo(hMP3dec, &mp3frameinfo); ///mp3frameinfo is fill with the frame information (?).
			bytesdec -= bytesleft;
			//memmove(mp3buff, &mp3buff[bytesdec], BYTES2READ - bytesdec);
			//mp3array = &mp3array[bytesdec];
			//I2StransmitionDMA(buff2send, mp3frameinfo.outputSamps);
			while (!I2Sdatasent());
			I2StransmitionDMA(buff2send);
		}
		else
			break;
	}
}

void audioPlayWave(AUDIO_WAVE wave) {
	//Create some variables to help us generate a sine wave
	static bool temp = true;
	static int i = 0;
	static float freq = 440;
	int samprate = I2S_16K, j; //Sample rate.

	if (I2Sdatasent()) {
		if (wave == AUDIO_SINE) {
			//Write the samples to the sample buffer
			for (j = 0; j < NDATA; j++) {
//				buff2send[j] = INT16_MAX * sin(2*M_PI*freq*(i++ / (float)samprate));
				if (i == samprate)
					i = 0;
			}
			if (temp) {
				freq += 1;
				if(freq > 20000)
					temp = false;
			}
			else {
				freq -= 1;
				if(freq < 20)
					temp = true;
			}
		}
		else if (wave == AUDIO_SQUARE) {
			//Write the samples to the sample buffer
			for (j = 0; j < NDATA; j++) {
				if (temp)
					buff2send[j] = 0xfff;
				else
					buff2send[j] = 0xf;
			}
			if (temp)
				temp = false;
			else
				temp = true;
		}
		//I2StransmitionDMA(buff2send, NDATA);
		I2StransmitionDMA(buff2send);
	}
}

void audioShutDown(void) {
	MP3FreeDecoder(hMP3dec);
}
