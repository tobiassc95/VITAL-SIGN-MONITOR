#ifndef I2C_h
#define I2C_H

#include <stdbool.h>
#include <stdint.h>

#define WRITE 1
#define READ 0

typedef enum {
	I2C_0,
	I2C_1,
	I2C_2
}I2C_MODULE;

void I2Cconfiguration (I2C_MODULE i2c_);

//void enableI2Cinterrupts (uint8_t i);
//void disableI2Cinterrupts (uint8_t i);

//para escribir la fifo de transmisor y leer la fifo de receptor.
void writeI2Cmessage(I2C_MODULE i2c_, uint8_t *msg, uint8_t Nbytes);
void readI2Cmessage(I2C_MODULE i2c_, uint8_t *msg, uint8_t Nbytes);

void blockingI2Ctransmition(I2C_MODULE i2c_, uint8_t address, uint8_t NbytesW, uint8_t NbytesR);
void I2Ctransmition(I2C_MODULE i2c_, uint8_t address, uint8_t NbytesW, uint8_t NbytesR);

bool isI2Cready(I2C_MODULE i2c_);
bool I2Cdata2read(I2C_MODULE i2c_);

#endif //I2C_H
