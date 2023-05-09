/*
 * packageTxHandler.h
 *
 *  Created on: 29 may. 2022
 *      Author: guido
 */

#ifndef PACKAGETXHANDLER_H_
#define PACKAGETXHANDLER_H_

void initPackage(void);
void createTempPackage(void);
void createOximeterPackage(void);
void createECGPackage(void);


//void setTempData(uint8_t temp);
void setTempData(float temp);
void setOximeterData(uint32_t* red, uint8_t hr, float spo2);
void setECGData(uint16_t* ecg_buff);


void sendTempPackage(void);
void sendOximeterPackage(void);
void sendECGPackage(void);

#endif /* PACKAGETXHANDLER_H_ */
