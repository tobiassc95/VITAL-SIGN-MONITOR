#include "hc05.h"
#include "uart.h"

void HC05init (void) {
	UARTconfigure (UART_0, 9600, UART_PARITY_DISABLE); ///check baudrate.
	//UARTconfigure (UART_3, 9600, UART_PARITY_DISABLE); ///check baudrate.
	enableUARTinterrupt(UART_0); ///check
	///If it is desire to change the NAME, UART (baudrate), PSWD, ROLE, ... Then change this function name to HC05configure and add code according to video.
}

uint16_t HC05receive(char* msg, uint16_t Nbytes) { //Not useful for this TP.
	UARTreadMSG(UART_0, msg, Nbytes);
	return 0;
}

uint8_t HC05trasmit(const char* msg, uint16_t Nbytes) { ///check
	UARTwriteMSG(UART_0, msg, Nbytes);
	return 0;
}
