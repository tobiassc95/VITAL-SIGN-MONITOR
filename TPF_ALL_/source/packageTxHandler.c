/*
 * packageTxHandler.c
 *
 *  Created on: 29 may. 2022
 *      Author: guido
 */

#include "hc05.h"
#include "stdbool.h"
#include "packageTxHandler.h"

#define FLOAT_2_BYTE			(4)
#define UINT32_2_BYTE			(4)			//4 Bytes en un uint32
#define UINT16_2_BYTE			(2)

#define OPCODE_TEMP				(0x6A)
#define OPCODE_HEART_RATE		(0x7A)
#define OPCODE_SPO2				(0x7B)
#define OPCODE_IR				(0x7C)
#define OPCODE_RED				(0x7D)	//luz roja
#define OPCODE_ECG				(0x8A)	//electrocardigrama

//#define OPCODES_QTY				(6)		//cantidad de opcodes
//#define ARRAYS_QTY				(3)
#define SIZE_OPCODE				(1)		//size de un opcode en bytes
#define LENGTH_SIZE				(2)		//size del dato de length los arrays en bytes
#define MAX30102_BUFF_LENGTH	(100) //MAX30102_BUFF_SIZE
#define AD8232_BUFF_LENGTH		(250) //AD8232_BUFF_SIZE
//#define PACKAGE_SIZE	(OPCODES_QTY*SIZE_OPCODE + ARRAYS_QTY*LENGTH_SIZE + AD8232_BUFF_LENGTH + 2*MAX30102_BUFF_LENGTH)	//largo del paquete a mandar

//#define TEMP_QTY_DATA			(FLOAT_2_BYTE)		//cantidad de bytes de informacion de temperatura ( primero el entero y luego el decimal ? )
//#define OXIMETER_QTY_DATA		(MAX30102_BUFF_LENGTH * UINT32_2_BYTE + 1 + FLOAT_2_BYTE)		// buffers de RED y IR, datos de heartrate y spo2 (hr y spo2 1 byte?) //(?)
#define TEMP_QTY_DATA			(UINT16_2_BYTE)		//cantidad de bytes de informacion de temperatura ( primero el entero y luego el decimal ? )
#define OXIMETER_QTY_DATA		(MAX30102_BUFF_LENGTH * UINT16_2_BYTE + 1 + UINT16_2_BYTE)		// buffers de RED y IR, datos de heartrate y spo2 (hr y spo2 1 byte?) //(?)
#define ECG_QTY_DATA			(AD8232_BUFF_LENGTH * UINT16_2_BYTE)				//

#define SAFEGUARD				(10) 	//hacemos unos buffers un poquito mas grandes por las dudas de no pasarse de la memoria

///////////////////// PAQUETES /////////////////////////////
typedef struct {
	uint8_t tempPackage[2+SIZE_OPCODE + LENGTH_SIZE + TEMP_QTY_DATA ]; //7 BYTES  			 // opcode, largo y dato //(?)
	uint8_t OximeterPackage[2+(SIZE_OPCODE + LENGTH_SIZE)*3 + OXIMETER_QTY_DATA]; //212 BYTES + SAFEGUARD  // Este paquete va a tener 4 datos: spo2, HR, RED y IR //(?)
	uint8_t ECGPackage[2+SIZE_OPCODE + LENGTH_SIZE + ECG_QTY_DATA]; //505 BYTES + SAFEGUARD
}SENSORS_DATA_t;

///////////////////// VARIABLES /////////////////////////////
static SENSORS_DATA_t sensorsData;
static bool ECGready = false;
static bool OXIready = false;
static bool TEMPready = false;

///////////////////// FUNCIONES /////////////////////////////

void initPackage(void) {
	HC05init();
	createTempPackage();
	createOximeterPackage();
	createECGPackage();
}

void createTempPackage(void)
{
	sensorsData.tempPackage[0] = 0x5A;

	sensorsData.tempPackage[1] = OPCODE_TEMP;
	sensorsData.tempPackage[2] = 0;
	sensorsData.tempPackage[3] = TEMP_QTY_DATA;

	sensorsData.tempPackage[4+TEMP_QTY_DATA] = 0xA5;
}

