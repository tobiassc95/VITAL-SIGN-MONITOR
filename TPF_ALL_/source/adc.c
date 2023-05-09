#include "adc.h"
#include "hardware.h"
#include "PIT_driver.h"
//#include "dma.h"
#include <stdbool.h>
//#include "portpin.h"

typedef enum { //SEE CHIP CONFIGURATION.
	ADC_CHDP0,
	ADC_CHDP1,
	ADC_CHDP2,
	ADC_CHDP3
}ADC_CHANNEL;

typedef enum {
	ADC_DIV1,
	ADC_DIV2,
	ADC_DIV4,
	ADC_DIV8
}ADC_CLKDIV;

typedef enum { //sample time = convertion time. //cycles = clock cycles.
	ADC_24CYCLES,
	ADC_16CYCLES,
	ADC_10CYCLES,
	ADC_6CYCLES,
	ADC_4CYCLES,
}ADC_SAMPTIME;

static ADC_Type* adc[2] = {ADC0, ADC1};
//static uint16_t adc0data; //el dato a guardar en memoria a traves de la dma.
static uint16_t adcNsamp;
static uint16_t adcsamples[256];
static bool ADCdataRdy = false; //flag para mandar el  valor.

//void ADCmajorDMA(void);
void ADC0start (void);

void ADCconfigure (uint8_t adc_, uint8_t mode) {
	uint8_t clkdiv = ADC_DIV8; //fs = (fclk)/(clkdiv*sampt) -> fclk = 25MHz (?)
	uint8_t sampt = ADC_24CYCLES;

	//CLOCK GATING
	if(adc_ == ADC_0)
		SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;
	else if(adc_ == ADC_1)
		SIM->SCGC3 |= SIM_SCGC3_ADC1_MASK;

	//CONFIG
	adc[adc_]->CFG1 = ADC_CFG1_ADIV(clkdiv) | ADC_CFG1_MODE(mode) | ADC_CFG1_ADICLK(1); //ADICLK can be 0(BUSCLOCK) or 1(BUSCLOCK/2).
	adc[adc_]->SC2 = ADC_SC2_ADTRG(0);
	if(sampt != ADC_4CYCLES) {
		adc[adc_]->CFG1 |= ADC_CFG1_ADLSMP(1);
		adc[adc_]->CFG2 = ADC_CFG2_ADLSTS(sampt);
	}

//	adc[adc_]->SC2 |= ADC_SC2_DMAEN_MASK;    // DMA Enable
//	// DMA configuration for ADC, data is an unsigned 16-bit variable for data receive.
//	DMAconfigure(DMA_CH0, DMA_NORMAL, DMAMUX_ADC0, (uint32_t*)(adc[adc_]->R), 0, DMA_16BITS, 0, (uint32_t*)(&adc0data), 0, DMA_16BITS, 0, 2, 1);
//	//DMAconfigure(DMA_CH0, DMA_NORMAL, DMAMUX_ADC0, (uint32_t*)(adc[adc_]->R), 0, DMA_16BITS, 0, (uint32_t*)(adc0data), 2, DMA_16BITS, 2*Nsamp, 2*Nsamp, 1);
//	DMAenableIRQ(DMA_CH0, DMA_INTMAJOR, ADCmajorDMA, NULL);
//	DMAenable(DMA_CH0);

	PIT_init();
	PIT_setRutine(PIT0, ADC0start);
}

void ADCstart (uint8_t adc_, uint16_t sampfreq, uint8_t samples) {
	//adc[adc_]->CFG2 = (adc[adc_]->CFG2 & ~ADC_CFG2_MUXSEL_MASK) | ADC_CFG2_MUXSEL(0); //???
	//adc[adc_]->SC1[0] = ADC_SC1_DIFF(0) | ADC_SC1_ADCH(ch_) | ADC_SC1_AIEN_MASK;

	adcNsamp = samples;
	PIT_start(PIT0, 1000000000/sampfreq);
	if(adc_ == ADC_0)
		NVIC_EnableIRQ(ADC0_IRQn);
	else if(adc_ == ADC_1)
		NVIC_EnableIRQ(ADC1_IRQn);
}

void ADC0start(void) {
	adc[ADC_0]->SC1[0] = ADC_SC1_DIFF(0) | ADC_SC1_ADCH(ADC_CHDP1) | ADC_SC1_AIEN_MASK;
}

void ADCstop (uint8_t adc_) {
	PIT_stop(PIT0);
	if(adc_ == ADC_0)
		NVIC_DisableIRQ(ADC0_IRQn);
	else if(adc_ == ADC_1)
		NVIC_DisableIRQ(ADC1_IRQn);
	adc[adc_]->SC1[0] |= ADC_SC1_ADCH(0x1F);
}

//void enableADCinterrupt(uint8_t adc_) {
//	if(adc_ == ADC_0)
//		NVIC_EnableIRQ(ADC0_IRQn);
//	else if(adc_ == ADC_1)
//		NVIC_EnableIRQ(ADC1_IRQn);
//}
//
//void disableADCinterrupt(uint8_t adc_) {
//	if(adc_ == ADC_0)
//		NVIC_DisableIRQ(ADC0_IRQn);
//	else if(adc_ == ADC_1)
//		NVIC_DisableIRQ(ADC1_IRQn);
//}

//uint16_t getADCdata(uint8_t adc_) {
////	if(adc[adc_]->SC2 & ADC_SC2_DMAEN_MASK) //si se usa el dma, se devuelve el dato guardado en memoria.
////		return adc0data;
//	return adc[adc_]->R[0];
//}

void getADCdata(uint8_t adc_, uint16_t* data_, uint8_t samples) {
	uint8_t i = 0;
	if(samples > adcNsamp)
		return;
	for(i=0; i<samples; i++) {
		data_[i] = adcsamples[i];
	}
	//return adc[adc_]->R[0];
}

bool ADCdata2read(uint8_t adc_) {
	if (ADCdataRdy) {
		ADCdataRdy = false;
		return true;
	}
	else
		return false;
//	if(adc[adc_]->SC1[0] & ADC_SC1_COCO_MASK)
//		return (adc[adc_]->SC1[0] & ADC_SC1_COCO_MASK)>>ADC_SC1_COCO_SHIFT;
//	return false;
}

__ISR__ ADC0_IRQHandler(void) {
	static uint16_t i = 0;
	NVIC_DisableIRQ(ADC0_IRQn);

	if(adc[ADC_0]->SC1[0] & ADC_SC1_COCO_MASK) {
		adcsamples[i++] = adc[ADC_0]->R[0];
		if(i >= adcNsamp) {
			i = 0;
			ADCdataRdy = true;
		}
	}

	NVIC_EnableIRQ(ADC0_IRQn);
}
