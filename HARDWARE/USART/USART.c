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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

uint8_t USART2_RxFlag;
uint8_t USART3_RxFlag;
uint8_t UART4_RxFlag;
uint8_t UART5_RxFlag;
uint8_t rx1_cnt;
uint8_t rx1_buf[50];
uint8_t rx2_cnt;
uint8_t rx2_buf[50];
uint8_t rx3_cnt;
uint8_t rx3_buf[50];
uint8_t rx4_cnt;
uint8_t rx4_buf[50];
uint8_t rx5_cnt;
uint8_t rx5_buf[50];
uint8_t buf[20];
uint8_t USART_Channel = 66;
uint8_t startup_flag = 0;

void USART1_Init(uint32_t bound)
{
  //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART1，GPIOA时钟
  
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
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
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
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOE  | RCC_APB2Periph_AFIO, ENABLE);//使能GPIOA,D时钟
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
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8位数据长度
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;///奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
	USART_Init(USART2, &USART_InitStructure); ; //初始化串口
  
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; //使能串口2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
 
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
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
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8位数据长度
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;///奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
	USART_Init(USART3, &USART_InitStructure); ; //初始化串口
  
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn; //使能串口2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; //从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
 
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断
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
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8位数据长度
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;///奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
	USART_Init(UART4, &USART_InitStructure); ; //初始化串口
  
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn; //使能串口2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //先占优先级2级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; //从优先级2级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //使能外部中断通道
	NVIC_Init(&NVIC_InitStructure); //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
 
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//开启中断
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
    if(USART_GetITStatus(USART1,USART_IT_RXNE) == SET)
    {
        USART_Channel = 1;
        rx1_cnt++;
		rx1_buf[rx1_cnt] = USART_ReceiveData(USART1);
        if((strstr((const char *)rx1_buf+1, "funcRSStartUp")))
        {	
			startup_flag = 1;
            USART_Channel = 0;
			rx1_cnt=0;
			memset(rx1_buf,0,sizeof(rx1_buf));
        }
		if(startup_flag)
		{
			funcRS_StartLearn(rx1_buf);
			if(rx1_buf[rx1_cnt] == '/')
			{
				USART_Channel = 0;
				sprintf((char *)buf, "funcRSreply\r\n");
				USARTx_SendArray(USART1, buf, 16);
				rx1_cnt=0;
				/***********func***********/
				funcRS_OCEPCtrl(rx1_buf);
				funcRS_StarSend(rx1_buf);
				/**************************/
				memset(rx1_buf,0,sizeof(rx1_buf));
			}
		}
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

void USART2_IRQHandler(void)
{
    if(USART_GetITStatus(USART2,USART_IT_RXNE) == SET)
    {
        USART_Channel = 2;
        rx2_cnt++;
		rx2_buf[rx2_cnt] = USART_ReceiveData(USART2);
        if((strstr((const char *)rx2_buf+1, "funcRSStartUp")))
		{
			startup_flag = 1;
            USART_Channel = 0;
			rx2_cnt=0;
			memset(rx2_buf,0,sizeof(rx2_buf));
        }
		if(startup_flag)
		{
			funcRS_StartLearn(rx2_buf);
			if(rx2_buf[rx2_cnt] == '/')
			{
				USART_Channel = 0;
				RS485rwack_1 = 1;
				sprintf((char *)buf, "funcRSreply\r\n");
				USARTx_SendArray(USART2, buf, 16);
				rx2_cnt=0;
				/***********func***********/
				funcRS_OCEPCtrl(rx2_buf);
				funcRS_StarSend(rx2_buf);
				/**************************/
				memset(rx2_buf,0,sizeof(rx2_buf));
				RS485rwack_1 = 0;
			}
		}
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

void USART3_IRQHandler(void)
{
    if(USART_GetITStatus(USART3,USART_IT_RXNE) == SET)
    {
        USART_Channel = 3;
        rx3_cnt++;
		rx3_buf[rx3_cnt] = USART_ReceiveData(USART3);
        if((strstr((const char *)rx3_buf+1, "funcRSStartUp")))
		{
			startup_flag = 1;
            USART_Channel = 0;
			rx3_cnt=0;
			memset(rx3_buf,0,sizeof(rx3_buf));
        }
		if(startup_flag)
		{
			funcRS_StartLearn(rx3_buf);
			if(rx3_buf[rx3_cnt] == '/')
			{
				USART_Channel = 0;
				RS485rwack_2 = 1;
				sprintf((char *)buf, "funcRSreply\r\n");
				USARTx_SendArray(USART3, buf, 16);
				rx3_cnt=0;
				/***********func***********/
				funcRS_StarSend(rx3_buf);
				funcRS_OCEPCtrl(rx3_buf);
				/**************************/
				memset(rx3_buf,0,sizeof(rx3_buf));
				RS485rwack_2 = 0;
			}
		}
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}

void UART4_IRQHandler(void)
{
    if(USART_GetITStatus(UART4,USART_IT_RXNE) == SET)
    {
        USART_Channel = 4;
        rx4_cnt++;
		rx4_buf[rx4_cnt] = USART_ReceiveData(UART4);
        if((strstr((const char *)rx4_buf+1, "funcRSStartUp")))
		{
			startup_flag = 1;
            USART_Channel = 0;
			rx4_cnt=0;
			memset(rx4_buf,0,sizeof(rx4_buf));
        }
		if(startup_flag)
		{
			funcRS_StartLearn(rx4_buf);
			if(rx4_buf[rx4_cnt] == '/')
			{
				USART_Channel = 0;
				RS485rwack_3 = 1;
				sprintf((char *)buf, "funcRSreply\r\n");
				USARTx_SendArray(UART4, buf, 16);
				rx4_cnt=0;
				/***********func***********/
				funcRS_StarSend(rx4_buf);
				funcRS_OCEPCtrl(rx4_buf);
				/**************************/
				memset(rx4_buf,0,sizeof(rx4_buf));
				RS485rwack_3 = 0;
			}
		}
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
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
			rx5_cnt=0;
			funcRS_OCEPCtrl(rx5_buf);
			memset(rx5_buf,0,sizeof(rx5_buf));
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


