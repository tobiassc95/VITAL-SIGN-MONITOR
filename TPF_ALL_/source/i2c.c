#include "i2c.h"
#include "portpin.h"
#include "hardware.h"

#define BUFFSIZE UINT8_MAX


//I2C0.
#define PTE24 PORTNUM2PIN(PORT_E, 24) //SCL
#define PTE25 PORTNUM2PIN(PORT_E, 25) //SDA

//I2C1.
#define PTC10 PORTNUM2PIN(PORT_C, 10) //SCL
#define PTC11 PORTNUM2PIN(PORT_C, 11) //SDA

enum {
	IDLE,
	WRITEADDRESS,
	WRITEDATA,
	READDUMMY,
	READDATA
};

//typedef struct {
//	uint8_t data[FIFOSIZE];
//	uint8_t start; //puntero al inicio del mensaje.
//	uint8_t end; //puntero al final del mensaje.
//}FIFOWRITE; //FIFO para transmitir datos (write).
//static FIFOWRITE fifowrite;
//
//typedef struct {
//	uint8_t data[FIFOSIZE];
//	uint8_t start;
//	uint8_t end;
//}FIFOREAD; //FIFO para recibir datos (read).
//static FIFOREAD fiforead;

typedef struct { //Buffer
	uint8_t data[BUFFSIZE];
	uint8_t i;
}DATA_;
static DATA_ data_[2]; //one for each i2c peripheral.

typedef struct {
	uint8_t status; //From FSM.
	uint8_t address;
	uint8_t NbytesW;
	uint8_t NbytesR;
	bool data2read;
}I2CINFO;
static I2CINFO i2cinfo[2]; //one for each i2c peripheral.

static I2C_Type* i2c[3] = { I2C0, I2C1, I2C2 };

//funciones utiles
void I2Cmachine(I2C_MODULE i2c_);
void startSignal (I2C_MODULE i2c_) {i2c[i2c_]->C1 |= I2C_C1_MST_MASK | I2C_C1_TX_MASK;}
void restartSignal (I2C_MODULE i2c_) {i2c[i2c_]->C1 |= I2C_C1_RSTA_MASK;}
void stopSignal (I2C_MODULE i2c_) {i2c[i2c_]->C1 &= ~(I2C_C1_MST_MASK | I2C_C1_TX_MASK);}
void delayLoop (void) {uint16_t delay = UINT16_MAX; while(delay--);}

