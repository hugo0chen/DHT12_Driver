#include "dht12.h"
#include "i2c.h"
#include "delay.h"
#include "stdio.h"

/*************************************************************
  Function   :Init_dht12
  Description:初始化dht12 
  Input      : none        
  return     : none    
*************************************************************/
void Init_DHT12(void)
{
	I2Cx_Init();
}
/*************************************************************
  Function   : Read_dht12
  Description: dht12读光照值
  Input      :  none
  return     : ---读回的数据  uint16_t    
*************************************************************/
uint8_t Read_DHT12(uint8_t reg)
{   	
	uint8_t data;

	I2C_Start();                      //起始信号
	if(I2C_WaiteForAck())
	{
		I2C_WriteByte(DHT12_ADDR);        //发送设备地址+读信号
		I2C_WaiteForAck();
		I2C_WriteByte(0);
		I2C_WaiteForAck();
		I2C_WriteByte(DHT12_ADDR+reg); 
		I2C_WaiteForAck();
		data = I2C_ReadByte(1); 					//NO_ACK
		I2C_Stop();                       //停止信号
	}
	return data;	
}
/*************************************************************
*************************************************************/
uint8_t Convert_DHT12(float* Temprature, float* Humi)
{
	uint8_t Humi_H,Humi_L;
	uint8_t Temp_H,Temp_L,Temp_CAL;
	uint8_t temp;
	float getTemp, getHumi;
	uint16_t i;
	
	I2C_Start();   
	I2C_WriteByte(DHT12_ADDR);    //发送IIC地址
	if(!I2C_WaiteForAck())  			//等待从机应答信号（如无应答：考虑IIC通讯频率是否太快，或者传感器接线错误）
	{
		i=0;
		I2C_WriteByte(0);
		while(I2C_WaiteForAck())//等待从机应答信号
		{
		 	if(++i >= 500)
			{
			 	break;
			}		 
		} 
		i=0;
		I2C_Start();       //主机发送起始信号
		I2C_WriteByte(DHT12_ADDR+1);     //发送读指令
		while(I2C_WaiteForAck())    //等待从机应答信号
		{
		 	if(++i>=500)
			{
			 	break;
			}			 
		}
		Humi_H=I2C_ReadByte(1);   //读取湿度高位
		Humi_L=I2C_ReadByte(1);   //读取湿度低位
		Temp_H=I2C_ReadByte(1);   //读取温度高位
		Temp_L=I2C_ReadByte(1);   //读取温度低位
		Temp_CAL=I2C_ReadByte(0); //读取校验位
		I2C_Stop();	   //发送停止信号	
		temp = (uint8_t)(Humi_H+Humi_L+Temp_H+Temp_L);//只取低8位
		
		if(Temp_CAL==temp)//如果校验成功，往下运行
		{
			getHumi=Humi_H*10+Humi_L; //湿度
	
			if(Temp_L&0X80)	//为负温度
			{
				getTemp =0-(Temp_H*10+((Temp_L&0x7F)));
			}
			else   //为正温度
			{
				getTemp=Temp_H*10+Temp_L;//为正温度
			} 	
			//判断温湿度是否超过测量范围（温度：-20℃~60摄氏度；湿度：20%RH~95%RH）		
			if(getHumi>950) 
			{
			  getHumi=950;
			}
			if(getHumi<200)
			{
				getHumi =200;
			}
			if(getTemp>600)
			{
			  getTemp=600;
			}
			if(getTemp<-200)
			{
				getTemp = -200;
			}
			getTemp = getTemp/10.0;
			getHumi=getHumi/10.0; 
			*Temprature = getTemp;
			*Humi = getHumi;
		}
		else //校验失败
		{
			printf("CRC Error !!\r\n");
		}
	}else
	{
	   printf("Sensor Error !!\r\n");
	}	
	return 0;
}


