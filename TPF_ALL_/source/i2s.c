#include "i2s.h"
#include "dma.h"
#include "portpin.h"
#include "hardware.h"

#define WORDSPERFRAME 2 //2 channels
#define BITSPERWORD 16 ///
#define FIFOFILL 4 //6 words are filled each time the watermark is reached.
//Pins.
#define PTC8 PORTNUM2PIN(PORT_C, 8) //MCLK
#define PTC1 PORTNUM2PIN(PORT_C, 1) //TXD0
#define PTB19 PORTNUM2PIN(PORT_B, 19) //TX_FS
#define PTB18 PORTNUM2PIN(PORT_B, 18) //TX_BCLK
#define PTC0 PORTNUM2PIN(PORT_C, 0) //TXD1

static I2S_Type* i2s = I2S0;
static uint16_t data2send[NDATA];
static bool datasent = true;
enum {_8BITS = 1, _16BITS, _32BITS = 4};

void I2Sreset(void);
void I2SmajorDMA(void);

void I2Sreset(void) {
	/* Set the software reset and FIFO reset to clear internal state */
	i2s->TCSR = I2S_TCSR_SR_MASK | I2S_TCSR_FR_MASK;

	/* Clear software reset bit, this should be done by software */
	i2s->TCSR &= ~I2S_TCSR_SR_MASK;

	/* Reset all Tx register values */
	i2s->TCR2 = 0;
	i2s->TCR3 = 0;
	i2s->TCR4 = 0;
	i2s->TCR5 = 0;
	i2s->TMR  = 0;
}

void I2SmajorDMA(void) {
	datasent = true;
}

void I2Sconfigure(I2S_SAMPRATE samplerate) {
	uint16_t fract;
	uint16_t divide;
	uint32_t div;

	if (samplerate == I2S_11K025 || samplerate == I2S_22K05 || samplerate == I2S_44K1) { //Set MCLK = 11.2896 MHz
		fract = 252;
		divide = 2240;
		div = 11289600/(2*samplerate*WORDSPERFRAME*BITSPERWORD) - 1;
	}

	if (samplerate == I2S_8K || samplerate == I2S_12K || samplerate == I2S_16K ||
	    samplerate == I2S_24K || samplerate == I2S_32K || samplerate == I2S_48K) { //Set MCLK = 12.28 MHz
		fract = 6; //28
		divide = 56; //235
		div = 12288000/(2*samplerate*WORDSPERFRAME*BITSPERWORD) - 1;
	}

//	switch (samplerate) { //Calculated with a python algorithm.
//	case I2S_8K:
//		fract = 28;
//		divide = 235;
//		div = 15;
//		break;
//	case I2S_11K025: ///
//		fract = 252;
//		divide = 2240;
//		div = 10;
//		break;
//	case I2S_12K: ///
//		fract = 28;
//		divide = 235;
//		div = 10;
//		break;
//	case I2S_16K:
//		fract = 28;
//		divide = 235;
//		div = 7;
//		break;
//	case I2S_22K05: ///
//		fract = 252;
//		divide = 2240;
//		div = 4;
//		break;
//	case I2S_24K: ///
//		fract = 28;
//		divide = 235;
//		div = 4;
//		break;
//	case I2S_32K:
//		fract = 28;
//		divide = 235;
//		div = 3;
//		break;
//	case I2S_44K1: ///
//		fract = 252;
//		divide = 2240;
//		div = 3;
//		break;
//	case I2S_48K: ///
//		fract = 28;
//		divide = 235;
//		div = 2;
//		break;
//	}

	//div >>= 1; div -= 1; //div [2, 512]

	SIM->SCGC6 |= SIM_SCGC6_I2S_MASK; //turn on CLOCK GATING.

	I2Sreset();

	i2s->TCR1 = I2S_TCR1_TFW(8-FIFOFILL); //Transmit FIFO Watermark. 2 words before empties.////

	i2s->TCR3 = I2S_TCR3_TCE(1); //Enable channel 0.////

	i2s->TCR2 = I2S_TCR2_SYNC(0) | //Asynchronous mode.
				I2S_TCR2_MSEL(1) | //Master Clock (MCLK) 1 option selected (See the chipspecific information for the meaning of each option.).
				I2S_TCR2_BCP_MASK | //Bit clock is active low with drive outputs on falling edge and sample inputs on rising edge (Bit Clock Polarity).
				I2S_TCR2_BCD_MASK | //Bit clock is generated internally in Master mode (Bit Clock Direction).
				I2S_TCR2_DIV(div); //Divides down the audio master clock to generate the bit clock (Bit Clock Divide). The division value is (DIV + 1) * 2.

	i2s->TCR5 = I2S_TCR5_WNW(BITSPERWORD - 1) | //Configures the number of bits in each word, for each word except the first in the frame. Word width of less than 8 bits is not supported.
				I2S_TCR5_W0W(BITSPERWORD - 1) | //Configures the number of bits in the first word in each frame. Word width of less than 8 bits is not supported if there is only one word per frame.
				I2S_TCR5_FBT(BITSPERWORD - 1); //Configures the bit index for the first bit transmitted for each word in the frame.////

	i2s->TMR = 0; //All words are enabled.

	i2s->TCR4 = I2S_TCR4_FRSZ(WORDSPERFRAME - 1) | //Configures the number of words in each frame (Frame Size). The maximum supported frame size is 32 words.
				I2S_TCR4_SYWD(BITSPERWORD - 1) | //Configures the length of the frame sync in number of bit clocks (Sync Width).
				I2S_TCR4_MF_MASK | //MSB is transmitted first.
				I2S_TCR4_FSE_MASK | //Frame sync asserts one bit before the first bit of the frame (Frame Sync Early). i.e. one bit early.///
				I2S_TCR4_FSP_MASK | //Frame sync is active low (Frame Sync Polarity).
				I2S_TCR4_FSD_MASK; //Frame sync is generated internally in Master mode (Frame Sync Direction).

	i2s->MCR = I2S_MCR_MOE_MASK | //MCLK signal pin is configured as an output from the MCLK divider and the MCLK divider is enabled.////
				I2S_MCR_MICS(0); //MCLK divider input clock 0 is selected (See the chipspecific information for the connections to these inputs).////

	i2s->MDR = I2S_MDR_FRACT(fract) | //Sets the MCLK (master clock) divide ratio such that: MCLK output = MCLKinput * ((FRACT + 1)/(DIVIDE + 1)).////
				I2S_MDR_DIVIDE(divide); //FRACT must be set equal or less than the value in the DIVIDE field.////

	//i2s->TCSR = I2S_TCSR_FR_MASK; //reset transmitter FIFO pointers.///
	i2s->TCSR = I2S_TCSR_TE(0) | //Transmitter is disabled
				I2S_TCSR_BCE(1) | //Transmit bit clock is enabled.
				0x001C0000; //clear flags.
				//I2S_TCSR_FRDE_MASK; //Enables the DMA request.///
	i2s->TCSR |= I2S_TCSR_FR_MASK; //reset transmitter FIFO pointers.///

	//Dummy writes.
	i2s->TDR[0] = 0;
	i2s->TDR[0] = 0;
	i2s->TDR[0] = 0;
	i2s->TDR[0] = 0;
	i2s->TDR[0] = 0;
	i2s->TDR[0] = 0;

	//Pins
	PINconfigure(PTC8, PIN_MUX4, PIN_IRQ_DISABLE); //I2S MCLK
	PINconfigure(PTC1, PIN_MUX6, PIN_IRQ_DISABLE); //I2S TXD0 //PIN_MUX6
	PINconfigure(PTB19, PIN_MUX4, PIN_IRQ_DISABLE); //I2S TX FS (FS = frame sync (word sel)).
	PINconfigure(PTB18, PIN_MUX4, PIN_IRQ_DISABLE); //I2S TX BCLK
	PINconfigure(PTC0, PIN_MUX4, PIN_IRQ_DISABLE); //I2S TXD1 //PIN_MUX6

	//DMAconfigure(DMA_CH0, DMA_NORMAL, DMAMUX_I2S0TX, (uint32_t*)data2send, 2, DMA_16BITS, (uint32_t*)i2s->TDR, 0, DMA_16BITS, DMAbytes, 1);
	//DMAconfigure(DMA_CH0, DMA_NORMAL, DMAMUX_I2S0TX, (uint32_t*)data2send, 2, DMA_16BITS, (uint32_t*)i2s->TDR, 0, DMA_16BITS, DMAbytes, 2);
	DMAconfigure(DMA_CH0, DMA_NORMAL, DMAMUX_I2S0TX, (uint32_t*)data2send, _16BITS, DMA_16BITS, _16BITS*NDATA, (uint32_t*)i2s->TDR, 0, DMA_16BITS, 0, _16BITS*FIFOFILL, NDATA/FIFOFILL);
	DMAenableIRQ(DMA_CH0, DMA_INTMAJOR, I2SmajorDMA, NULL);
	DMAenable(DMA_CH0);
}