void I2Cmachine(I2C_MODULE i2c_) { //FSM = finit state machine.
	switch (i2cinfo[i2c_].status) {
	case WRITEADDRESS:
		if (i2c[i2c_]->S & I2C_S_RXAK_MASK) { //si nadie responde...
			stopSignal(i2c_);
			i2cinfo[i2c_].status = IDLE;
			//return; //fallo la comunicacion.
		}
		else {
			i2c[i2c_]->D = data_[i2c_].data[data_[i2c_].i++]; //escribimos el primer byte.
//			if (fifowrite.start == FIFOSIZE)
//				fifowrite.start = 0;
			i2cinfo[i2c_].status = WRITEDATA;
		}
		break;
	case WRITEDATA:
		if (i2c[i2c_]->S & I2C_S_RXAK_MASK) { //si el slave mando un NACK...
			stopSignal(i2c_);
			i2cinfo[i2c_].status = IDLE;
			data_[i2c_].i = 0;
			//return; //fallo la comunicacion.
		}
		else {
			//mientras aun hay bytes que mandar y no se vacio la fifowrite...
			if (--i2cinfo[i2c_].NbytesW) {
				i2c[i2c_]->D = data_[i2c_].data[data_[i2c_].i++]; //escribimos.
//				if (fifowrite.start == FIFOSIZE)
//					fifowrite.start = 0;
			}
			else {
				if(i2cinfo[i2c_].NbytesR) { // si terminamos de escribir y tambien tenemos para leer...
					restartSignal(i2c_);
					i2c[i2c_]->D = i2cinfo[i2c_].address | 0x01; //Now we read.
					i2cinfo[i2c_].status = READDUMMY;
					data_[i2c_].i = 0; //We'll fill up the data array with received data.
				}
				else {
					stopSignal(i2c_);
					i2cinfo[i2c_].status = IDLE;
					data_[i2c_].i = 0;
				}
			}
		}
		break;
	case READDUMMY:
		if (i2c[i2c_]->S & I2C_S_RXAK_MASK) { //si nadie responde...
			stopSignal(i2c_);
			i2cinfo[i2c_].status = IDLE;
			data_[i2c_].i = 0;
			//return; //fallo la comunicacion.
		}
		else {
			i2c[i2c_]->C1 &= ~(I2C_C1_TX_MASK | I2C_C1_TXAK_MASK); //master receive mode.
			if (i2cinfo[i2c_].NbytesR == 1)
				i2c[i2c_]->C1 |= I2C_C1_TXAK_MASK; //2nd to last byte to be read, el master envia NACK para terminar la comunicación.
			i2c[i2c_]->D; //hacemos dummy read.
			i2cinfo[i2c_].status = READDATA;
		}
		break;
	case READDATA:
		//mientras aun hay bytes que leer y no se lleno la fiforead...
		if (--i2cinfo[i2c_].NbytesR) {
			if (i2cinfo[i2c_].NbytesR == 1) //anteultimo byte de data a leer.
				i2c[i2c_]->C1 |= I2C_C1_TXAK_MASK; //2nd to last byte to be read, el master envia NACK para terminar la comunicación.
			data_[i2c_].data[data_[i2c_].i++] = i2c[i2c_]->D; //leemos.
//			if (fiforead.end == FIFOSIZE)
//				fiforead.end = 0;
		}
		else {
			stopSignal(i2c_); //last byte to be read.
			i2cinfo[i2c_].status = IDLE;
			i2cinfo[i2c_].data2read = true;
			data_[i2c_].data[data_[i2c_].i++] = i2c[i2c_]->D; //leemos el ultimo byte.
			data_[i2c_].i = 0;
		}
		break;
	}
}

bool isI2Cready(I2C_MODULE i2c_) {
	if(!(i2c[i2c_]->S & I2C_S_BUSY_MASK) && i2cinfo[i2c_].status == IDLE)
		return true;
	else
		return false;
}

bool I2Cdata2read(I2C_MODULE i2c_) {
	if(i2cinfo[i2c_].data2read) {
		i2cinfo[i2c_].data2read = false;
		return true;
	}
	else
		return false;
}

