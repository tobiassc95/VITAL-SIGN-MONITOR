/******************************************* HEADERS ***************************************************/

#include <AD8232_APIv2.h>
#include "adc.h"
#include "portpin.h"
//#include <ADC_API.h>
//#include <PIT_API.h>
//#include "fsl_gpio.h"


/******************************************* CONSTANTES ***************************************************/
//#define GPIO_LO_plus	GPIOC
//#define GPIO_LO_minus	GPIOC
//#define GPIO_SDN		GPIOB
//
//#define PIN_LO_plus		17U
//#define PIN_LO_minus	16U
//#define PIN_SDN
//
//#define MY_ADC			ADC_0
//#define MY_ADC_CHANNEL	12U		// El single ended

#define PTC17 PORTNUM2PIN(PORT_C, 17) //LO_plus
#define PTC16 PORTNUM2PIN(PORT_C, 16) //LO_minus

#define HIGH			1

/******************************************* ENUM ***************************************************/
//typedef enum {buffer_A, buffer_B} loading_buffer_t;		// Esto indica sobre que buffer se cargan las muestras

/**************************************** VARIABLES GLOBALES ************************************************/

bool done = false;
//bool buffA_active = false;
//loading_buffer_t buff_loading = buffer_A;
//uint16_t buffA[AD8232_BUFF_SIZE];
//uint16_t buffB[AD8232_BUFF_SIZE];
uint16_t cardiobuff[AD8232_BUFF_SIZE];
uint16_t samplesTaken;

/**************************************** FUNCIONES PRIVADAS ************************************************/
//void AD8232_Irq(void);


/************************************** DEFINICION DE FUNCIONES ************************************************/

void AD8232_init(void){
//	PITenable(PIT1);
//	//PITsetFreq(PIT1, AD8232_SAMPLING_FREC);
//	PITsetFreq(PIT1, AD8232_SAMPLING_FREC/2);
//	//adc_Init(MY_ADC, MY_ADC_CHANNEL);
//	adc_setTrigger(MY_ADC, PIT1, AD8232_Irq);

	ADCconfigure(ADC_0, ADC_16BITS);
	ADCstart(ADC_0, AD8232_SAMPLING_FREC/2, AD8232_BUFF_SIZE);

	PINconfigure(PTC17, PIN_MUX1, PIN_IRQ_DISABLE); //LO_plus
	PINmode(PTC17, PIN_INPUT);
	PINconfigure(PTC16, PIN_MUX1, PIN_IRQ_DISABLE); //LO_minus
	PINmode(PTC16, PIN_INPUT);
}

bool AD8232_checkState(void)
{
//	if((GPIO_PinRead(GPIO_LO_plus, PIN_LO_plus) == HIGH) || (GPIO_PinRead(GPIO_LO_minus, PIN_LO_minus) == HIGH))
//		return 0;
//	else
//		return 1;
	if((PINread(PTC17) == HIGH) || (PINread(PTC16) == HIGH))
		return 0;
	else
		return 1;
}

//bool AD8232_read(uint16_t * data)
//{
////	if(done)		//se checkea si las muestras estan listas o no
////	{
////		uint16_t *buff = (buff_loading == buffer_A)?  buffB : buffA;
////		uint16_t i;
////		for(i = 0; i < AD8232_BUFF_SIZE; i++)
////			data[i] = buff[i];
////
////		done = false;
////		return true;
////	}
////	else
////		return false;
//	uint8_t i = 0;
//	if(ADCdata2read(ADC_0)) {
//		getADCdata(ADC_0, buffA, AD8232_BUFF_SIZE);
//		for(i = 0; i < AD8232_BUFF_SIZE; i++)
//			data[i] = buffA[i];
//		return true;
//	}
//	else
//		return false;
//}

bool CARDIOdata2read(void) {
	return ADCdata2read(ADC_0);
}

void getCARDIOdata(uint16_t* data) {
	uint8_t i = 0;

	getADCdata(ADC_0, cardiobuff, AD8232_BUFF_SIZE);
	for(i = 0; i < AD8232_BUFF_SIZE; i++) {
		data[i] = cardiobuff[i];
	}
}


//void AD8232_Irq(void)
//{
//	if(samplesTaken >= AD8232_BUFF_SIZE)
//	{
//		done = true;
//		samplesTaken = 0;
//		buff_loading = (buff_loading == buffer_A)?  buffer_B : buffer_A;	//se conmuta el buffer sobre el cual se cargan las muestras
//	}
//	if(buff_loading == buffer_A)
//		buffA[samplesTaken] = adc_Read(ADC_0);
//	else if(buff_loading == buffer_B)
//		buffB[samplesTaken] = adc_Read(ADC_0);
//	samplesTaken++;
//}

