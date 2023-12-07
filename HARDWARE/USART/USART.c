/*********************************************************
 * @description: 串口1、2、3、4、5初始化
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2023-06-06 11:29:19
 *********************************************************/
#include "stm32f10x.h"
#include "USART.h"
#include "sys.h"
#include "delay.h"
#include "Remote.h"
#include "W5500.h"
#include "DataPackage.h"
#include "ESP01.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "24cxx.h"

uint8_t USART1_RxFlag;
uint8_t USART1_RxState = 0;//状态机步骤标志位
uint16_t rx1_cnt;
uint8_t rx1_buf[1024];
uint8_t WIFI_UpLineFlag = 0;
_Bool WIFI_CommandFlag = 0;

uint8_t USART2_RxFlag;
uint8_t USART2_RxState = 0;//状态机步骤标志位
uint16_t rx2_cnt;
uint8_t rx2_buf[1024];

uint8_t USART3_RxFlag;
uint8_t USART3_RxState = 0;//状态机步骤标志位
uint16_t rx3_cnt;
uint8_t rx3_buf[1024];

uint8_t UART4_RxFlag;
uint8_t USART4_RxState = 0;//状态机步骤标志位
uint16_t rx4_cnt;
uint8_t rx4_buf[1024];

uint8_t UART5_RxFlag;
uint8_t rx5_cnt;
uint8_t rx5_buf[100];

uint8_t buf[20];
uint8_t USART_Channel = 0;
uint8_t U_Channel;
uint8_t startup_flag = 0;
uint32_t Buad_Tab[3] = { 9600,9600,9600};

USART_Param_Tab USART_Param_Arr;
USART_Param_Index USART2_Config = { 0,0,0,0 };
USART_Param_Index USART3_Config = { 0,0,0,0 };
USART_Param_Index UART4_Config = { 0,0,0,0 };

void Load_USART_Param(void)
{
	unsigned char eeprom_test[2];
	uint8_t i;
	uint16_t dataToWrite[2];

	AT24CXX_Read(55, eeprom_test, 2);

	if (eeprom_test[0] == 0xff && eeprom_test[1] == 0xff)
	{
		AT24CXX_Write(55, USART2_Config.USART_Param_Index, 4);
		AT24CXX_Write(59, USART3_Config.USART_Param_Index, 4);
		AT24CXX_Write(63, UART4_Config.USART_Param_Index, 4);
		for (i = 0;i <= 2;i++)
		{
			dataToWrite[0] = (uint16_t)(Buad_Tab[i] & 0xFFFF);         // 最低 16 位
			dataToWrite[1] = (uint16_t)((Buad_Tab[i] >> 16) & 0xFFFF);  // 高 16 位
			AT24C512_Write2Byte(67 + 4 * i, dataToWrite, 2);
		}
			
	}

	AT24CXX_Read(55, USART2_Config.USART_Param_Index, 4);
	AT24CXX_Read(59, USART3_Config.USART_Param_Index, 4);
	AT24CXX_Read(63, UART4_Config.USART_Param_Index, 4);
	for (i = 0;i <= 2;i++)
	{
		AT24C512_Read2Byte(67 + 4 * i, dataToWrite, 2);
		Buad_Tab[i] = ((uint32_t)dataToWrite[1] << 16) | (uint32_t)dataToWrite[0];
	}
	//	AT24C512_Read2Byte(67, Buad_Tab, 4);

	USART_Param_Arr.WordLength_Tab[0] = USART_WordLength_8b;
	USART_Param_Arr.WordLength_Tab[1] = USART_WordLength_9b;

	USART_Param_Arr.StopBits_Tab[0] = USART_StopBits_1;
	USART_Param_Arr.StopBits_Tab[1] = USART_StopBits_0_5;
	USART_Param_Arr.StopBits_Tab[2] = USART_StopBits_2;
	USART_Param_Arr.StopBits_Tab[3] = USART_StopBits_1_5;

	USART_Param_Arr.Parity_Tab[0] = USART_Parity_No;
	USART_Param_Arr.Parity_Tab[1] = USART_Parity_Even;
	USART_Param_Arr.Parity_Tab[2] = USART_Parity_Odd;

	USART_Param_Arr.HardwareFlowControl_Tab[0] = USART_HardwareFlowControl_None;
	USART_Param_Arr.HardwareFlowControl_Tab[1] = USART_HardwareFlowControl_RTS;
	USART_Param_Arr.HardwareFlowControl_Tab[2] = USART_HardwareFlowControl_CTS;
	USART_Param_Arr.HardwareFlowControl_Tab[3] = USART_HardwareFlowControl_RTS_CTS;
}