void I2Cconfiguration (I2C_MODULE i2c_) {
	//inicializamos clock.
	if (i2c_ == I2C_0 || i2c_ == I2C_1)
		SIM->SCGC4 |= SIM_SCGC4_I2C0_MASK << i2c_;
//	else if (i2c_ == I2C_2)
//		SIM->SCGC1 |= SIM_SCGC1_I2C2_MASK;

	//deshabilitamos el modulo para configurarlo.
	i2c[i2c_]->C1 &= ~(I2C_C1_IICEN_MASK);
	//reseteamos
	i2c[i2c_]->F = I2C_F_ICR(0x20) | I2C_F_MULT(0x02); //baudrate = 93750 bps.
	//i2c[i2c_]->F = 0; //baudrate = 3 Mbps.
	i2c[i2c_]->C1 = 0;
	i2c[i2c_]->S = 0x12;
	i2c[i2c_]->C2 = 0;
	i2c[i2c_]->FLT = 0x50U;

	i2c[i2c_]->F = 0x94;
	i2c[i2c_]->C1 |= I2C_C1_IICEN_MASK | I2C_C1_IICIE_MASK; //habilitamos el modulo

	//configuramos los pines del i2c que se usara (i2c[i2c_]).
	if(i2c_ == I2C_0) {
		PINconfigure(PTE24, PIN_MUX5, PIN_IRQ_DISABLE);
		PINopendrain(PTE24);///
		PINconfigure(PTE25, PIN_MUX5, PIN_IRQ_DISABLE);
		PINopendrain(PTE25);
	}
	else if(i2c_ == I2C_1) {
		PINconfigure(PTC10, PIN_MUX2, PIN_IRQ_DISABLE);
		PINopendrain(PTC10);///
		PINconfigure(PTC11, PIN_MUX2, PIN_IRQ_DISABLE);
		PINopendrain(PTC11);
	}
//	switch () {
//	case 0:
//		pinPCRconfigure(PTE24, MUX(ALT5) | OPENDRAIN);
//		pinPCRconfigure(PTE25, MUX(ALT5) | OPENDRAIN); //(PCRCFG){IRQ_DISABLE, ALT5, 0, 1 ,0 , 1, 1}
//		break;
//	case 1:
//		break;
//	case 2:
//		break;
//	}

	i2cinfo[I2C_0].status = IDLE;
	i2cinfo[I2C_0].data2read = false;
	data_[I2C_0].i = 0;
	i2cinfo[I2C_1].status = IDLE;
	i2cinfo[I2C_1].data2read = false;
	data_[I2C_1].i = 0;
//	i2cinfo[I2C_2].status = IDLE;
//	i2cinfo[I2C_2].data2read = false;
//	data_[I2C_2].i = 0;
//	fifowrite.start = 0;
//	fifowrite.end = 0;
//	fiforead.start = 0;
//	fiforead.end = 0;
}

void writeI2Cmessage(I2C_MODULE i2c_, uint8_t *msg, uint8_t Nbytes) {
	for (data_[i2c_].i = 0; data_[i2c_].i < Nbytes && data_[i2c_].i < BUFFSIZE ; data_[i2c_].i++) {
		data_[i2c_].data[data_[i2c_].i] = msg[data_[i2c_].i];
	}
	data_[i2c_].i = 0;

//	for (j = 0; j < Nbytes && (fifowrite.end - fifowrite.start != FIFOSIZE - 1); j++) {
//		fifowrite.data[fifowrite.end++] = msg[j];
//		if (fifowrite.end == FIFOSIZE)
//			fifowrite.end = 0;
//	}
}

void readI2Cmessage(I2C_MODULE i2c_, uint8_t *msg, uint8_t Nbytes) {
	for (data_[i2c_].i = 0; data_[i2c_].i < Nbytes && data_[i2c_].i < BUFFSIZE ; data_[i2c_].i++) {
		msg[data_[i2c_].i] = data_[i2c_].data[data_[i2c_].i];
//		if (fiforead.start == FIFOSIZE)
//			fiforead.start = 0;
	}
	data_[i2c_].i = 0;
}

