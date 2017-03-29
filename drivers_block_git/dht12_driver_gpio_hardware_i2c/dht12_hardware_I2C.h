#ifndef _BH1750_H
#define _BH1750_H
#include "stm32l1xx.h"

#define DHT12_ADDR 					0xB8
#define REG_HUMIDITY_HIGH   0x00
#define REG_HUMIDITY_LOW    0x01
#define REG_TEMP_HIGH       0x02
#define REG_TEMP_LOW        0x03
#define REG_CRC							0x04


struct dht12_Reg_Data{
	uint8_t humi_high;
	uint8_t humi_low;
	uint8_t temperature_high;
	uint8_t temperature_low;
	uint8_t cal_value;	
};

void inti_I2C2(void);
uint8_t getDHT12_Temp_Humi(float* temperature, float* huminity);
#endif
