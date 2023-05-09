//#include <i2c_API.h>
#include "i2c.h"
#include "MAX30102_API.h"
#include "portpin.h"
#include "PIT_driver.h"
//#include "fsl_debug_console.h"

#define PTB9 PORTNUM2PIN(PORT_B, 9)

// Dado que es muy comun en esta funciónes escribir o leer 1 byte a un registro se hacen funciones especificas para eso
// se devuelve 1 si no hubo error, 0 si hubo error
bool writeReg(MAX30102_Registers_t reg, uint8_t value);
bool readReg(MAX30102_Registers_t reg, uint8_t *value);

void MAX30102read(void);


/************************************ ESCRITURA Y LECTURA DE REGISTROS Y ADDRESS DEL SLAVE *******************************************
 *************************************************************************************************************************************
 *************************************************************************************************************************************/

    // Configuration Register Cache
char _interruptEnable1;
char _interruptEnable2;
char _fifoConfiguration;
char _modeConfiguration;
char _spo2Configuration;
char _led1PulseAmplitude;
char _led2PulseAmplitude;
char _led3PulseAmplitude;
char _pilotPulseAmplitude;
char _multiLedControl1;
char _multiLedControl2;
char _proxIntThreshold;

uint8_t _ambientLigth[1];
uint8_t OXIi2cmsg[4]; //to I2C.

static uint32_t buffIR[MAX30102_BUFF_SIZE];				// buffer para las muestras ir
static uint32_t buffRED[MAX30102_BUFF_SIZE];				// buffer para las muestras red
static bool OXIdataRdy = false; //flag para mandar el  valor

bool writeReg(MAX30102_Registers_t reg, uint8_t value)
{
	uint8_t volatile dataTransmit;
	dataTransmit = value;
	//i2c_writeReg(MAX30102_I2C_ADDR, reg, &dataTransmit, 1);
	//while(i2c_status() != i2c_IDLE);
	OXIi2cmsg[0] = reg; OXIi2cmsg[1] = dataTransmit;
	writeI2Cmessage(I2C_0, OXIi2cmsg, 2);
	blockingI2Ctransmition(I2C_0, MAX30102_I2C_ADDR, 2, 0);

	return true;
}

bool readReg(MAX30102_Registers_t reg, uint8_t *value)
{
	uint8_t volatile dataRecieve;
	//i2c_readReg(MAX30102_I2C_ADDR, reg, &dataRecieve, 1);
	//while(i2c_status() != i2c_IDLE);
	OXIi2cmsg[0] = reg;
	writeI2Cmessage(I2C_0, OXIi2cmsg, 1);
	blockingI2Ctransmition(I2C_0, MAX30102_I2C_ADDR, 1, 1);
	readI2Cmessage(I2C_0, &dataRecieve, 1);

	*value=(uint8_t) dataRecieve;
	return true;
}