//void I2Senable (void) {
//	i2s->TCSR |= I2S_TCSR_TE_MASK; //Transmitter is enabled
//}

void I2Stransmition(uint16_t data) {
	//uint32_t i;

	i2s->TCSR |= I2S_TCSR_TE_MASK; //Transmitter is enabled
//	while (!((i2s->TCSR & I2S_TCSR_FWF_MASK) >> I2S_TCSR_FWF_SHIFT)); //FIFO warning flag.
//	i2s->TDR[0] = data;
	if ((i2s->TCSR & I2S_TCSR_FRF_MASK) >> I2S_TCSR_FRF_SHIFT) { //FIFO request flag.
		//i2s->TDR[0] = 0xff00;
		i2s->TDR[0] = data;
	}
	//i = i2s->TFR[0];
}

void I2StransmitionDMA(uint16_t* data) {
	uint16_t j;
	for (j = 0; j < NDATA; j++) {
		data2send[j] = data[j];
	}

	i2s->TCSR |= I2S_TCSR_TE_MASK | //Transmitter is enabled
				I2S_TCSR_FRDE_MASK; //Enables the DMA request.///

	//i = i2s->TFR[0];
}

//void I2StransmitionDMA(uint16_t* data) {
//	uint8_t j;
//	if (datasent) {
//		for (j = 0; j < NDATA; j++) {
//			data2send[j] = data[j];
//		}
//		datasent = false;
//	}
//
//	i2s->TCSR |= I2S_TCSR_TE_MASK | //Transmitter is enabled
//				I2S_TCSR_FRDE_MASK; //Enables the DMA request.///
//
//	//i = i2s->TFR[0];
//}

bool I2Sdatasent(void) {
	if (datasent) {
		datasent = false;
		return true;
	}
	else
		return false;
}

//bool I2Sdatasent(void) {
//	return data2send;
//}
