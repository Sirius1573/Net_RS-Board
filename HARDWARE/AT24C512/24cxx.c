/*********************************************************
 * @description: 该文件包含24C512相关函数
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2023-04-12 13:54:59
 *********************************************************/

#include "24cxx.h" 
#include "delay.h"
 
//初始化IIC
void IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_SetBits(GPIOD,GPIO_Pin_14|GPIO_Pin_15); 	//PD14,PD15 输出高
}
//产生IIC起始信号
void IIC_Start(void)
{
	SDA_OUT();//sda线输出
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	Delay_us(4);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	Delay_us(4);
	IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	Delay_us(4);
	IIC_SCL=1; 
	IIC_SDA=1;//发送I2C总线结束信号
	Delay_us(4);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
	IIC_SDA=1;Delay_us(1);	   
	IIC_SCL=1;Delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL=0;//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void IIC_Ack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	Delay_us(2);
	IIC_SCL=1;
	Delay_us(2);
	IIC_SCL=0;
}
//不产生ACK应答		    
void IIC_NAck(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	Delay_us(2);
	IIC_SCL=1;
	Delay_us(2);
	IIC_SCL=0;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(u8 txd)
{                        
  u8 t;   
	SDA_OUT(); 	    
  IIC_SCL=0;//拉低时钟开始数据传输
  for(t=0;t<8;t++)
  {              
        //IIC_SDA=(txd&0x80)>>7;
		if((txd&0x80)>>7)
			IIC_SDA=1;
		else
			IIC_SDA=0;
		txd<<=1; 	  
		Delay_us(2);   //对TEA5767这三个延时都是必须的
		IIC_SCL=1;
		Delay_us(2); 
		IIC_SCL=0;	
		Delay_us(2);
  }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
  for(i=0;i<8;i++ )
	{
    IIC_SCL=0; 
    Delay_us(2);
		IIC_SCL=1;
    receive<<=1;
    if(READ_SDA)receive++;   
		Delay_us(1); 
  }					 
  if (!ack)
    IIC_NAck();//发送nACK
  else
    IIC_Ack(); //发送ACK   
  return receive;
}


//初始化IIC接口
void AT24CXX_Init(void)
{
	IIC_Init();
}
//在AT24CXX指定地址读出一个数据
//ReadAddr:开始读数的地址  
//返回值  :读到的数据
u8 AT24CXX_ReadOneByte(u16 ReadAddr)
{
  u8 temp=0;		  	    																 
	IIC_Start();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	   //发送写命令
		IIC_Wait_Ack();
		IIC_Send_Byte(ReadAddr>>8);//发送高地址
		IIC_Wait_Ack();		 			//这句也得考虑注释掉
	}
	else
	  IIC_Send_Byte(0XA0+((ReadAddr/256)<<1));   //发送器件地址0XA0,写数据 	 
 
		//IIC_Wait_Ack(); 						//这句去掉就OK了，就可以通过AT24CXX_Check()了
	IIC_Send_Byte(ReadAddr%256);   //发送低地址
	IIC_Wait_Ack();	    
	IIC_Start();  	 	   
	IIC_Send_Byte(0XA1);           //进入接收模式			   
	IIC_Wait_Ack();	 
	temp=IIC_Read_Byte(0);		   
	IIC_Stop();//产生一个停止条件	    
	return temp;
}

//在AT24CXX指定地址写入一个数据
//WriteAddr  :写入数据的目的地址    
//DataToWrite:要写入的数据
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
{
	IIC_Start();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	    //发送写命令
		IIC_Wait_Ack();
		IIC_Send_Byte(WriteAddr>>8);//发送高地址
	}
	else
	{
		IIC_Send_Byte(0XA0+((WriteAddr/256)<<1));   //发送器件地址0XA0,写数据 
	}	 
	IIC_Wait_Ack();	   
	IIC_Send_Byte(WriteAddr%256);   //发送低地址
	IIC_Wait_Ack(); 	 										  		   
	IIC_Send_Byte(DataToWrite);     //发送字节							   
	IIC_Wait_Ack();  		    	   
	IIC_Stop();//产生一个停止条件 
	Delay_ms(10); 
}

//在AT24CXX里面的指定地址开始写入长度为Len的数据
//该函数用于写入16bit或者32bit的数据.
//WriteAddr  :开始写入的地址  
//DataToWrite:数据数组首地址
//Len        :要写入数据的长度2,4
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
{
	u8 t;
	for(t=0;t<Len;t++)
	{
		AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
	}												    
}



//在AT24CXX里面的指定地址开始读出长度为Len的数据
//该函数用于读出16bit或者32bit的数据.
//ReadAddr   :开始读出的地址 
//返回值     :数据
//Len        :要读出数据的长度2,4
u32 AT24C512_ReadLenByte(u16 ReadAddr,u8 Len)
{
	u8 t;
	u32 temp=0;
	for(t=0;t<Len;t++)
	{
		temp<<=8;
		temp+=AT24CXX_ReadOneByte(ReadAddr+Len-t-1); 	 				   
	}
	return temp;												    
}

//检查AT24CXX是否正常
//这里用了24XX的最后一个地址(255)来存储标志字.
//如果用其他24C系列,这个地址要修改
//返回1:检测失败
//返回0:检测成功
u8 AT24CXX_Check(void)
{
	u8 temp1;
	temp1=AT24CXX_ReadOneByte(255);//避免每次开机都写AT24CXX			   
	if(temp1==0X55)return 0;		   
	else//排除第一次初始化的情况
	{
		AT24CXX_WriteOneByte(255,0X55);
	    temp1=AT24CXX_ReadOneByte(255);	  
		if(temp1==0X55)return 0;
	}
	return 1;											  
}


//在AT24CXX里面的指定地址开始读出指定个数的数据
//ReadAddr :开始读出的地址 对24c02为0~255
//pBuffer  :数据数组首地址
//NumToRead:要读出数据的个数
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=AT24CXX_ReadOneByte(ReadAddr++);	
		NumToRead--;
	}
}

//在AT24CXX里面的指定地址开始写入指定个数的数据
//WriteAddr :开始写入的地址 对24c02为0~255
//pBuffer   :数据数组首地址
//NumToWrite:要写入数据的个数
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite)
{
	while(NumToWrite--)
	{
		AT24CXX_WriteOneByte(WriteAddr,*pBuffer);
		WriteAddr++;
		pBuffer++;
	}
}

/*********************************************************
 * @brief AT24C512写入n个16bit数据
 * @param {u16} WriteAddr 开始写入的地址
 * @param {u32} DataToWrite 数据数组首地址
 * @param {u16} NumToWrite 写入数据的个数
 * @return None
 *********************************************************/
void AT24C512_Write2Byte(u16 WriteAddr,u16 *DataToWrite,u16 NumToWrite)
{
    while(NumToWrite--)
	{
		u8 t;
        for(t=0;t<2;t++)
        {
            AT24CXX_WriteOneByte(WriteAddr+t,(*DataToWrite>>(8*t))&0xff);
        }
		WriteAddr += 2;
        DataToWrite++;
    }
}

/*********************************************************
 * @brief AT24C512读出n个16bit数据
 * @param {u16} ReadAddr 开始读出的首地址
 * @param {u8} *pBuffer  存放数据数组首地址
 * @param {u16} NumToRead   要读出数据的个数
 * @return None
 *********************************************************/
void AT24C512_Read2Byte(u16 ReadAddr,u16 *pBuffer,u16 NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=AT24C512_ReadLenByte(ReadAddr,2);
        ReadAddr +=2;
        NumToRead--;
	}
}