int setSingleLED(smp_ave_t smpAve, bool fifoRollOver, fifo_a_full_t fifoAFull, adc_rge_t adcRange, smp_rt_t smpRate, led_pw_t ledPW, char led1PA)
{
//	bool status_1;
//	bool status_2;
	uint8_t dataBuf[3];
	_fifoConfiguration = (smpAve & MASK_SMP_AVE) | (fifoAFull & MASK_FIFO_A_FULL);
	if(fifoRollOver) _fifoConfiguration |= MASK_FIFO_ROLLOVER_EN;
	_modeConfiguration = (_modeConfiguration & MASK_SHDN) | MODE_1LED;
	_spo2Configuration = (adcRange & MASK_ADC_RGE) | (smpRate & MASK_SMP_RT) | (ledPW & MASK_LED_PW);
	_led1PulseAmplitude = led1PA;
	//dataBuf[0] = REG_LED1_PA;
	//dataBuf[1] = _led1PulseAmplitude;
    //writeI2Cmessage(dataBuf, 2);
    //I2Ctransmition(MAX30102_I2C_ADDR, 2, 0);
	writeReg(REG_LED1_PA, _led1PulseAmplitude);
	//status_1 = I2C_WriteReg(I2C0, MAX30102_I2C_ADDR, REG_LED1_PA, _led1PulseAmplitude);
	//dataBuf[0] = REG_FIFO_CONFIG;
	//dataBuf[1] = _fifoConfiguration;
	//dataBuf[2] = _modeConfiguration;
	//dataBuf[3] = _spo2Configuration;
    //writeI2Cmessage(dataBuf, 4);
    //I2Ctransmition(MAX30102_I2C_ADDR, 4, 0);
	dataBuf[0] = _fifoConfiguration;
	dataBuf[1] = _modeConfiguration;
	dataBuf[2] = _spo2Configuration;
    //i2c_writeReg(MAX30102_I2C_ADDR, REG_FIFO_CONFIG, dataBuf, 3);
    //while(i2c_status()!=i2c_IDLE);		// Esto lo hace bloquenate
	OXIi2cmsg[0] = REG_FIFO_CONFIG; OXIi2cmsg[1] = dataBuf[0]; OXIi2cmsg[2] = dataBuf[1]; OXIi2cmsg[3] = dataBuf[2];
	writeI2Cmessage(I2C_0, OXIi2cmsg, 4);
	blockingI2Ctransmition(I2C_0, MAX30102_I2C_ADDR, 4, 0);

//	status_2 = I2C_WriteRegs(I2C0, MAX30102_I2C_ADDR, REG_FIFO_CONFIG, dataBuf, 3);
//
//	if(status_1 == false || status_2 == false)
//	{
//		PRINTF("No se ha podido comunicar con el MAX30102 | setSingleLED()");
//		return MAX30102_ERROR;
//	}
//	else
//		return MAX30102_NO_ERROR;

    return true;
}

int setSingleLED_defaultMode()
{
	return setSingleLED(SMP_AVE_1, false, FIFO_A_FULL_F, ADC_RGE_01, SMP_RT_100, LED_PW_18BIT, MAX30102_DEFAULT_LED_PA);
}

int setDualLED(smp_ave_t smpAve, bool fifoRollOver, fifo_a_full_t fifoAFull, adc_rge_t adcRange, smp_rt_t smpRate, led_pw_t ledPW, char led1PA, char led2PA)
{
//	bool status_1;
//	bool status_2;
	uint8_t dataBuf[3];
	_fifoConfiguration = (smpAve & MASK_SMP_AVE) | (fifoAFull & MASK_FIFO_A_FULL);
	if(fifoRollOver) _fifoConfiguration |= MASK_FIFO_ROLLOVER_EN;
	_modeConfiguration = (_modeConfiguration & MASK_SHDN) | MODE_2LED;
	_spo2Configuration = (adcRange & MASK_ADC_RGE) | (smpRate & MASK_SMP_RT) | (ledPW & MASK_LED_PW);
	_led1PulseAmplitude = led1PA;
	_led2PulseAmplitude = led2PA;
	//dataBuf[0] = REG_LED1_PA;
	//dataBuf[1] = _led1PulseAmplitude;
	//dataBuf[2] = _led2PulseAmplitude;
	//writeI2Cmessage(dataBuf, 3);
	//I2Ctransmition(MAX30102_I2C_ADDR, 3, 0);
	dataBuf[0] = _led1PulseAmplitude;
	dataBuf[1] = _led2PulseAmplitude;
	//i2c_writeReg(MAX30102_I2C_ADDR, REG_LED1_PA, dataBuf, 2);
    //while(i2c_status()!=i2c_IDLE);		// Esto lo hace bloquenate
    OXIi2cmsg[0] = REG_LED1_PA; OXIi2cmsg[1] = dataBuf[0]; OXIi2cmsg[2] = dataBuf[1];
	writeI2Cmessage(I2C_0, OXIi2cmsg, 3);
	blockingI2Ctransmition(I2C_0, MAX30102_I2C_ADDR, 3, 0);

	//status_1 = I2C_WriteRegs(I2C0, MAX30102_I2C_ADDR, REG_LED1_PA, dataBuf, 2);
	//dataBuf[0] = REG_FIFO_CONFIG;
    //dataBuf[1] = _fifoConfiguration;
    //dataBuf[2] = _modeConfiguration;
    //dataBuf[3] = _spo2Configuration;
	//writeI2Cmessage(dataBuf, 4);
	//I2Ctransmition(MAX30102_I2C_ADDR, 4, 0);
	dataBuf[0] = _fifoConfiguration;
	dataBuf[1] = _modeConfiguration;
	dataBuf[2] = _spo2Configuration;
	//i2c_writeReg(MAX30102_I2C_ADDR, REG_FIFO_CONFIG, dataBuf, 3);
    //while(i2c_status()!=i2c_IDLE);		// Esto lo hace bloquenate
    OXIi2cmsg[0] = REG_FIFO_CONFIG; OXIi2cmsg[1] = dataBuf[0]; OXIi2cmsg[2] = dataBuf[1]; OXIi2cmsg[3] = dataBuf[2];
	writeI2Cmessage(I2C_0, OXIi2cmsg, 4);
	blockingI2Ctransmition(I2C_0, MAX30102_I2C_ADDR, 4, 0);

//	status_2 = I2C_WriteRegs(I2C0, MAX30102_I2C_ADDR, REG_FIFO_CONFIG, dataBuf, 3);
//
//    if(status_1 == false || status_2 == false)
//    {
//    	PRINTF("No se ha podido comunicar con el MAX30102 | setDualLED()");
//    	return MAX30102_ERROR;
//    }
//    else
//    	return MAX30102_NO_ERROR;
    return true;
}

