#include "uda1380.h"
#include "i2c.h"

#define UDA1380_WRITE_ADDRESS     0x30 >> 1 //Actually, it is 0x30. But the address is then shifted to introduce the r/w bit.
#define UDA1380_SOFTWARE_RESET    0x7F
#define UDA1380_REG_EVALCLK	      0x00
#define UDA1380_REG_I2S		      0x01
#define UDA1380_REG_PWRCTRL	      0x02
#define UDA1380_REG_ANAMIX	      0x03
#define UDA1380_REG_HEADAMP	      0x04
#define UDA1380_REG_MSTRVOL	      0x10
#define UDA1380_REG_MIXVOL	      0x11
#define UDA1380_REG_MODEBBT	      0x12
#define UDA1380_REG_MSTRMUTE      0x13
#define UDA1380_REG_MIXSDO	      0x14
#define UDA1380_REG_DECVOL	      0x20
#define UDA1380_REG_PGA		      0x21
#define UDA1380_REG_ADC		      0x22
#define UDA1380_REG_AGC		      0x23

uint8_t UDAi2cmsg[3];

void UDA1380Init(void) {
	//i2c config
	I2Cconfiguration(I2C_0);

	//uda config
	UDAi2cmsg[0] = UDA1380_SOFTWARE_RESET; UDAi2cmsg[1] = 0x00; UDAi2cmsg[2] = 0x00; //CODEC ADC and DAC clock from WSPLL, all clocks enabled///
	writeI2Cmessage(I2C_0, UDAi2cmsg, sizeof(UDAi2cmsg));
	blockingI2Ctransmition(I2C_0, UDA1380_WRITE_ADDRESS, sizeof(UDAi2cmsg), 0); //bloqueante ya que EN EL INIT (ATENCION) LA INTERRUPCIONES ESTAN DES HA BI LI TA DAS.

	UDAi2cmsg[0] = UDA1380_REG_EVALCLK; UDAi2cmsg[1] = 0x0F; UDAi2cmsg[2] = 0x32; //CODEC ADC and DAC clock from WSPLL, all clocks enabled///
	writeI2Cmessage(I2C_0, UDAi2cmsg, sizeof(UDAi2cmsg));
	blockingI2Ctransmition(I2C_0, UDA1380_WRITE_ADDRESS, sizeof(UDAi2cmsg), 0); //bloqueante ya que EN EL INIT (ATENCION) LA INTERRUPCIONES ESTAN DES HA BI LI TA DAS.

	UDAi2cmsg[0] = UDA1380_REG_I2S; UDAi2cmsg[1] = 0x00; UDAi2cmsg[2] = 0x00; //I2S bus data I/O formats, use digital mixer for output. BCKO is slave.
	writeI2Cmessage(I2C_0, UDAi2cmsg, sizeof(UDAi2cmsg));
	blockingI2Ctransmition(I2C_0, UDA1380_WRITE_ADDRESS, sizeof(UDAi2cmsg), 0); //bloqueante ya que EN EL INIT (ATENCION) LA INTERRUPCIONES ESTAN DES HA BI LI TA DAS.

	UDAi2cmsg[0] = UDA1380_REG_PWRCTRL; UDAi2cmsg[1] = 0xA5; UDAi2cmsg[2] = 0xDF; //Enable all power for now
	writeI2Cmessage(I2C_0, UDAi2cmsg, sizeof(UDAi2cmsg));
	blockingI2Ctransmition(I2C_0, UDA1380_WRITE_ADDRESS, sizeof(UDAi2cmsg), 0); //bloqueante ya que EN EL INIT (ATENCION) LA INTERRUPCIONES ESTAN DES HA BI LI TA DAS.

	UDAi2cmsg[0] = UDA1380_REG_ANAMIX; UDAi2cmsg[1] = 0x00; UDAi2cmsg[2] = 0x00; //Full mixer analog input gain
	writeI2Cmessage(I2C_0, UDAi2cmsg, sizeof(UDAi2cmsg));
	blockingI2Ctransmition(I2C_0, UDA1380_WRITE_ADDRESS, sizeof(UDAi2cmsg), 0); //bloqueante ya que EN EL INIT (ATENCION) LA INTERRUPCIONES ESTAN DES HA BI LI TA DAS.

	UDAi2cmsg[0] = UDA1380_REG_HEADAMP; UDAi2cmsg[1] = 0x02; UDAi2cmsg[2] = 0x02; //Enable headphone short circuit protection
	writeI2Cmessage(I2C_0, UDAi2cmsg, sizeof(UDAi2cmsg));
	blockingI2Ctransmition(I2C_0, UDA1380_WRITE_ADDRESS, sizeof(UDAi2cmsg), 0); //bloqueante ya que EN EL INIT (ATENCION) LA INTERRUPCIONES ESTAN DES HA BI LI TA DAS.

	UDAi2cmsg[0] = UDA1380_REG_MSTRVOL; UDAi2cmsg[1] = 0x00; UDAi2cmsg[2] = 0x00; //Full master volume///
	writeI2Cmessage(I2C_0, UDAi2cmsg, sizeof(UDAi2cmsg));
	blockingI2Ctransmition(I2C_0, UDA1380_WRITE_ADDRESS, sizeof(UDAi2cmsg), 0); //bloqueante ya que EN EL INIT (ATENCION) LA INTERRUPCIONES ESTAN DES HA BI LI TA DAS.

	UDAi2cmsg[0] = UDA1380_REG_MIXVOL; UDAi2cmsg[1] = 0x00; UDAi2cmsg[2] = 0x00; //Enable full mixer volume on both channels
	writeI2Cmessage(I2C_0, UDAi2cmsg, sizeof(UDAi2cmsg));
	blockingI2Ctransmition(I2C_0, UDA1380_WRITE_ADDRESS, sizeof(UDAi2cmsg), 0); //bloqueante ya que EN EL INIT (ATENCION) LA INTERRUPCIONES ESTAN DES HA BI LI TA DAS.

//	UDAi2cmsg[0] = UDA1380_REG_MODEBBT; UDAi2cmsg[1] = 0x55; UDAi2cmsg[2] = 0x15; //Bass and treble boost set to flat////
//	writeI2Cmessage(I2C_0, UDAi2cmsg, sizeof(UDAi2cmsg));
//	blockingI2Ctransmition(I2C_0, UDA1380_WRITE_ADDRESS, sizeof(UDAi2cmsg), 0); //bloqueante ya que EN EL INIT (ATENCION) LA INTERRUPCIONES ESTAN DES HA BI LI TA DAS.

	UDAi2cmsg[0] = UDA1380_REG_MSTRMUTE; UDAi2cmsg[1] = 0x00; UDAi2cmsg[2] = 0x00; //Disable mute and de-emphasis
	writeI2Cmessage(I2C_0, UDAi2cmsg, sizeof(UDAi2cmsg));
	blockingI2Ctransmition(I2C_0, UDA1380_WRITE_ADDRESS, sizeof(UDAi2cmsg), 0); //bloqueante ya que EN EL INIT (ATENCION) LA INTERRUPCIONES ESTAN DES HA BI LI TA DAS.

//	UDAi2cmsg[0] = UDA1380_REG_MIXSDO; UDAi2cmsg[1] = 0x00; UDAi2cmsg[2] = 0x00; //Mixer off, other settings off
//	writeI2Cmessage(I2C_0, UDAi2cmsg, sizeof(UDAi2cmsg));
//	blockingI2Ctransmition(I2C_0, UDA1380_WRITE_ADDRESS, sizeof(UDAi2cmsg), 0); //bloqueante ya que EN EL INIT (ATENCION) LA INTERRUPCIONES ESTAN DES HA BI LI TA DAS.

//	UDAi2cmsg[0] = UDA1380_REG_DECVOL; UDAi2cmsg[1] = 0x00; UDAi2cmsg[2] = 0x00; //ADC decimator volume to max
//	writeI2Cmessage(I2C_0, UDAi2cmsg, sizeof(UDAi2cmsg));
//	blockingI2Ctransmition(I2C_0, UDA1380_WRITE_ADDRESS, sizeof(UDAi2cmsg), 0); //bloqueante ya que EN EL INIT (ATENCION) LA INTERRUPCIONES ESTAN DES HA BI LI TA DAS.

//	UDAi2cmsg[0] = UDA1380_REG_PGA; UDAi2cmsg[1] = 0x00; UDAi2cmsg[2] = 0x00; //No PGA mute, full gain////
//	writeI2Cmessage(I2C_0, UDAi2cmsg, sizeof(UDAi2cmsg));
//	blockingI2Ctransmition(I2C_0, UDA1380_WRITE_ADDRESS, sizeof(UDAi2cmsg), 0); //bloqueante ya que EN EL INIT (ATENCION) LA INTERRUPCIONES ESTAN DES HA BI LI TA DAS.
//
//	UDAi2cmsg[0] = UDA1380_REG_ADC; UDAi2cmsg[1] = 0x0F; UDAi2cmsg[2] = 0x02; //Select line in and MIC, max MIC gain////
//	writeI2Cmessage(I2C_0, UDAi2cmsg, sizeof(UDAi2cmsg));
//	blockingI2Ctransmition(I2C_0, UDA1380_WRITE_ADDRESS, sizeof(UDAi2cmsg), 0); //bloqueante ya que EN EL INIT (ATENCION) LA INTERRUPCIONES ESTAN DES HA BI LI TA DAS.

//	UDAi2cmsg[0] = UDA1380_REG_AGC; UDAi2cmsg[1] = 0x00; UDAi2cmsg[2] = 0x00; //AGC
//	writeI2Cmessage(I2C_0, UDAi2cmsg, sizeof(UDAi2cmsg));
//	blockingI2Ctransmition(I2C_0, UDA1380_WRITE_ADDRESS, sizeof(UDAi2cmsg), 0); //bloqueante ya que EN EL INIT (ATENCION) LA INTERRUPCIONES ESTAN DES HA BI LI TA DAS.
}
