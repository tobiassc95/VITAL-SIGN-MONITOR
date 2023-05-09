/*************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "audio.h"
#include "AD8232_APIv2.h"
#include "MAX30102_API.h"
/*#include "TempSensor.h"*/
#include "algorithm_by_RF.h"
#include "packageTxHandler.h"

#include "hc05.h"

#define HRMAX 1		//Acá va el HR máximo
#define HRMIN 1		//Acá va el HR mínimo
#define SPO2MAX 1	//Acá va el SPO2 máximo
#define SPO2MIN 1	//Acá va el SPO2 mínimo
#define TEMPMAX 1 	//Acá va el TEMP máximo
#define TEMPMIN 1 	//Acá va el TEMP mínimo

//#define OK	1
//#define MAX30102_BUFF_LENGTH MAX30102_BUFF_SIZE
//#define AD8232_BUFF_LENGTH AD8232_BUFF_SIZE

//static int count = 0;
//static bool MAX30102_OK = false;
static uint32_t OXIir[MAX30102_BUFF_SIZE];				// buffer para las muestras ir
static uint32_t OXIred[MAX30102_BUFF_SIZE];				// buffer para las muestras red
static uint16_t CARDIOecg[AD8232_BUFF_SIZE];
static float OXIspo2; 										// valor de spo2 (saturación de oxigeno en la sangre)
static int32_t OXIhr;									// valor de heart rate
static float TEMPtemp; //(?)float

static char* HC05msg = "Hello World! 0123456789"; //TODO: delete
static char nombrePaciente[] = "Guido";

/*
 * @brief   App de prueba para los dispositivos
 */
//typedef enum {MAX30205, MAX30102, AD8232} disp_t;	//MAX30205 - temperatura  |  MAX30102 - Oximetria  |  AD8232 - freq cardiaca

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

