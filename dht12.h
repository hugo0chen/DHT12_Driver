#ifndef _BH1750_H
#define _BH1750_H
#include "stm32l1xx.h"

#define DHT12_ADDR 					0xB8
#define READ_COMMAND				0x01
#define WRITE_COMMAND				0x00
#define REG_HUMIDITY_HIGH   0x00
#define REG_HUMIDITY_LOW    0x01
#define REG_TEMP_HIGH       0x02
#define REG_TEMP_LOW        0x03
#define REG_CRC							0x04

void Init_DHT12(void);	
uint8_t Convert_DHT12(float* Temprature, float* Humi);
#endif
