#include "dht12.h"
#include "i2c.h"
#include "delay.h"
#include "stdio.h"


#define I2C_CLOCK 		 300000
#define I2C_OWN_ADDR   0xC5
/*************************************************************
  Function   :Init_DHT12
  Description:初始化dht12 
  Input      : none        
  return     : none    
*************************************************************/
static void init_I2C_InitStructure(void)
{
	I2C_InitTypeDef I2C_InitStructure;
	
	I2C_DeInit(I2C2);
	I2C_InitStructure.I2C_ClockSpeed = I2C_CLOCK;  		//100KHZ
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1 = I2C_OWN_ADDR;		//STM自身地址，不与从器件地址相同
	I2C_Init(I2C2, &I2C_InitStructure);
	
	I2C_Cmd(I2C2, ENABLE);
	I2C_AcknowledgeConfig(I2C2, ENABLE);
}

void inti_I2C2(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
		
	RCC_AHBPeriphClockCmd	(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
//	GPIO_PinAFConfig(GPIOB, GPIO_Pin_10, GPIO_AF_I2C2);
//	GPIO_PinAFConfig(GPIOB, GPIO_Pin_11, GPIO_AF_I2C2);
	GPIOB->AFR[1] |= 0x00000400;
	GPIOB->AFR[1] |= 0x00004000;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
		
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
		
	init_I2C_InitStructure();
}

uint32_t I2CTimeout;
#define WIAT_FOR_FLAG(flag, value, timeout, errorcode) I2CTimeout = timeout;\
																												while(I2C_CheckEvent(I2C2,flag) != value )\
																												{	if((I2CTimeout--) == 0)\
																													return I2Cx_TIMEOUT_UserCallback(errorcode);}
/*************************************************************
*************************************************************/

static uint8_t I2Cx_TIMEOUT_UserCallback(uint8_t errorcode){
	
	I2C_GenerateSTOP(I2C2, ENABLE);
	I2C_SoftwareResetCmd(I2C2, ENABLE);
	I2C_SoftwareResetCmd(I2C2, DISABLE);
	
	init_I2C_InitStructure();
		
	return 1;
}

#define WAIT_FLAG_TIME 200
#define LONG_WAIT_FLGA_TIME (10*WAIT_FLAG_TIME)

static uint8_t Convert_DHT12(struct dht12_Reg_Data* reg_data)
{	
	uint8_t id;
	uint8_t point[4];
	//------WRITE Operation-----
	WIAT_FOR_FLAG(I2C_FLAG_BUSY, RESET,LONG_WAIT_FLGA_TIME,1);

	// Start
	I2C_GenerateSTART(I2C2, ENABLE); 

	//Test On EV5 and clear it
	WIAT_FOR_FLAG(I2C_EVENT_MASTER_MODE_SELECT, SET,WAIT_FLAG_TIME,2);

	//Dev SEL  
	I2C_Send7bitAddress(I2C2, DHT12_ADDR, I2C_Direction_Transmitter);
	WIAT_FOR_FLAG(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED, SET,LONG_WAIT_FLGA_TIME,3);

	//Send Byte Addr
	I2C_SendData(I2C2, REG_HUMIDITY_HIGH);
	WIAT_FOR_FLAG(I2C_EVENT_MASTER_BYTE_TRANSMITTED, SET,LONG_WAIT_FLGA_TIME,4);
	
	//-------READ Operation------
	I2C_GenerateSTART(I2C2, ENABLE);       //主机发送起始信号	
	WIAT_FOR_FLAG(I2C_EVENT_MASTER_MODE_SELECT, SET,WAIT_FLAG_TIME,5);	

	//发送读指令
	I2C_Send7bitAddress(I2C2, DHT12_ADDR, I2C_Direction_Receiver);
	// Read Data	
	for( id = 0;id < 4;id++)
	{
		WIAT_FOR_FLAG(I2C_EVENT_MASTER_BYTE_RECEIVED, SET,LONG_WAIT_FLGA_TIME,6);
		point[id] = I2C_ReceiveData(I2C2);
	}
	I2C_AcknowledgeConfig(I2C2, DISABLE);
	WIAT_FOR_FLAG(I2C_EVENT_MASTER_BYTE_RECEIVED, SET,LONG_WAIT_FLGA_TIME,6);	
	reg_data->cal_value = I2C_ReceiveData(I2C2);
	
	reg_data->humi_high 			 = point[0];
	reg_data->humi_low  			 = point[1];
	reg_data->temperature_high = point[2];
	reg_data->temperature_low  = point[3];
	
	I2C_GenerateSTOP(I2C2, ENABLE);	        
	I2C_AcknowledgeConfig(I2C2, ENABLE);
	return 0;
}

uint8_t getDHT12_Temp_Humi(float* temperature, float* huminity)
{
	uint8_t temp_cal = 0;
	float getTemp = 0.0, getHumi = 0.0;
	struct dht12_Reg_Data data;
	
	Convert_DHT12(&data);

	temp_cal = (uint8_t)(data.humi_high + data.humi_low + data.temperature_high + data.temperature_low);//只取低8位
		
	if(data.cal_value == temp_cal)																																			//如果校验成功，往下运行
	{
		getHumi = data.humi_high*10 + data.humi_low; 																											//湿度
		if(data.temperature_low&0X80)																																			//为负温度
		{
			getTemp = 0 - (data.temperature_high*10 + ((data.temperature_low&0x7F)));
		}
		else   																																														//为正温度
		{
			getTemp=data.temperature_high*10+data.temperature_low;																					//为正温度
		} 	
		//判断温湿度是否超过测量范围（温度：-20℃~60摄氏度；湿度：20%RH~95%RH）		
		if(getHumi > 950) 
		{
			getHumi = 950;
		}
		else if(getHumi < 200)
		{
			getHumi = 200;
		}
    // 湿度范围
		if(getTemp > 600)
		{ 
			getTemp = 600;
		}
		else if(getTemp < -200)
		{
			getTemp = -200;
		}
		*temperature = getTemp/10.0;
		*huminity = getHumi/10.0;		
	}	
	else		
	{
		return 1;
	}
	return 0;
}


