#ifndef __MAX30205_H_
#define __MAX30205_H_

#include <stdint.h>
#include <stdbool.h>

/*************************************** PARAMETROS DE CONFIGURACION DEL MAX30205 ***********************************************
 ********************************************************************************************************************************
 ********************************************************************************************************************************/

					// Posibles salve address del MAX30205 segun la conexion electrica de A0, A1 y A2
typedef enum {
	MAX30205_ADDRESS_DEFAULT   = 0x48,					// Según algunas notas el address es este
	MAX30205_ADDRESS_DEFAULT_AUX   = 0x90,				// A0, A1 y A2 a GND
	//...
}salveAddress_t;

					// Los Adresses de los registros del MAX30205
typedef enum {
    Temperature   = 0x00,
    Configuration = 0x01,
    THYST         = 0x02,			// limite inferior del comparador
    TOS           = 0x03			// limite superior del comparador
}MAX30205_Registers_t;

					// Bitfields para el registro de configuración del MAX30205
typedef union Configuration
{
    uint8_t all;
    struct BitField_s
    {
        uint8_t shutdown    : 1;			// 1-> modo de ahorro de energia / 0-> funcionamiento normal
        uint8_t comp_int    : 1;			// el MAX30205 tiene una funcion de comparador en el cual ve que la temperatura actual este dentro de un intervalor determinado por THYST y TOS (tamb tiene funcion de interrupcion)
        uint8_t os_polarity : 1;			// 0-> active low / 1-> active high
        uint8_t fault_queue : 2;			// numero de "faults" necesarios para trigerear el OS
        uint8_t data_format : 1;			// 0-> formato normal / 1-> formato extendido
        uint8_t timeout     : 1;			// 1-> disable bus timeout / 0-> to reset the I2C-compatible interface when SDA is low for more than 50ms
        uint8_t one_shot    : 1;			// para consumo de potencia
    }bits;
}Configuration_u;	// El bitfield va menos a mas significativo

typedef enum{
	CELSIUS,
    FAHRENHEIT,
    UNITS_N
}temp_unit_t;

extern const Configuration_u defaul_MAX30205_config;

/********************************************************************************************************************************
 ********************************************************************************************************************************
 ********************************************************************************************************************************/

// Cuando la tempreatura supera Tos el pin out OS se pone en HIGH
// cuando deciende por debajo de Thyst el Os se pone en LOW
// La frecuencia maxima de I2C que se banca es 400 KHz


/********************************************* FUNCIONES DEL MAX30205 ***********************************************************
 ********************************************************************************************************************************
 ********************************************************************************************************************************/

/*
* @brief: Se setea el salve address del MAX30205 para poder establecer la comunicacion
* @param salveAdress: slave address
*/
void tempSensor_setSlaveAdr(const salveAddress_t salveAddress);

/**
* @brief Devuelve el estado del sensor de temperatura
*@return:si devuelve false, el sensor esta realizando la medicion
*en el caso contrario(true), la medicion esta lista
*/
bool MAX30205_transferReady(void);

/*
* @brief: Se extrae la lectura de la temperatura
* @param[out] value: Donde se escribira la lectura
* @return: void
*/
void MAX30205_requestTemp(void);
/*
* @brief: Se comunica con el MAX30205 llenando su registro de configuracion
* @param config: Configuracion que se escribira en el registro
* @return: Estado de la transmisión
*/
void MAX30205_writeConfig(const Configuration_u config);

/*
* @brief: Lee la configuracion actual segun su registro
* @param config: Se guarda alli la configuracion actual
* @return: void
*/
void MAX30205_requestConfig(void);

/*
* @brief: Se escribe el registro THYST
* @param value: valor a escribir
* @return: void
*/
void MAX30205_writeTHYST(uint16_t THYSTvalue);

/*
* @brief: Se lee el valor actual de THYST
* @param[out] value: donde se guarda el valor
* @return: void
*/
void MAX30205_requestTHYST(void);

/*
* @brief: Se escribe el registro TOS
* @param value: valor a escribir
* @return: void
*/
void MAX30205_writeTOS(uint16_t TOSvalue);

/*
* @brief: Se lee el valor actual de TOS
* @param[out] value: donde se guarda el valor
* @return: Estado de la transmisión
*/
void MAX30205_requestTOS(void);

/*
* @brief: Convierte los datos RAW de temperatura solicitada
* @param rawTemp: valor raw a convertir
* @return: el valor convertido
*/
float MAX30205_get_temp(temp_unit_t unit);

/*
* @brief: Devuelve la configuración actual
* @param void
* @return: configuración actual
*/
uint8_t MAX30205_get_config(void);

/*
* @brief: Devuelve el valor actual de THYST
* @param void
* @return: registro THYST
*/
uint16_t MAX30205_get_THYST(void);

/*
* @brief: Devuelve el valor actual de TOS
* @param void
* @return: registro TOS
*/
uint16_t MAX30205_get_TOS(void);


#endif //INTERRUPT_H