void createOximeterPackage(void)
{
	//uint8_t data1, data2;
	sensorsData.OximeterPackage[0] = 0x5A;

	sensorsData.OximeterPackage[1] = OPCODE_HEART_RATE;
	sensorsData.OximeterPackage[2] = 0; //1st byte size
	sensorsData.OximeterPackage[3] = 1;	//2nd byte size					// un byte para el heartrate

	sensorsData.OximeterPackage[5] = OPCODE_SPO2;
	sensorsData.OximeterPackage[6] = 0; //1st byte size
	sensorsData.OximeterPackage[7] = UINT16_2_BYTE; //FLOAT_2_BYTE; //2nd byte size

//	sensorsData.OximeterPackage[11] = OPCODE_RED; //or IR
	//data2 = (MAX30102_BUFF_LENGTH * UINT32_2_BYTE) >> 8;
	//data1 = (MAX30102_BUFF_LENGTH * UINT32_2_BYTE) & 0x00FF;
	sensorsData.OximeterPackage[10] = OPCODE_RED; //or IR
	sensorsData.OximeterPackage[11] = (MAX30102_BUFF_LENGTH * /* UINT32_2_BYTE */ UINT16_2_BYTE) >> 8; //1st byte size
	sensorsData.OximeterPackage[12] = (MAX30102_BUFF_LENGTH * /* UINT32_2_BYTE */ UINT16_2_BYTE) & 0x00FF; //2nd byte size

	sensorsData.OximeterPackage[13+MAX30102_BUFF_LENGTH*UINT16_2_BYTE] = 0xA5;

//	sensorsData.OximeterPackage[14+MAX30102_BUFF_LENGTH * UINT32_2_BYTE] = OPCODE_IR;
//	sensorsData.OximeterPackage[15+MAX30102_BUFF_LENGTH * UINT32_2_BYTE] = MAX30102_BUFF_LENGTH * UINT32_2_BYTE; //400>255 (?)
}

void createECGPackage(void)
{
	sensorsData.ECGPackage[0] = 0x5A;

	sensorsData.ECGPackage[1] = OPCODE_ECG;
	sensorsData.ECGPackage[2] = (ECG_QTY_DATA) >> 8;;
	sensorsData.ECGPackage[3] = (ECG_QTY_DATA) & 0x00FF;

	sensorsData.ECGPackage[4+ECG_QTY_DATA] = 0xA5;
}

void setTempData(float temp)
{
	TEMPready = true;
//	memcpy(sensorsData.tempPackage + 3, &temp, sizeof(temp)); //(?)

	uint16_t t = temp*100;
	sensorsData.tempPackage[4] = t/100;
	sensorsData.tempPackage[5] = t%100;

//	uint8_t t = (uint8_t)temp;
//	uint8_t decimal = (temp - t)*100;
//	sensorsData.tempPackage[2] = t;
//	sensorsData.tempPackage[3] = decimal;
}

void setOximeterData(uint32_t* red, uint8_t hr, float spo2)
{
	OXIready = true;
	sensorsData.OximeterPackage[4] = hr;
//	memcpy(sensorsData.OximeterPackage + 7, &spo2, sizeof(spo2)); //(?)
//	memcpy(sensorsData.OximeterPackage + 14, red, MAX30102_BUFF_LENGTH * /* UINT32_2_BYTE */ UINT16_2_BYTE);

	uint16_t temp[MAX30102_BUFF_LENGTH];
	int i = 0;
	for (i = 0; i < MAX30102_BUFF_LENGTH; i++ ) {
		temp[i] = (red[i]-120000-210)*0.011417;
	}
	uint16_t t = spo2*100;
	sensorsData.OximeterPackage[8] = t/100;
	sensorsData.OximeterPackage[9] = t%100;


	memcpy(sensorsData.OximeterPackage + 13, temp, MAX30102_BUFF_LENGTH * /* UINT32_2_BYTE */ UINT16_2_BYTE);
}

void setECGData(uint16_t* ecg_buff)
{
	ECGready = true;
	memcpy(sensorsData.ECGPackage + 4, ecg_buff, ECG_QTY_DATA);
}

void sendTempPackage(void)
{
	HC05trasmit(sensorsData.tempPackage, sizeof(sensorsData.tempPackage));
}

void sendOximeterPackage(void)
{
	HC05trasmit(sensorsData.OximeterPackage, sizeof(sensorsData.OximeterPackage));
}

void sendECGPackage(void)
{
	HC05trasmit(sensorsData.ECGPackage, sizeof(sensorsData.ECGPackage));
}

void sendFullPackage(void)
{
	if (ECGready && OXIready && TEMPready) {
		ECGready = false;
		OXIready = false;
		TEMPready = false;
		HC05trasmit(&sensorsData, sizeof(sensorsData));
	}
}
