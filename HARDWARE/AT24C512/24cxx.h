/*********************************************************
 * @description: 
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2023-04-12 13:56:55
 *********************************************************/
#ifndef __24CXX_H
#define __24CXX_H
#include "sys.h"   

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767
#define AT24C512    65535
//ʹ�õ���24c512�����Զ���EE_TYPEΪAT24C512
#define EE_TYPE AT24C512


//IO��������
#define SDA_IN()  {GPIOD->CRL&=0XFFFFFFF0;GPIOD->CRL|=8<<0;}
#define SDA_OUT() {GPIOD->CRL&=0XFFFFFFF0;GPIOD->CRL|=3<<0;}

//IO��������	 
#define IIC_SCL    PDout(15) //SCL
#define IIC_SDA    PDout(14) //SDA
#define READ_SDA   PDin(14)  //����SDA

//IIC���в�������
void IIC_Init(void);                 //��ʼ��IIC��IO��				 
void IIC_Start(void);				 //����IIC��ʼ�ź�
void IIC_Stop(void);	  			 //����IICֹͣ�ź�
void IIC_Send_Byte(u8 txd);			 //IIC����һ���ֽ�
u8 IIC_Read_Byte(unsigned char ack); //IIC��ȡһ���ֽ�
u8 IIC_Wait_Ack(void); 				 //IIC�ȴ�ACK�ź�
void IIC_Ack(void);					 //IIC����ACK�ź�
void IIC_NAck(void);				 //IIC������ACK�ź�

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	  
u8 AT24CXX_ReadOneByte(u16 ReadAddr);							 //ָ����ַ��ȡһ���ֽ�
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite);		 //ָ����ַд��һ���ֽ�
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len); //ָ����ַ��ʼд��ָ�����ȵ�����
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len);					 //ָ����ַ��ʼ��ȡָ����������
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite);	 //��ָ����ַ��ʼд��ָ�����ȵ�����
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead);   	 //��ָ����ַ��ʼ����ָ�����ȵ�����
void AT24C512_Write2Byte(u16 WriteAddr, u16 *DataToWrite, u16 NumToWrite); 
void AT24C512_Read2Byte(u16 ReadAddr, u16 *pBuffer, u16 NumToRead);

u8 AT24CXX_Check(void);  //�������
void AT24CXX_Init(void); //��ʼ��IIC

#endif