void USART1_Init(uint32_t bound)
{
  //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟

	//USART1_TX   GPIOA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
  	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.9
   
	//USART1_RX	  GPIOA.10初始化
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.10  

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置
	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART1, &USART_InitStructure); //初始化串口1
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启串口接受中断
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//开启空闲中断
	USART_Cmd(USART1, ENABLE);                    //使能串口1 
}

/*********************************************************
 * @brief USART2初始化
 * @param {uint32_t} bound 波特率
 * @return None
 *********************************************************/
void USART2_Init(uint32_t bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);//使能GPIOA,D时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;				 //PE7 RS485R/W_1
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	USART_DeInit(USART2);  //复位串口2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	//PA2 TXD2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3 RXD2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //
	GPIO_Init(GPIOA, &GPIO_InitStructure);  

	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_Param_Arr.WordLength_Tab[USART2_Config.USART_Param_Index[0]];//8位数据长度
	USART_InitStructure.USART_StopBits = USART_Param_Arr.StopBits_Tab[USART2_Config.USART_Param_Index[1]];//一个停止位
	USART_InitStructure.USART_Parity = USART_Param_Arr.Parity_Tab[USART2_Config.USART_Param_Index[2]];///奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_Param_Arr.HardwareFlowControl_Tab[USART2_Config.USART_Param_Index[3]];//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
	USART_Init(USART2, &USART_InitStructure); ; //初始化串口
  
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; //使能串口2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
 
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//开启空闲中断
	USART_Cmd(USART2, ENABLE);                    //使能串口 
}

/*********************************************************
 * @brief USART3初始化
 * @param {uint32_t} bound 波特率
 * @return None
 *********************************************************/
void USART3_Init(uint32_t bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE  | RCC_APB2Periph_AFIO, ENABLE);//使能GPIOA,D时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//使能USART3时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 //PE8 RS485R/W_2
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOE, &GPIO_InitStructure);
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	//PB10 TXD3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PB11 RXD3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //浮空输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);  

	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_Param_Arr.WordLength_Tab[USART3_Config.USART_Param_Index[0]];//8位数据长度
	USART_InitStructure.USART_StopBits = USART_Param_Arr.StopBits_Tab[USART3_Config.USART_Param_Index[1]];//一个停止位
	USART_InitStructure.USART_Parity = USART_Param_Arr.Parity_Tab[USART3_Config.USART_Param_Index[2]];///奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_Param_Arr.HardwareFlowControl_Tab[USART3_Config.USART_Param_Index[3]];//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
	USART_Init(USART3, &USART_InitStructure); //初始化串口
  
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn; //使能串口2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; //从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
 
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//开启空闲中断
	USART_Cmd(USART3, ENABLE);                    //使能串口 
}

/*********************************************************
 * @brief USART4初始化
 * @param {uint32_t} bound 波特率
 * @return None
 *********************************************************/