int setDualLED_defaultMode()
{
	return setDualLED(SMP_AVE_1, false, FIFO_A_FULL_F, ADC_RGE_01, SMP_RT_100, LED_PW_18BIT, MAX30102_DEFAULT_LED_PA, MAX30102_DEFAULT_LED_PA);
}

int setMultiLED(smp_ave_t smpAve, bool fifoRollOver, fifo_a_full_t fifoAFull, adc_rge_t adcRange, smp_rt_t smpRate, led_pw_t ledPW, char led1PA, char led2PA, char led3PA, char pilotPA, slot_t slot1, slot_t slot2, slot_t slot3, slot_t slot4)
{
//	bool status_1;
//	bool status_2;
//	bool status_3;
	uint8_t dataBuf[3];
	_fifoConfiguration = (smpAve & MASK_SMP_AVE) | (fifoAFull & MASK_FIFO_A_FULL);
	if(fifoRollOver) _fifoConfiguration |= MASK_FIFO_ROLLOVER_EN;
	_modeConfiguration = (_modeConfiguration & MASK_SHDN) | MODE_MULTI;
	_spo2Configuration = (adcRange & MASK_ADC_RGE) | (smpRate & MASK_SMP_RT) | (ledPW & MASK_LED_PW);
	_led1PulseAmplitude = led1PA;
	_led2PulseAmplitude = led2PA;
	_led3PulseAmplitude = led3PA;
	_pilotPulseAmplitude = pilotPA;
	_multiLedControl1 = (slot2 << 4) | slot1;
	_multiLedControl2 = (slot4 << 4) | slot3;
	//dataBuf[0] = REG_PILOT_PA;
	//dataBuf[1] = _pilotPulseAmplitude;
	//dataBuf[2] = _multiLedControl1;
	//dataBuf[3] = _multiLedControl2;
	//writeI2Cmessage(dataBuf, 4);
	//I2Ctransmition(MAX30102_I2C_ADDR, 4, 0);
	dataBuf[0] = _pilotPulseAmplitude;
	dataBuf[1] = _multiLedControl1;
	dataBuf[2] = _multiLedControl2;
	//i2c_writeReg(MAX30102_I2C_ADDR, REG_PILOT_PA, dataBuf, 3);
    //while(i2c_status()!=i2c_IDLE);		// Esto lo hace bloquenate
    OXIi2cmsg[0] = REG_PILOT_PA; OXIi2cmsg[1] = dataBuf[0]; OXIi2cmsg[2] = dataBuf[1]; OXIi2cmsg[3] = dataBuf[2];
	writeI2Cmessage(I2C_0, OXIi2cmsg, 4);
	blockingI2Ctransmition(I2C_0, MAX30102_I2C_ADDR, 4, 0);

//	status_1 = I2C_WriteRegs(I2C0, MAX30102_I2C_ADDR, REG_PILOT_PA, dataBuf, 3);
    //dataBuf[0] = REG_LED1_PA;
    //dataBuf[1] = _led1PulseAmplitude;
    //dataBuf[2] = _led2PulseAmplitude;
    //dataBuf[3] = _led3PulseAmplitude;
	//writeI2Cmessage(dataBuf, 4);
	//I2Ctransmition(MAX30102_I2C_ADDR, 4, 0);
	dataBuf[0] = _led1PulseAmplitude;
	dataBuf[1] = _led2PulseAmplitude;
	dataBuf[2] = _led3PulseAmplitude;
	//i2c_writeReg(MAX30102_I2C_ADDR, REG_LED1_PA, dataBuf, 3);
    //while(i2c_status()!=i2c_IDLE);		// Esto lo hace bloquenate
    OXIi2cmsg[0] = REG_LED1_PA; OXIi2cmsg[1] = dataBuf[0]; OXIi2cmsg[2] = dataBuf[1]; OXIi2cmsg[3] = dataBuf[2];
	writeI2Cmessage(I2C_0, OXIi2cmsg, 4);
	blockingI2Ctransmition(I2C_0, MAX30102_I2C_ADDR, 4, 0);

//	status_2 = I2C_WriteRegs(I2C0, MAX30102_I2C_ADDR, REG_LED1_PA, dataBuf, 3);
    //dataBuf[0] = REG_FIFO_CONFIG;
    //dataBuf[1] = _fifoConfiguration;
    //dataBuf[2] = _modeConfiguration;
    //dataBuf[3] = _spo2Configuration;
    //writeI2Cmessage(dataBuf, 4);
    //I2Ctransmition(MAX30102_I2C_ADDR, 4, 0);
	dataBuf[0] = _fifoConfiguration;
	dataBuf[1] = _modeConfiguration;
	dataBuf[2] = _spo2Configuration;
	//i2c_writeReg(MAX30102_I2C_ADDR, REG_FIFO_CONFIG, dataBuf, 3);
    //while(i2c_status()!=i2c_IDLE);		// Esto lo hace bloquenate
    OXIi2cmsg[0] = REG_FIFO_CONFIG; OXIi2cmsg[1] = dataBuf[0]; OXIi2cmsg[2] = dataBuf[1]; OXIi2cmsg[3] = dataBuf[2];
	writeI2Cmessage(I2C_0, OXIi2cmsg, 4);
	blockingI2Ctransmition(I2C_0, MAX30102_I2C_ADDR, 4, 0);

//	status_3 = I2C_WriteRegs(I2C0, MAX30102_I2C_ADDR, REG_FIFO_CONFIG, dataBuf, 3);
//
//    if(status_1 == false || status_2 == false || status_3 == false)
//    {
//        PRINTF("No se ha podido comunicar con el MAX30102 | setMultiLED()");
//        return MAX30102_ERROR;
//    }
//    else
//        return MAX30102_NO_ERROR;

    return true;
}

