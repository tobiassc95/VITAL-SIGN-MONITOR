#include "flash.h"
#include "FreescaleIAP.h"

int flashWrite(char *data, uint16_t size, char **flashptr) {
	static uint8_t count = 0; //sector counter.
	uint8_t sectors;
    int address;
    uint8_t i;

    sectors = (size/SECTOR_SIZE)+1;
    count += sectors;
    address = flash_size() - count*SECTOR_SIZE;
    *flashptr = (char*)address;

    if (*flashptr[0] == 0xff) { //If first time running, program the flash with data.
		for (i = 0; i < sectors; i++) {
			if(erase_sector(address))
				return -1;
		}
		if(program_flash(address, data, size))
			return -1;
    }

    return 0;
}
