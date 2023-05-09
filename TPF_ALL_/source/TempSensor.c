/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

//#include <i2c_API.h>
#include "i2c.h"
#include "TempSensor.h"
#include "stdint.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define TEMPERATURE_SENSOR_RECIVE_DATA_SIZE 2
#define THYST_REGISTER_SIZE 2
#define TOS_REGISTER_SIZE 2
#define CONFIGURATION_REGISTER_SIZE 1

/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

//bool writeRegister(MAX30205_Registers_t reg, uint16_t value);
//bool readRegister(MAX30205_Registers_t reg, uint16_t *value);

uint16_t actualTemp=0;//donde se almacena la temperatura actual medida
uint8_t volatile tempBuff[TEMPERATURE_SENSOR_RECIVE_DATA_SIZE];	//buffer donde se almacena la informacion recivida por el registro de temperatura
uint8_t volatile thystBuff[THYST_REGISTER_SIZE];				//buffer donde se almacena la informacion recivida por el registro de thyst
uint8_t volatile tosBuff[TOS_REGISTER_SIZE];					//buffer donde se almacena la informacion recivida por el registro de tos
uint8_t volatile configur[CONFIGURATION_REGISTER_SIZE];									//información sobre la configuración actual del dispositivo
uint8_t salveAddress;
const Configuration_u defaul_MAX30205_config = {.all = 0b01000110};
uint8_t TEMPi2cmsg[3]; //to I2C.


/*************************************************************************************************************************************
 *************************************************************************************************************************************
 *************************************************************************************************************************************/




/********************************************** DEFINICION DE LAS FUNCIONES API ******************************************************
 *************************************************************************************************************************************
 *************************************************************************************************************************************/


void tempSensor_setSlaveAdr(const salveAddress_t salveAdrs) //Importante setear el address antes de usar el dispositivo
{
	salveAddress = salveAdrs;
	I2Cconfiguration(I2C_0); //Necesitamos el I2C para leer los datos.
}

bool MAX30205_transferReady(void){
//	if(i2c_status() == i2c_IDLE)
//		return true;
//	else
//		return false;
	return isI2Cready(I2C_0);
}

void MAX30205_requestTemp(void) //Solicita la lectura de la temperatura
{
	//i2c_readReg(salveAddress, Temperature, tempBuff, TEMPERATURE_SENSOR_RECIVE_DATA_SIZE);
	if (isI2Cready(I2C_0)) {
		TEMPi2cmsg[0] = Temperature;
		writeI2Cmessage(I2C_0, TEMPi2cmsg, 1);
		I2Ctransmition(I2C_0, salveAddress, 1, TEMPERATURE_SENSOR_RECIVE_DATA_SIZE);
		//readI2Cmessage(uint8_t *TEMPi2cmsg, uint8_t Nbytes);
	}
}

void MAX30205_writeConfig(const Configuration_u config)
{
	*configur = config.all;
	TEMPi2cmsg[0] = Configuration; TEMPi2cmsg[1] = *configur;
	//i2c_writeReg(salveAddress, Configuration, configur, CONFIGURATION_REGISTER_SIZE);
	writeI2Cmessage(I2C_0, TEMPi2cmsg, CONFIGURATION_REGISTER_SIZE+1);
	blockingI2Ctransmition(I2C_0, salveAddress, CONFIGURATION_REGISTER_SIZE+1, 0);
}

void MAX30205_requestConfig(void)
{
	TEMPi2cmsg[0] = Configuration;
	//i2c_readReg(salveAddress, Configuration, configur, CONFIGURATION_REGISTER_SIZE);
	writeI2Cmessage(I2C_0, TEMPi2cmsg, 1);
	blockingI2Ctransmition(I2C_0, salveAddress, 1, CONFIGURATION_REGISTER_SIZE);
	readI2Cmessage(I2C_0, configur, CONFIGURATION_REGISTER_SIZE);
}

void MAX30205_writeTHYST(uint16_t THYSTvalue)	//recordar que son 2 registros de 8bits para el THYST
{
	thystBuff[0] = (0x00FF & THYSTvalue >> 8);
	thystBuff[1] = (0x00FF & THYSTvalue);
	TEMPi2cmsg[0] = THYST; TEMPi2cmsg[1] = thystBuff[0]; TEMPi2cmsg[2] = thystBuff[1];
	//i2c_writeReg(salveAddress, THYST, thystBuff, THYST_REGISTER_SIZE);
	writeI2Cmessage(I2C_0, TEMPi2cmsg, THYST_REGISTER_SIZE+1);
	blockingI2Ctransmition(I2C_0, salveAddress, THYST_REGISTER_SIZE+1, 0);
}

void MAX30205_requestTHYST(void)
{
	TEMPi2cmsg[0] = THYST;
	//i2c_readReg(salveAddress, THYST, thystBuff, THYST_REGISTER_SIZE);
	writeI2Cmessage(I2C_0, TEMPi2cmsg, 1);
	blockingI2Ctransmition(I2C_0, salveAddress, 1, THYST_REGISTER_SIZE);
	readI2Cmessage(I2C_0, thystBuff, THYST_REGISTER_SIZE);
}

