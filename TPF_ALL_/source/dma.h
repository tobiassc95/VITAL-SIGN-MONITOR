#ifndef DMA_H
#define DMA_H

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"

typedef enum {
	DMA_NORMAL,
	DMA_PERIODIC
}DMA_MODE;

typedef enum {
	DMA_CH0,
	DMA_CH1,
	DMA_CH2,
	DMA_CH3,
	DMA_CH4,
	DMA_CH5,
	DMA_CH6,
	DMA_CH7,
	DMA_CH8,
	DMA_CH9,
	DMA_CH10,
	DMA_CH11,
	DMA_CH12,
	DMA_CH13,
	DMA_CH14,
	DMA_CH15,
}DMA_CHANNEL;

typedef enum {
	DMAMUX_DISABLE,
	DMAMUX_UART0RX = 2, //UART0 Receive.
	DMAMUX_UART0TX, //UART0 Transmit.
	DMAMUX_UART1RX, //UART1 Receive.
	DMAMUX_UART1TX, //UART1 Transmit.
	DMAMUX_UART2RX, //UART2 Receive.
	DMAMUX_UART2TX, //UART2 Transmit.
	DMAMUX_UART3RX, //UART3 Receive.
	DMAMUX_UART3TX, //UART3 Transmit.
	DMAMUX_UART4, //UART4 Transmit or Receive.
	DMAMUX_UART5, //UART5 Transmit or Receive.
	DMAMUX_I2S0RX, //I2S0 Receive.
	DMAMUX_I2S0TX, //I2S0 Transmit.
	DMAMUX_SPI0RX, //SPI0 Receive.
	DMAMUX_SPI0TX, //SPI0 Transmit.
	DMAMUX_SPI1, //SPI1 Transmit or Receive.
	DMAMUX_SPI2, //SPI2 Transmit or Receive.
	DMAMUX_I2C0, //I2C0.
	DMAMUX_I2C1I2C2, //I2C1 and I2C2.
	DMAMUX_FTM0CH0, //FTM0 CH0.
	DMAMUX_FTM0CH1, //FTM0 CH1.
	DMAMUX_FTM0CH2, //FTM0 CH1.
	DMAMUX_FTM0CH3, //FTM0 CH1.
	DMAMUX_FTM0CH4, //FTM0 CH4.
	DMAMUX_FTM0CH5, //FTM0 CH5.
	DMAMUX_FTM0CH6, //FTM0 CH6.
	DMAMUX_FTM0CH7, //FTM0 CH7.
	DMAMUX_FTM1CH0, //FTM1 CH0.
	DMAMUX_FTM1CH1, //FTM1 CH1.
	DMAMUX_FTM2CH0, //FTM2 CH0.
	DMAMUX_FTM2CH1, //FTM2 CH1.
	DMAMUX_FTM3CH0, //FTM3 CH0.
	DMAMUX_FTM3CH1, //FTM3 CH1.
	DMAMUX_FTM3CH2, //FTM3 CH1.
	DMAMUX_FTM3CH3, //FTM3 CH1.
	DMAMUX_FTM3CH4, //FTM3 CH4.
	DMAMUX_FTM3CH5, //FTM3 CH5.
	DMAMUX_FTM3CH6, //FTM3 CH6.
	DMAMUX_FTM3CH7, //FTM3 CH7.
	DMAMUX_ADC0, //ADC0.
	DMAMUX_ADC1, //ADC1.
	DMAMUX_CMP0, //CMP0.
	DMAMUX_CMP1, //CMP1.
	DMAMUX_CMP2, //CMP2.
	DMAMUX_DAC0, //DAC0.
	DMAMUX_DAC1, //DAC1.
	DMAMUX_CMT, //CMT.
	DMAMUX_PDB, //PDB0.
	DMAMUX_PORTA, //PTA.
	DMAMUX_PORTB, //PTB.
	DMAMUX_PORTC, //PTC.
	DMAMUX_PORTD, //PTD.
	DMAMUX_PORTE //PTE.
//	kDmaRequestMux0IEEE1588Timer0   = 54|0x100U,   /**< ENET IEEE 1588 timer 0. */
//	kDmaRequestMux0IEEE1588Timer1   = 55|0x100U,   /**< ENET IEEE 1588 timer 1. */
//	kDmaRequestMux0IEEE1588Timer2   = 56|0x100U,   /**< ENET IEEE 1588 timer 2. */
//	kDmaRequestMux0IEEE1588Timer3   = 57|0x100U,   /**< ENET IEEE 1588 timer 3. */
//	kDmaRequestMux0AlwaysOn58       = 58|0x100U,   /**< DMAMUX Always Enabled slot. */
//	kDmaRequestMux0AlwaysOn59       = 59|0x100U,   /**< DMAMUX Always Enabled slot. */
//	kDmaRequestMux0AlwaysOn60       = 60|0x100U,   /**< DMAMUX Always Enabled slot. */
//	kDmaRequestMux0AlwaysOn61       = 61|0x100U,   /**< DMAMUX Always Enabled slot. */
//	kDmaRequestMux0AlwaysOn62       = 62|0x100U,   /**< DMAMUX Always Enabled slot. */
//	kDmaRequestMux0AlwaysOn63       = 63|0x100U,   /**< DMAMUX Always Enabled slot. */
}DMA_SOURCE;

typedef enum {
	DMA_8BITS,
	DMA_16BITS,
	DMA_32BITS
}DMA_DATASIZE;

typedef enum {
	DMA_INTMAJOR = 1,
	DMA_INTHALF,
	DMA_INTBOTH
}DMA_INTERRUPT;

void DMAconfigure(DMA_CHANNEL ch, DMA_MODE mode, DMA_SOURCE source, uint32_t *saddr, uint16_t soff/*bytes*/, DMA_DATASIZE ssize, uint32_t slast/*bytes*/,
				uint32_t *daddr, uint16_t doff/*bytes*/, DMA_DATASIZE dsize, uint32_t dlast/*bytes*/, uint32_t nbytes, uint16_t citer); //nbytes >= ssize!!
void DMAenable(DMA_CHANNEL ch);
void DMAenableIRQ(DMA_CHANNEL ch, DMA_INTERRUPT intrpt, void (*majorfunc) (void), void (*halffunc) (void));
void DMAdisable(DMA_CHANNEL ch);
void DMAdisableIRQ(DMA_CHANNEL ch);
void DMAstart(DMA_CHANNEL ch); //starts via software.
void DMAcancel(void); //cancel transfer.
bool DMAdone(DMA_CHANNEL ch);

#endif //DMA_H
