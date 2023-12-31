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
//使用的是24c512，所以定义EE_TYPE为AT24C512
#define EE_TYPE AT24C512


//IO方向设置
#define SDA_IN()  {GPIOD->CRL&=0XFFFFFFF0;GPIOD->CRL|=8<<0;}
#define SDA_OUT() {GPIOD->CRL&=0XFFFFFFF0;GPIOD->CRL|=3<<0;}

//IO操作函数	 
#define IIC_SCL    PDout(15) //SCL
#define IIC_SDA    PDout(14) //SDA
#define READ_SDA   PDin(14)  //输入SDA

//IIC所有操作函数
void IIC_Init(void);                 //初始化IIC的IO口				 
void IIC_Start(void);				 //发送IIC开始信号
void IIC_Stop(void);	  			 //发送IIC停止信号
void IIC_Send_Byte(u8 txd);			 //IIC发送一个字节
u8 IIC_Read_Byte(unsigned char ack); //IIC读取一个字节
u8 IIC_Wait_Ack(void); 				 //IIC等待ACK信号
void IIC_Ack(void);					 //IIC发送ACK信号
void IIC_NAck(void);				 //IIC不发送ACK信号

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	  
u8 AT24CXX_ReadOneByte(u16 ReadAddr);							 //指定地址读取一个字节
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite);		 //指定地址写入一个字节
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len); //指定地址开始写入指定长度的数据
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len);					 //指定地址开始读取指定长度数据
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite);	 //从指定地址开始写入指定长度的数据
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead);   	 //从指定地址开始读出指定长度的数据
void AT24C512_Write2Byte(u16 WriteAddr, u16 *DataToWrite, u16 NumToWrite); 
void AT24C512_Read2Byte(u16 ReadAddr, u16 *pBuffer, u16 NumToRead);

u8 AT24CXX_Check(void);  //检查器件
void AT24CXX_Init(void); //初始化IIC

#endif
