#ifndef HC05_H
#define HC05_H

#include <stdint.h>

void HC05init (void);
uint16_t HC05receive(char* msg, uint16_t Nbytes);
uint8_t HC05trasmit(const char* msg, uint16_t Nbytes);

#endif //HC05_H