int setMultiLED_defaultMode()
{
	return setMultiLED(SMP_AVE_1, false, FIFO_A_FULL_F, ADC_RGE_01, SMP_RT_100, LED_PW_18BIT, MAX30102_DEFAULT_LED_PA, MAX30102_DEFAULT_LED_PA, MAX30102_DEFAULT_LED_PA, MAX30102_DEFAULT_PILOT_PA, SLOT_RED_LED1, SLOT_IR_LED2, SLOT_GREEN_LED3, SLOT_DISABLED);
}

int MAX30102_init()
{
	I2Cconfiguration(I2C_0); //Necesitamos el I2C para leer los datos.

	softReset();

    if(writeReg(REG_INTR_ENABLE_1,0xc0) != true) // 0xe0 para habilitar la interrupciones por el efecto de la luz ambiente
    	return MAX30102_ERROR;
    if(writeReg(REG_INTR_ENABLE_2,0x00) != true)
    	return MAX30102_ERROR;
	if(writeReg(REG_FIFO_WR_PTR,0x00) != true)  //FIFO_WR_PTR[4:0]
		return MAX30102_ERROR;
	if(writeReg(REG_OVF_COUNTER,0x00) != true)//OVF_COUNTER[4:0]
		return MAX30102_ERROR;
	if(writeReg(REG_FIFO_RD_PTR,0x00) != true)  //FIFO_RD_PTR[4:0]
		return MAX30102_ERROR;
	if(writeReg(REG_FIFO_CONFIG,0x4f) != true) //sample avg = 4, fifo rollover=false, fifo almost full = 17
		return MAX30102_ERROR;
	if(writeReg(REG_MODE_CONFIG,0x03) != true)   //0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
		return MAX30102_ERROR;
	if(writeReg(REG_SPO2_CONFIG,0x27) != true)  // SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (400uS)
		return MAX30102_ERROR;
	if(writeReg(REG_LED1_PA,0x24) != true)   //Choose value for ~ 7mA for LED1
		return MAX30102_ERROR;
	if(writeReg(REG_LED2_PA,0x24) != true)   // Choose value for ~ 7mA for LED2
		return MAX30102_ERROR;
	if(writeReg(REG_PILOT_PA,0x7f) != true)   // Choose value for ~ 25mA for Pilot LED
		return MAX30102_ERROR;

	PINconfigure(PTB9, PIN_MUX1, PIN_IRQ_DISABLE);
	PINmode(PTB9, PIN_INPUT);

	PIT_init();
	PIT_setRutine(PIT1, MAX30102read);
	PIT_start(PIT1, 1000000000/20); //T = 50ms

    return MAX30102_NO_ERROR;
}

