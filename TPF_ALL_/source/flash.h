#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

int flashWrite(char *data, uint16_t size, char **flashptr);
//int flashAddress(uint16_t size);

#endif //FLASH_H
