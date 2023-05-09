#ifndef ADC_H
#define ADC_H

//#define DEBUG_

#include <stdint.h>
#include <stdbool.h>

//sample mode
//#define SHORTSAMPLE 0
//#define LONGSAMPLE 1

typedef enum {
	ADC_0,
	ADC_1
}ADC_MODULE;

typedef enum { //resolution
	ADC_8BITS,
	ADC_12BITS,
	ADC_10BITS,
	ADC_16BITS
}ADC_MODE;

void ADCconfigure (uint8_t adc_, uint8_t mode); //fs = (fclk)/(clkdiv*sampt) -> fclk = 25MHz (?)
void ADCstart (uint8_t adc_, uint16_t sampfreq, uint8_t samples);
void ADCstop (uint8_t adc_); //disables ADC
//void enableADCinterrupt(uint8_t adc_);
//void disableADCinterrupt(uint8_t adc_);
void getADCdata(uint8_t adc_, uint16_t* data_, uint8_t samples);
bool ADCdata2read(uint8_t adc_);

#endif //ADC_H
