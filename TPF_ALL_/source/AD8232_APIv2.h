#ifndef __AD8232_H_
#define __AD8232_H_

/******************************************* HEADERS ***************************************************/

#include <stdbool.h>
#include <stdint.h>

/******************************************* CONSTANTES ***************************************************/

//#define AD8232_BUFF_SIZE 4
#define AD8232_BUFF_SIZE 250
#define AD8232_SAMPLING_FREC 100

/************************************** DECLARACIÓN DE FUNCIONES ************************************************/
/* Resumen:
 * funcion: inicialización del ECG
 * entrada: void
 * Salida: void
 */
void AD8232_init(void);

/* Resumen:
 * funcion: checkea la correcta conexion de los electrodos
 * entrada: void
 * Salida: 1 si todo ok, 0 si alguno de los electrodos que van al brazo izquierdo o al brazo derecho no estan conectados
 */
bool AD8232_checkState(void);

/* Resumen:
 * funcion: Entrega los valores almacenados en el buffer del ADC una vez este esté lleno
 * entrada[out]: puntero donde almacenar los datos
 * Salida: 1 si todo ok, 0 si aun los datos no estan listos
 */
//bool AD8232_read(uint16_t * data);

void getCARDIOdata(uint16_t* data);

bool CARDIOdata2read(void);

#endif
