/*********************************************************
 * @description: ����1��2��3��4��5��ʼ��
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
uint8_t USART1_RxState = 0;//״̬�������־λ
uint16_t rx1_cnt;
uint8_t rx1_buf[1024];
uint8_t WIFI_UpLineFlag = 0;
_Bool WIFI_CommandFlag = 0;

uint8_t USART2_RxFlag;
uint8_t USART2_RxState = 0;//״̬�������־λ
uint16_t rx2_cnt;
uint8_t rx2_buf[1024];

uint8_t USART3_RxFlag;
uint8_t USART3_RxState = 0;//״̬�������־λ
uint16_t rx3_cnt;
uint8_t rx3_buf[1024];

uint8_t UART4_RxFlag;
uint8_t USART4_RxState = 0;//״̬�������־λ
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
			dataToWrite[0] = (uint16_t)(Buad_Tab[i] & 0xFFFF);         // ��� 16 λ
			dataToWrite[1] = (uint16_t)((Buad_Tab[i] >> 16) & 0xFFFF);  // �� 16 λ
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
  //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��

	//USART1_TX   GPIOA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
	//USART1_RX	  GPIOA.10��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

	USART_Init(USART1, &USART_InitStructure); //��ʼ������1
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//���������ж�
	USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 
}

/*********************************************************
 * @brief USART2��ʼ��
 * @param {uint32_t} bound ������
 * @return None
 *********************************************************/
void USART2_Init(uint32_t bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOE | RCC_APB2Periph_AFIO, ENABLE);//ʹ��GPIOA,Dʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//ʹ��USART2ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;				 //PE7 RS485R/W_1
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	USART_DeInit(USART2);  //��λ����2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;	//PA2 TXD2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA3 RXD2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //
	GPIO_Init(GPIOA, &GPIO_InitStructure);  

	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_Param_Arr.WordLength_Tab[USART2_Config.USART_Param_Index[0]];//8λ���ݳ���
	USART_InitStructure.USART_StopBits = USART_Param_Arr.StopBits_Tab[USART2_Config.USART_Param_Index[1]];//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Param_Arr.Parity_Tab[USART2_Config.USART_Param_Index[2]];///��żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_Param_Arr.HardwareFlowControl_Tab[USART2_Config.USART_Param_Index[3]];//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure); ; //��ʼ������
  
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn; //ʹ�ܴ���2�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; //�����ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure); //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
 
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);//���������ж�
	USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 
}

/*********************************************************
 * @brief USART3��ʼ��
 * @param {uint32_t} bound ������
 * @return None
 *********************************************************/
void USART3_Init(uint32_t bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE  | RCC_APB2Periph_AFIO, ENABLE);//ʹ��GPIOA,Dʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);//ʹ��USART3ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 //PE8 RS485R/W_2
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOE, &GPIO_InitStructure);
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	//PB10 TXD3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PB11 RXD3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);  

	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_Param_Arr.WordLength_Tab[USART3_Config.USART_Param_Index[0]];//8λ���ݳ���
	USART_InitStructure.USART_StopBits = USART_Param_Arr.StopBits_Tab[USART3_Config.USART_Param_Index[1]];//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Param_Arr.Parity_Tab[USART3_Config.USART_Param_Index[2]];///��żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_Param_Arr.HardwareFlowControl_Tab[USART3_Config.USART_Param_Index[3]];//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�շ�ģʽ
	USART_Init(USART3, &USART_InitStructure); //��ʼ������
  
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn; //ʹ�ܴ���2�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; //�����ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure); //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
 
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�����ж�
	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//���������ж�
	USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ��� 
}

/*********************************************************
 * @brief USART4��ʼ��
 * @param {uint32_t} bound ������
 * @return None
 *********************************************************/
void UART4_Init(uint32_t bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOE  | RCC_APB2Periph_AFIO, ENABLE);//ʹ��GPIOA,Dʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);//ʹ��USART3ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 //PE9 RS485R/W_3
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOE, &GPIO_InitStructure);
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;	//PC10 TXD4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PC11 RXD4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);  

	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_Param_Arr.WordLength_Tab[USART3_Config.USART_Param_Index[0]];//8λ���ݳ���
	USART_InitStructure.USART_StopBits = USART_Param_Arr.StopBits_Tab[USART3_Config.USART_Param_Index[1]];//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Param_Arr.Parity_Tab[USART3_Config.USART_Param_Index[2]];///��żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_Param_Arr.HardwareFlowControl_Tab[USART3_Config.USART_Param_Index[3]];//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�շ�ģʽ
	USART_Init(UART4, &USART_InitStructure);  //��ʼ������
  
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn; //ʹ�ܴ���2�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2; //�����ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure); //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
 
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//�����ж�
	USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);//���������ж�
	USART_Cmd(UART4, ENABLE);                    //ʹ�ܴ��� 
}

/*********************************************************
 * @brief UART5��ʼ��
 * @param {uint32_t} bound ������
 * @return None
 *********************************************************/
void UART5_Init(uint32_t bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOD  | RCC_APB2Periph_AFIO, ENABLE);//ʹ��GPIOA,Dʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5,ENABLE);//ʹ��UART5ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;	//PC12 TXD5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;//PD2 RXD5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //��������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);  

	USART_InitStructure.USART_BaudRate = bound;//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//8λ���ݳ���
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;///��żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//�շ�ģʽ
	USART_Init(UART5, &USART_InitStructure); ; //��ʼ������
  
	NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn; //ʹ�ܴ���2�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3; //�����ȼ�2��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure); //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
 
	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);//�����ж�
	USART_Cmd(UART5, ENABLE);                    //ʹ�ܴ��� 
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
 * @brief USARTx�����ֽ�
 * @param {USART_TypeDef*} x ���ں�
 * @param {uint8_t} Byte Ҫ���͵��ֽ�
 * @return None
 *********************************************************/
void USARTx_SendByte(USART_TypeDef* x, uint8_t Byte)
{
	USART_SendData(x, Byte);
	while (USART_GetFlagStatus(x, USART_FLAG_TXE) == RESET);
}

/*********************************************************
 * @brief : USARTx��������
 * @param {USART_TypeDef*} x ���ں�
 * @param {uint8_t} *Array Ҫ���͵�����
 * @param {uint16_t} Length ���鳤��
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
 * @brief USARTx�����ַ���
 * @param {USART_TypeDef*} x ���ں�
 * @param {char} *String Ҫ���͵��ַ���
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
 * @brief USARTx��������
 * @param {USART_TypeDef*} x ���ں�
 * @param {uint32_t} Number ��Ҫ���͵�����
 * @param {uint8_t} Length ���ֳ���
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