void UART4_Init(uint32_t bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOE  | RCC_APB2Periph_AFIO, ENABLE);//使能GPIOA,D时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);//使能USART3时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 //PE9 RS485R/W_3
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOE, &GPIO_InitStructure);
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	//PC10 TXD4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PC11 RXD4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //浮空输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);  

	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_Param_Arr.WordLength_Tab[USART3_Config.USART_Param_Index[0]];//8位数据长度
	USART_InitStructure.USART_StopBits = USART_Param_Arr.StopBits_Tab[USART3_Config.USART_Param_Index[1]];//一个停止位
	USART_InitStructure.USART_Parity = USART_Param_Arr.Parity_Tab[USART3_Config.USART_Param_Index[2]];///奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_Param_Arr.HardwareFlowControl_Tab[USART3_Config.USART_Param_Index[3]];//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
	USART_Init(UART4, &USART_InitStructure);  //初始化串口
  
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn; //使能串口2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; //从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
 
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//开启中断
	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);//开启空闲中断
	USART_Cmd(UART4, ENABLE);                    //使能串口 
}

/*********************************************************
 * @brief UART5初始化
 * @param {uint32_t} bound 波特率
 * @return None
 *********************************************************/
void UART5_Init(uint32_t bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD  | RCC_APB2Periph_AFIO, ENABLE);//使能GPIOA,D时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5,ENABLE);//使能UART5时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	//PC12 TXD5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//PD2 RXD5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //浮空输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);  

	USART_InitStructure.USART_BaudRate = bound;//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8位数据长度
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;///奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
	USART_Init(UART5, &USART_InitStructure); ; //初始化串口
  
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn; //使能串口2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; //从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
 
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//开启中断
	USART_Cmd(UART5, ENABLE);                    //使能串口 
}

void USART1_IRQHandler(void)
{
	uint16_t clear;

	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		rx1_buf[rx1_cnt++] = USART_ReceiveData(USART1);
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
	else if (USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)
	{
		USART1_RxFlag = 1;
		U_Channel = 1;
		if (startup_flag == 0)
		{
			if (strstr((const char*)rx1_buf, "funcWIFIStartUp"))
			{
				USART1_RxFlag = 0;
				WIFI_UpLineFlag = 1;
				memset(rx1_buf, 0, rx1_cnt);
				rx1_cnt = 0;
				
			}
			else
			{	
				memset(rx1_buf, 0, rx1_cnt);
				rx1_cnt = 0;
			}
		}
		clear = USART1->SR;
		clear = USART1->DR;
	}
}