int readFIFO(uint32_t *redLED, uint32_t *irLED)
{
    uint32_t un_temp;
//    char uch_temp;
    *redLED=0;
    *irLED=0;
    uint8_t ach_i2c_data[6];

    //read and clear status register
    //readReg(REG_INTR_STATUS_1, &uch_temp);
    //readReg(REG_INTR_STATUS_2, &uch_temp);

    //ach_i2c_data[0]=REG_FIFO_DATA;
    //writeI2Cmessage(ach_i2c_data, 1);
    //I2Ctransmition(MAX30102_I2C_ADDR, 1, 6);
    //readI2Cmessage(ach_i2c_data, 6);
    //bool ok;
    //ok = I2C_ReadRegs(I2C0, MAX30102_I2C_ADDR, REG_FIFO_DATA, ach_i2c_data, 6);
    //i2c_readReg(MAX30102_I2C_ADDR, REG_FIFO_DATA, ach_i2c_data, 6);
    //while(i2c_status()!=i2c_IDLE);		// Esto lo hace bloquenate
    OXIi2cmsg[0] = REG_FIFO_DATA;
	writeI2Cmessage(I2C_0, OXIi2cmsg, 1);
	blockingI2Ctransmition(I2C_0, MAX30102_I2C_ADDR, 1, 6);
	readI2Cmessage(I2C_0, ach_i2c_data, 6);

    un_temp=(unsigned char) ach_i2c_data[0];
    un_temp<<=16;
    *redLED+=un_temp;
    un_temp=(unsigned char) ach_i2c_data[1];
    un_temp<<=8;
    *redLED+=un_temp;
    un_temp=(unsigned char) ach_i2c_data[2];
    *redLED+=un_temp;

    un_temp=(unsigned char) ach_i2c_data[3];
    un_temp<<=16;
    *irLED+=un_temp;
    un_temp=(unsigned char) ach_i2c_data[4];
    un_temp<<=8;
    *irLED+=un_temp;
    un_temp=(unsigned char) ach_i2c_data[5];
    *irLED+=un_temp;
    *redLED&=0x03FFFF;  //Mask MSB [23:18]
    *irLED&=0x03FFFF;  //Mask MSB [23:18]

//    if(ok)
//    	return MAX30102_NO_ERROR;
//    else
//    	return MAX30102_ERROR;
    return true;
}