void MAX30205_writeTOS(uint16_t TOSvalue)	//recordar que son 2 registros de 8bits para el TOS
{
	tosBuff[0] = (0x00FF & TOSvalue >> 8);
	tosBuff[1] = (0x00FF & TOSvalue);
	TEMPi2cmsg[0] = TOS; TEMPi2cmsg[1] = tosBuff[0]; TEMPi2cmsg[2] = tosBuff[1];
	//i2c_writeReg(salveAddress, TOS, tosBuff, TOS_REGISTER_SIZE);
	writeI2Cmessage(I2C_0, TEMPi2cmsg, TOS_REGISTER_SIZE+1);
	blockingI2Ctransmition(I2C_0, salveAddress, TOS_REGISTER_SIZE+1, 0);
}

void MAX30205_requestTOS(void)
{
	TEMPi2cmsg[0] = TOS;
	//i2c_readReg(salveAddress, TOS, tosBuff, TOS_REGISTER_SIZE);
	writeI2Cmessage(I2C_0, TEMPi2cmsg, 1);
	blockingI2Ctransmition(I2C_0, salveAddress, 1, TOS_REGISTER_SIZE);
	readI2Cmessage(I2C_0, tosBuff, TOS_REGISTER_SIZE);
}

float MAX30205_get_temp(temp_unit_t unit)
{
	float Temp;
	if(I2Cdata2read(I2C_0)) {
		readI2Cmessage(I2C_0, tempBuff, sizeof(tempBuff)); //sizeof(tempBuff) = 2
		actualTemp = tempBuff[0] << 8 | tempBuff[1];
		Temp=actualTemp* 0.00390625;
		if (unit==FAHRENHEIT)
			Temp=(Temp*(9.0/5.0))+32;
		return Temp;
	}
	else
		return 0;
}

uint8_t MAX30205_get_config(void)
{
	uint8_t ret = *configur;
    return ret;
}

uint16_t MAX30205_get_THYST(void)
{
	uint16_t ret;
	ret = thystBuff[0] << 8 | thystBuff[1];
    return ret;
}

uint16_t MAX30205_get_TOS(void)
{
	uint16_t ret;
	ret = tosBuff[0] << 8 | tosBuff[1];
    return ret;
}



//bool writeRegister(MAX30205_Registers_t reg, uint16_t value)
//{
//  uint8_t hi = ((value >> 8) & 0xFF);
//  uint8_t lo = (value & 0xFF);
//  unsigned char cmdData[2] = {hi, lo};
//
//  memset(&I2C_1_transfer, 0, sizeof(I2C_1_transfer));
//  I2C_1_transfer.slaveAddress = salveAddress;
//  I2C_1_transfer.direction = kI2C_Write;
//  I2C_1_transfer.subaddress = reg;
//  I2C_1_transfer.subaddressSize = 1;
//  I2C_1_transfer.data = cmdData;
//  I2C_1_transfer.dataSize = 2;
//  I2C_1_transfer.flags = kI2C_TransferDefaultFlag;
//
//  I2C_MasterTransferNonBlocking(I2C0, &I2C_1_handle, &I2C_1_transfer);
//
//  /*  wait for transfer completed. */
//  while ((!nakFlag_1) && (!completionFlag_1))
//  {
//  }
//
//  nakFlag_1 = false;
//
//  if (completionFlag_1 == true)
//  {
//	  completionFlag_1 = false;
//	  return true;
//  }
//  else
//  {
//	  return false;
//  }
//
//  //writeI2Cmessage(cmdData, 3);			//escribimos el registro que queremos escrbir y el correspondiente valor, se gurdan en la fifo de escritura
//  //I2Ctransmition(slaveAddress, 3, 0);	//queremos hacer 3 bytes de escritura y ninguno de lectura
//  //result = m_i2c.write(m_writeAddress, cmdData, 3);
//}

//bool readRegister(MAX30205_Registers_t reg, uint16_t *value)
//{
//  unsigned char data[2];
//  //char cmdData[1] = {reg};
//
//  memset(&I2C_1_transfer, 0, sizeof(I2C_1_transfer));
//  I2C_1_transfer.slaveAddress = salveAddress;
//  I2C_1_transfer.direction = kI2C_Read;
//  I2C_1_transfer.subaddress = reg;
//  I2C_1_transfer.subaddressSize = 1;
//  I2C_1_transfer.data = data;
//  I2C_1_transfer.dataSize = 2;
//  I2C_1_transfer.flags = kI2C_TransferDefaultFlag;
//
//  /*  direction=write : start+device_write;cmdbuff;xBuff; */
//  /*  direction=recive : start+device_write;cmdbuff;repeatStart+device_read;xBuff; */
//
//  I2C_MasterTransferNonBlocking(I2C0, &I2C_1_handle, &I2C_1_transfer);
//
//  /*  wait for transfer completed. */
//  while ((!nakFlag_1) && (!completionFlag_1))	// ESTO LO HACE BLOQUEANTE
//  {												// LUEGO SE PUEDE SACAR Y HACER UNA FUNCION QUE CONSULTE EL ESTADO DE LA TRANSIMISIÓN
//  }
//
//  *value = (data[0] << 8) + data[1];
//
//  nakFlag_1 = false;
//
//  if (completionFlag_1 == true)
// {
//      completionFlag_1 = false;
//      return true;
//  }
//  else
//  {
//      return false;
//  }
//
//  //writeI2Cmessage(cmdData, 1);			//le decimos al sensor de cual registro queremos leer
//  //I2Ctransmition(slaveAddress, 1, 2);	//queremos hacer 1 bytes de escritura y 2 de lectura
//  //readI2Cmessage(data, 2);
//}