//static void delayLoop(uint32_t veces);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void) {
	uint8_t dummy;

	//MAX30102
	if(MAX30102_init() != MAX30102_ERROR) {
		//MAX30102_OK = true;
		dummy = getIntr1(&dummy); //para inicializar el CI (?)
	}

	//MAX30205
	/*tempSensor_setSlaveAdr(MAX30205_ADDRESS_DEFAULT);
	MAX30205_writeConfig(defaul_MAX30205_config);*/

	//AD8232
	AD8232_init();

	//CREACION DE PAQUETES
	initPackage();

	//HC05
	HC05init(); //Se inicializa por segunda vez la UART (!)

	//AUDIO
	audioInit();
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void) {
	//////////////////////////////////////////////////////////////////
	//				VARIABLES
	//////////////////////////////////////////////////////////////////
	//MAX30102
	int8_t isValidHr = 0;								// flag validez hr
	int8_t isValidSpo2 = 0; 							// flag validez spo2
	float redmedia = 0; 								// valor medio de las muestras red (?)_
	float irmedia = 0; 									// valor medio de las muestras ir (?)_
	float ratio; 										// ratio of ratios (?)_
	float cor;											// Pearson correlation between red and IR (?)_

	//HC05
	static uint8_t buff[35];
	static bool connected = false;

	//AUDIO
	static bool wrngs[3] = {true, true, true}; //{false, false, false};
	static uint8_t wrngmsg = AUDIO_NONE;
	uint8_t i;

	//CONNECTING TO APP
	if (!connected) {
		while (!(buff[0] == 0x5A && buff[1] == 0x88)) { //wait for ID opcode.
			HC05receive(buff, 5);
		}
		buff[0] = 0x5A; buff[1] = 0x81; buff[2] = 0x00; buff[3] = 0x00; buff[4] = 0xA5;
		HC05trasmit(buff, 5);
		while(!(buff[0] == 0x5A && buff[1] == 0xC1)) { //wait for configuration opcode.
			HC05receive(buff, 5);
		}
		buff[0] = 0x5A;
		buff[1] = 0xC8; buff[2] = 0x00; buff[3] = sizeof(nombrePaciente)-1 + 11;
		buff[4] = 0xD6; buff[5] = 0x00; buff[6] = 0x01; buff[7] = AD8232_BUFF_SIZE;
		buff[8] = 0xD7; buff[9] = 0x00; buff[10] = 0x01; buff[11] = MAX30102_BUFF_SIZE;
		buff[12] = 0xC9; buff[13] = 0x00; buff[14] = sizeof(nombrePaciente)-1; memcpy(buff + 15, nombrePaciente, sizeof(nombrePaciente)-1);
		buff[15 + sizeof(nombrePaciente)-1] = 0xA5;
		HC05trasmit(buff, 15 + sizeof(nombrePaciente));


		while (!(buff[0] == 0x5A && buff[1] == 0x88)) { //wait for ID opcode.
			HC05receive(buff, 5);
		}
		buff[0] = 0x5A; buff[1] = 0x81; buff[2] = 0x00; buff[3] = 0x00; buff[4] = 0xA5;
		HC05trasmit(buff, 5);
		while(!(buff[0] == 0x5A && buff[1] == 0xC1)) { //wait for configuration opcode.
			HC05receive(buff, 5);
		}
		buff[0] = 0x5A;
		buff[1] = 0xC8; buff[2] = 0x00; buff[3] = sizeof(nombrePaciente)-1 + 11;
		buff[4] = 0xD6; buff[5] = 0x00; buff[6] = 0x01; buff[7] = AD8232_BUFF_SIZE;
		buff[8] = 0xD7; buff[9] = 0x00; buff[10] = 0x01; buff[11] = MAX30102_BUFF_SIZE;
		buff[12] = 0xC9; buff[13] = 0x00; buff[14] = sizeof(nombrePaciente)-1; memcpy(buff + 15, nombrePaciente, sizeof(nombrePaciente)-1);
		buff[15 + sizeof(nombrePaciente)-1] = 0xA5;
		HC05trasmit(buff, 15 + sizeof(nombrePaciente));

		while(buff[1] != 0x91) { //wait for ready2receive opcode.
			HC05receive(buff, 5);
		}

		connected = true;
	}
	while(1);


	/////////////////////////////////////////////////////////////////

	//MAX30102
	if (OXIdata2read()) {
		getOXIdata(OXIred, OXIir);
		rf_heart_rate_and_oxygen_saturation(OXIred, MAX30102_BUFF_SIZE, OXIir, &OXIspo2, &isValidSpo2, &OXIhr, &isValidHr, &ratio, &cor, &irmedia, &redmedia);
//		maxim_heart_rate_and_oxygen_saturation(buffRED, MAX30102_BUFF_LENGTH, buffIR,  &spo2, &isValidSpo2, &heart_rate, &isValidHr);
		if(isValidHr == false) { //(?)
//			PRINTF("Error en la obtencion del ritmo cardiaco \n");
//			log(0);
//			delayLoop(UINT16_MAX*200);
		}
		else if(isValidSpo2 == false) { //(?)
//			PRINTF("Error en la obtencion de la oxigenacion de la sangre \n");
		}
		else {
//			PRINTF("saturacion de oxigeno en la sangre: %d  | ritmo cardiaco: %d [BPM]\n", (int)spo2, heart_rate);	//medicion exitosa
			/*setOximeterData(OXIred, OXIir, (uint8_t)OXIhr, (uint8_t)OXIspo2); //(?)*/
			setOximeterData(OXIred, (uint8_t)OXIhr, OXIspo2);
			sendOximeterPackage();
//			log(heart_rate);
//			delayLoop(UINT16_MAX*200);
		}
		TEMPtemp = MAX30102_readTemperature();
		//MAX30205
		/*if(MAX30205_transferReady()) {
			MAX30205_requestTemp();
//			temp = (uint32_t)MAX30205_get_temp(CELSIUS);
			setTempDataf(MAX30205_get_temp(CELSIUS));
//			log(temp);
			sendTempPackage();
			//delayLoop(UINT16_MAX*1000);
		}*/
		setTempData(TEMPtemp);
		sendTempPackage();
//		else {
//			//log(0);
//		}
	}

	//AD8232
	if(CARDIOdata2read()) {
		getCARDIOdata(CARDIOecg);
//		logArray_u16(ecg2send, AD8232_BUFF_SIZE);
		setECGData(CARDIOecg);
		sendECGPackage();
//		delayLoop(UINT16_MAX*2000);
	}

	//sendFullPackage();

	//AUDIO
	/* //SENSOR MEASUREMENTS
	 * cardioMeas = getCardio();
	 * oxiMeas = getOxi();
	 * tempMeas = getTemp();
	 *
	 * //BLUETOOTH
	 * HC05trasmit(cardioMeas, sizeof(cardioMeas));
	 * HC05trasmit(oxiMeas, sizeof(oxiMeas));
	 * HC05trasmit(tempMeas, sizeof(tempMeas));
	 *
	 * //OUT OF RANGE
	 * if(cardioMeas > cardioMAX || cardioMeas < cardioMIN)
	 * 		wrngs[0] = true;
	 * if(oxiMeas > oxiMAX || oxiMeas < oxiMIN)
	 * 		wrngs[1] = true;
	 * if(tempMeas > tempMAX || tempMeas < tempMIN)
	 * 		wrngs[2] = true;
	 * */
	if(OXIhr > HRMAX || OXIhr < HRMIN)
		wrngs[0] = true;
	else
		wrngs[0] = false;
	if(OXIspo2 > SPO2MAX || OXIspo2 < SPO2MIN)
		wrngs[1] = true;
	else
		wrngs[1] = false;
	if(TEMPtemp > TEMPMAX || TEMPtemp < TEMPMIN)
		wrngs[2] = true;
	else
		wrngs[2] = false;

	if (!audioPlaying()) {
		//HC05trasmit(HC05msg, 23); //TODO: delete
		for (i=wrngmsg+1; i!=wrngmsg; i++) {
			if (i == 4)
				i = 0;
			if (i != AUDIO_NONE)
				if (wrngs[i-1]) {
					wrngmsg = i;
					break;
				}
				else
					wrngmsg = AUDIO_NONE;
			else
				wrngmsg = AUDIO_NONE;
		}
	}
	audioPlay(wrngmsg);
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

//static void delayLoop(uint32_t veces)
//{
//    while (veces--);
//}

/*******************************************************************************
 ******************************************************************************/