int getIntr1(uint8_t *intStatus)
{
    readReg(REG_INTR_STATUS_1, intStatus);
    return (int)*intStatus;
}

int getIntr2(uint8_t *intStatus)
{
    readReg(REG_INTR_STATUS_2, intStatus);
    return (int)*intStatus;
}

int enableIntr(char intrBits)
{
    char intr1 = intrBits & (INTR_A_FULL|INTR_DATA_RDY|INTR_ALC_OVF|INTR_PROX);
    char intr2 = intrBits & INTR_TEMP_RDY;
    _interruptEnable1 |= intr1;
    if(writeReg(REG_INTR_ENABLE_1, _interruptEnable1) != true)
    	return MAX30102_ERROR;
    _interruptEnable2 |= intr2;
    if(writeReg(REG_INTR_ENABLE_2, _interruptEnable2) != true)
    	return MAX30102_ERROR;
    return MAX30102_NO_ERROR;
}

int disableIntr(char intrBits)
{
    char intr1 = intrBits & (INTR_A_FULL|INTR_DATA_RDY|INTR_ALC_OVF|INTR_PROX);
    char intr2 = intrBits & INTR_TEMP_RDY;
    _interruptEnable1 &= ~intr1;
    if(writeReg(REG_INTR_ENABLE_1, _interruptEnable1) != true)
    	return MAX30102_ERROR;
    _interruptEnable2 &= ~intr2;
    if(writeReg(REG_INTR_ENABLE_2, _interruptEnable2) != true)
    	return MAX30102_ERROR;
    return MAX30102_NO_ERROR;
}

int setProx(char proxAmp, char proxThresh)
{
	bool status_1, status_2;

	status_1 = writeReg(REG_PILOT_PA, proxAmp);
	status_2 = writeReg(REG_PROX_INTR_THRESH, proxThresh);

	if(status_1 == false || status_2 == false)
		return MAX30102_ERROR;
	return MAX30102_NO_ERROR;
}

float MAX30102_readTemperature()
{
	uint8_t dataBuf[2];
    char * dataSigned = (char *)dataBuf;

    writeReg(REG_TEMP_CONFIG, 0x01);
    //writeReg(REG_TEMP_CONFIG, 0x01);		// Pone en Enable la temperatura en su registro de config
    //delay_loop();
    //dataBuf[0] = REG_TEMP_INT;
    //writeI2Cmessage(dataBuf, 1);
    //I2Ctransmition(MAX30102_I2C_ADDR, 1, 2);
    //readI2Cmessage(dataBuf, 2);
//    I2C_ReadRegs(I2C0, MAX30102_I2C_ADDR, REG_TEMP_INT, dataBuf, 2);
    //i2c_readReg(MAX30102_I2C_ADDR, REG_TEMP_INT, dataBuf, 2);
    //while(i2c_status() != i2c_IDLE);		// Esto lo hace bloquenate
    OXIi2cmsg[0] = REG_TEMP_INT;
	writeI2Cmessage(I2C_0, OXIi2cmsg, 1);
	blockingI2Ctransmition(I2C_0, MAX30102_I2C_ADDR, 1, 2);
	readI2Cmessage(I2C_0, dataBuf, 2);

    return ((float)dataSigned[0] + ((float)dataSigned[1] * 0.0625));

/*
    char dataBuf[2];
    int8_t * dataSigned = reinterpret_cast<int8_t *>(dataBuf);

    if(writeReg(REG_TEMP_CONFIG, 0x01) != MAX30102_NO_ERROR) return MAX30102_TEMP_ERROR;
    wait_ms(30);
    if(readReg(REG_TEMP_CONFIG, dataBuf) != MAX30102_NO_ERROR) return MAX30102_TEMP_ERROR;
    while (dataBuf[0]) {
//        Thread::wait(1);
        wait_ms(1);
        if(readReg(REG_TEMP_CONFIG, dataBuf) != MAX30102_NO_ERROR) return MAX30102_TEMP_ERROR;
    }
    dataBuf[0] = REG_TEMP_INT;
    if (_i2c.write(MAX30102_I2C_ADDR, dataBuf, 1) != 0) return MAX30102_TEMP_ERROR;
    if (_i2c.read(MAX30102_I2C_ADDR, dataBuf, 2) != 0) return MAX30102_TEMP_ERROR;
    return ((float)dataSigned[0] + ((float)dataSigned[1] * 0.0625));
*/
}