void USART2_IRQHandler(void)
{
	uint16_t clear;
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
	{
		USART_Channel = 2;
		rx2_buf[rx2_cnt++] = USART_ReceiveData(USART2);
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
	else if (USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
	{
		USART2_RxFlag = 1;
		clear = USART2->SR;
		clear = USART2->DR;
	}
}

void USART3_IRQHandler(void)
{

	uint16_t clear;
	if (USART_GetITStatus(USART3, USART_IT_RXNE) == SET)
	{
		USART_Channel = 3;
		rx3_buf[rx3_cnt++] = USART_ReceiveData(USART3);
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
	}
	else if (USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)
	{
		USART3_RxFlag = 4;
		clear = USART3->SR;
		clear = USART3->DR;
	}

}

void UART4_IRQHandler(void)
{
	uint16_t clear;
	if (USART_GetITStatus(UART4, USART_IT_RXNE) == SET)
	{
		USART_Channel = 3;
		rx4_buf[rx4_cnt++] = USART_ReceiveData(UART4);
		USART_ClearITPendingBit(UART4, USART_IT_RXNE);
	}
	else if (USART_GetITStatus(UART4, USART_IT_IDLE) != RESET)
	{
		UART4_RxFlag = 1;
		clear = UART4->SR;
		clear = UART4->DR;
	}
}

void UART5_IRQHandler(void)
{
    if(USART_GetITStatus(UART5,USART_IT_RXNE) == SET)
    {
        UART5_RxFlag = 1;
		rx5_cnt++;
		rx5_buf[rx5_cnt] = USART_ReceiveData(UART5);
		if(rx5_buf[rx5_cnt] == '/')
		{
			// rx5_cnt=0;
			// funcRS_OCEPCtrl(rx5_buf);
			// memset(rx5_buf,0,sizeof(rx5_buf));
		}
        USART_ClearITPendingBit(UART5, USART_IT_RXNE);
    }
}

//*********************************************************


/*********************************************************
 * @brief USARTx发送字节
 * @param {USART_TypeDef*} x 串口号
 * @param {uint8_t} Byte 要发送的字节
 * @return None
 *********************************************************/
void USARTx_SendByte(USART_TypeDef* x, uint8_t Byte)
{
	USART_SendData(x, Byte);
	while (USART_GetFlagStatus(x, USART_FLAG_TXE) == RESET);
}

/*********************************************************
 * @brief : USARTx发送数组
 * @param {USART_TypeDef*} x 串口号
 * @param {uint8_t} *Array 要发送的数组
 * @param {uint16_t} Length 数组长度
 * @return None
 *********************************************************/
void USARTx_SendArray(USART_TypeDef* x, uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)
	{
		USARTx_SendByte(x, Array[i]);
	}
}

/*********************************************************
 * @brief USARTx发送字符串
 * @param {USART_TypeDef*} x 串口号
 * @param {char} *String 要发送的字符串
 * @return None
 *********************************************************/
void USARTx_SendString(USART_TypeDef* x, char *String)
{
	uint16_t i;
	for (i = 0; String[i] != '\0'; i ++)
	{
		USARTx_SendByte(x,String[i]);
	}
}


uint32_t USARTx_Pow(USART_TypeDef* x, uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

/*********************************************************
 * @brief USARTx发送数字
 * @param {USART_TypeDef*} x 串口号
 * @param {uint32_t} Number 需要发送的数字
 * @param {uint8_t} Length 数字长度
 * @return None
 *********************************************************/
void USARTx_SendNumber(USART_TypeDef* x, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		USARTx_SendByte(x,Number / USARTx_Pow(x,10, Length - i - 1) % 10 + '0');
	}
}

int fputc(int ch, FILE *f)
{
	USARTx_SendByte(UART5,ch);
	return 0;
}

void USART5_Printf(char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	USARTx_SendString(UART5,String);
}

void USART_Proce(void)
{
	if (USART1_RxFlag == 1)
	{
		if (WIFI_Config_Flag == 0)
		{
			USARTx_SendString(USART1, "funcWIFIreply\r\n\0");
		}
		
		/****************function*****************/
		funcWIFI_StartLearn(rx1_buf);
		funcWIFI_MesgToRS(rx1_buf);
		funcNet_MesgToUDP(rx1_buf,(uint8_t *)"0");
		funcWIFI_OCEPCtrl(rx1_buf);
		funcWIFI_StarSend(rx1_buf);
		funcNet_SetUARTParam(rx1_buf);
		Get_USARTParam(rx1_buf);
		func_Rest(rx1_buf);
		Set_WIFIParam(Rx_Buffer, rx1_buf);
			
		/****************function*****************/

		
		memset(rx1_buf, 0, rx1_cnt);
		rx1_cnt = 0;
		USART1_RxFlag = 0;
	}

	if (USART2_RxFlag)
	{
		funcNet_MesgToUDP(Rx_Buffer, rx2_buf);
		rx2_cnt = 0;
		USART_Channel = 0;
		memset(rx2_buf, 0, sizeof(rx2_buf));
		USART2_RxFlag = 0;
	}
	else if (USART3_RxFlag)
	{
		funcNet_MesgToUDP(Rx_Buffer, rx3_buf);
		rx3_cnt = 0;
		USART_Channel = 0;
		memset(rx3_buf, 0, sizeof(rx3_buf));
		USART3_RxFlag = 0;
	}
	else if (UART4_RxFlag)
	{
		funcNet_MesgToUDP(Rx_Buffer, rx4_buf);
		rx4_cnt = 0;
		USART_Channel = 0;
		memset(rx4_buf, 0, sizeof(rx4_buf));
		UART4_RxFlag = 0;
	}
	
}