void blockingI2Ctransmition(I2C_MODULE i2c_, uint8_t address, uint8_t NbytesW, uint8_t NbytesR) {
	i2cinfo[i2c_].address = address;
	i2cinfo[i2c_].NbytesW = NbytesW;
	i2cinfo[i2c_].NbytesR = NbytesR;

	if(!(i2c[i2c_]->S & I2C_S_BUSY_MASK) && i2cinfo[i2c_].status == IDLE) {
		//preparamos la direccion del slave junto con la direccion de la comunicacion.
		i2cinfo[i2c_].address <<= 1;
		if(i2cinfo[i2c_].NbytesW)
			i2cinfo[i2c_].address |= 0x00; //al shiftear el address, automaticamente el lsb es 0 -> write.
		else if(i2cinfo[i2c_].NbytesR) //si no vamos a escribir nada, pero vamos a leer...
			i2cinfo[i2c_].address |= 0x01; //vamos a leer.
		else //si no vamos a escribir ni a leer...
			return;

		//inicializamos la transmision. mandamos start.
		if(i2cinfo[i2c_].NbytesW)
			i2cinfo[i2c_].status = WRITEADDRESS;
		else if(i2cinfo[i2c_].NbytesR) //si no vamos a escribir nada, pero vamos a leer...
			i2cinfo[i2c_].status = READDUMMY; //vamos a leer.
		//delayLoop(); //Needs to be here so the status can be written by the CPU. It may not be necessary in blocking function.
		startSignal(i2c_);
		i2c[i2c_]->D = i2cinfo[i2c_].address;
	}
	else
		return;

	while (i2cinfo[i2c_].status != IDLE) {
		while (!(i2c[i2c_]->S & I2C_S_IICIF_MASK)); //esperamos hasta haya interrupcion (leer reference manual). THis is the ACK!
		i2c[i2c_]->S = I2C_S_IICIF_MASK; //bajamos el flag de interrupciones de i2c.

		I2Cmachine(i2c_);
		if(i2cinfo[i2c_].status == IDLE)
			delayLoop(); //We wait a little, and then we return.
	}
}

void I2Ctransmition(I2C_MODULE i2c_, uint8_t address, uint8_t NbytesW, uint8_t NbytesR) {
	i2cinfo[i2c_].address = address;
	i2cinfo[i2c_].NbytesW = NbytesW;
	i2cinfo[i2c_].NbytesR = NbytesR;

	if(!(i2c[i2c_]->S & I2C_S_BUSY_MASK) && i2cinfo[i2c_].status == IDLE) {
		//preparamos la direccion del slave junto con la direccion de la comunicacion.
		i2cinfo[i2c_].address <<= 1;
		if(i2cinfo[i2c_].NbytesW)
			i2cinfo[i2c_].address |= 0x00; //al shiftear el address, automaticamente el lsb es 0 -> write.
		else if(i2cinfo[i2c_].NbytesR) //si no vamos a escribir nada, pero vamso a leer...
			i2cinfo[i2c_].address |= 0x01; //vamos a leer.
		else //si no vamos a escribir ni a leer...
			return;

		//inicializamos la transmicion. mandamos start.
		if(i2cinfo[i2c_].NbytesW)
			i2cinfo[i2c_].status = WRITEADDRESS; //al shiftear el address, automaticamente el lsb es 0 -> write.
		else if(i2cinfo[i2c_].NbytesR) //si no vamos a escribir nada, pero vamso a leer...
			i2cinfo[i2c_].status = READDUMMY; //vamos a leer.
		delayLoop(); //Needs to be here so the status can be written by the CPU. In that case, put the startSignal() above the closest if.
		//		if (i != 2)
		//			NVIC_EnableIRQ(I2C0_IRQn + i);
		//		else
		//			NVIC_EnableIRQ(I2C2_IRQn);
		NVIC_ClearPendingIRQ(I2C0_IRQn + i2c_);
		NVIC_EnableIRQ(I2C0_IRQn + i2c_);
		startSignal(i2c_);
		i2c[i2c_]->D = i2cinfo[i2c_].address;
	}
	else
		return;
}

__ISR__ I2C0_IRQHandler(void) {
	i2c[I2C_0]->S = I2C_S_IICIF_MASK; //bajamos el flag de interrupciones de i2c.

	I2Cmachine(I2C_0);
	if(i2cinfo[I2C_0].status == IDLE) {
		NVIC_DisableIRQ(I2C0_IRQn);
		delayLoop();
	}
}

__ISR__ I2C1_IRQHandler(void) {
	i2c[I2C_1]->S = I2C_S_IICIF_MASK; //bajamos el flag de interrupciones de i2c.

	I2Cmachine(I2C_1);
	if(i2cinfo[I2C_1].status == IDLE) {
		NVIC_DisableIRQ(I2C1_IRQn);
		delayLoop();
	}
}