int shutDown()
{
	PIT_stop(PIT1);
    _modeConfiguration |= MASK_SHDN;
    writeReg(REG_MODE_CONFIG, _modeConfiguration);
    return MAX30102_NO_ERROR;
}

int wakeUp()
{
    _modeConfiguration &= ~MASK_SHDN;
    writeReg(REG_MODE_CONFIG, _modeConfiguration);
    return MAX30102_NO_ERROR;
}

int softReset()
{
    writeReg(REG_MODE_CONFIG, MASK_RESET);
    _interruptEnable1 = 0x00;
    _interruptEnable2 = 0x00;
    _fifoConfiguration = 0x00;
    _modeConfiguration = 0x00;
    _spo2Configuration = 0x00;
    _led1PulseAmplitude = 0x00;
    _led2PulseAmplitude = 0x00;
    _led3PulseAmplitude = 0x00;
    _pilotPulseAmplitude = 0x00;
    _multiLedControl1 = 0x00;
    _multiLedControl2 = 0x00;
    _proxIntThreshold = 0x00;
    return MAX30102_NO_ERROR;
}

int ambientLigthNoise()
{
    readReg(REG_INTR_STATUS_1, _ambientLigth);
    *_ambientLigth &= INTR_ALC_OVF;
    if(*_ambientLigth != 0)
    	return 1;
    else
    	return 0;
}

bool MAX30102_convertionRdy(void) {
	return PINread(PTB9);
}

bool OXIdata2read(void) {
	if (OXIdataRdy) {
		OXIdataRdy = false;
		return true;
	}
	else
		return false;
}

void getOXIdata(uint32_t* buffred_, uint32_t* buffir_) {
	uint8_t i;

	for (i = 0; i < MAX30102_BUFF_SIZE; i++) {
		buffred_[i] = buffRED[i];
		buffir_[i] = buffIR[i];
	}
}

void MAX30102read(void) {
	uint32_t redLED, irLED;								// Almacenamiento de las muestras de red y ir
	static bool meassureAfected = false;
	static uint8_t i= 0;

	if (!MAX30102_convertionRdy()) {
		if(readFIFO(&redLED, &irLED) == MAX30102_ERROR) {
//			PRINTF("Error al realizar la lectura de las luces \n");
			//break;
			meassureAfected = false;
			i = 0;
		}
		else {
			//PRINTF("Rojo: %d  |  InfraRojo: %d\r\n", redLED, irLED);
			buffIR[i] = irLED;
			buffRED[i++] = redLED;
		}
		if(ambientLigthNoise())
			meassureAfected = true;
	}
	if(i >= MAX30102_BUFF_SIZE) {
		if(meassureAfected) {
//			PRINTF("Medición afectada por la luz ambiente! \n");
			meassureAfected = false;
		}
		OXIdataRdy = true;
		i = 0;
	}
}
