#include "dht12.h"
#include "i2c.h"
#include "delay.h"
#include "stdio.h"

/*************************************************************
  Function   :Init_dht12
  Description:��ʼ��dht12 
  Input      : none        
  return     : none    
*************************************************************/
void Init_DHT12(void)
{
	I2Cx_Init();
}
/*************************************************************
  Function   : Read_dht12
  Description: dht12������ֵ
  Input      :  none
  return     : ---���ص�����  uint16_t    
*************************************************************/
uint8_t Read_DHT12(uint8_t reg)
{   	
	uint8_t data;

	I2C_Start();                      //��ʼ�ź�
	if(I2C_WaiteForAck())
	{
		I2C_WriteByte(DHT12_ADDR);        //�����豸��ַ+���ź�
		I2C_WaiteForAck();
		I2C_WriteByte(0);
		I2C_WaiteForAck();
		I2C_WriteByte(DHT12_ADDR+reg); 
		I2C_WaiteForAck();
		data = I2C_ReadByte(1); 					//NO_ACK
		I2C_Stop();                       //ֹͣ�ź�
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
	I2C_WriteByte(DHT12_ADDR);    //����IIC��ַ
	if(!I2C_WaiteForAck())  			//�ȴ��ӻ�Ӧ���źţ�����Ӧ�𣺿���IICͨѶƵ���Ƿ�̫�죬���ߴ��������ߴ���
	{
		i=0;
		I2C_WriteByte(0);
		while(I2C_WaiteForAck())//�ȴ��ӻ�Ӧ���ź�
		{
		 	if(++i >= 500)
			{
			 	break;
			}		 
		} 
		i=0;
		I2C_Start();       //����������ʼ�ź�
		I2C_WriteByte(DHT12_ADDR+1);     //���Ͷ�ָ��
		while(I2C_WaiteForAck())    //�ȴ��ӻ�Ӧ���ź�
		{
		 	if(++i>=500)
			{
			 	break;
			}			 
		}
		Humi_H=I2C_ReadByte(1);   //��ȡʪ�ȸ�λ
		Humi_L=I2C_ReadByte(1);   //��ȡʪ�ȵ�λ
		Temp_H=I2C_ReadByte(1);   //��ȡ�¶ȸ�λ
		Temp_L=I2C_ReadByte(1);   //��ȡ�¶ȵ�λ
		Temp_CAL=I2C_ReadByte(0); //��ȡУ��λ
		I2C_Stop();	   //����ֹͣ�ź�	
		temp = (uint8_t)(Humi_H+Humi_L+Temp_H+Temp_L);//ֻȡ��8λ
		
		if(Temp_CAL==temp)//���У��ɹ�����������
		{
			getHumi=Humi_H*10+Humi_L; //ʪ��
	
			if(Temp_L&0X80)	//Ϊ���¶�
			{
				getTemp =0-(Temp_H*10+((Temp_L&0x7F)));
			}
			else   //Ϊ���¶�
			{
				getTemp=Temp_H*10+Temp_L;//Ϊ���¶�
			} 	
			//�ж���ʪ���Ƿ񳬹�������Χ���¶ȣ�-20��~60���϶ȣ�ʪ�ȣ�20%RH~95%RH��		
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
		else //У��ʧ��
		{
			printf("CRC Error !!\r\n");
		}
	}else
	{
	   printf("Sensor Error !!\r\n");
	}	
	return 0;
}


