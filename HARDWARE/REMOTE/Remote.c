/*********************************************************
 * @description:该文件包含红外遥控相关函数
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2023-06-06 21:59:50
 *********************************************************/
#include "Remote.h"
#include "delay.h"
#include "USART.h"
#include "24cxx.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t tx_buf[20];
uint8_t Flag_LearnState = 0;
uint16_t pulseWidth = 0;
uint16_t arr_length = 0;
u16 PulseTab[500];
u16 Last_DataEAddr[1];
u16 Data_FirstAddr[]={120,244,250,9000};
u16 Send_DataTab[500];
u16 test[4];
uint16_t test_num = 0;

int start_addr;
int data_length;
uint8_t channel;
/*********************************************************
 * @brief 红外接收头初始化
 * @return None
 *********************************************************/
void Remote_Reciv_Init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource8);
  	EXTI_InitStructure.EXTI_Line=EXTI_Line8;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	  	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

	//NVIC配置
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//子优先级3
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);

	EXTI->IMR &= ~(EXTI_Line8);
}

void EXTI9_5_IRQHandler(void)
{
	Flag_LearnState = 0;
	//中断指示
	while(1)
	{
		if(IR_RDATA)						//有高脉冲出现，代表空闲信号
		{
			pulseWidth = 0;
			while(IR_RDATA)
			{
				pulseWidth++;
				Delay_us(19);
				if(pulseWidth >= 2500)			// >50ms 则结束记录
					break;
			}
			
			if(pulseWidth<=15 || pulseWidth>=2500)		// >50ms || <300us 则结束记录
				break;
			PulseTab[arr_length] = pulseWidth*20;
			arr_length++;
		}		
		else										//载波信号,偶数位为低电平（载波），奇数位为高电平（空闲）
		{	
			pulseWidth = 0;
			while(IR_RDATA == 0)			
			{
				pulseWidth++;
				Delay_us(19);
			}
			if(pulseWidth<=15 || pulseWidth>=2500) 		// >50ms || <300us  则结束记录
				break;
			PulseTab[arr_length] = pulseWidth*20;
			arr_length++;		
		}
	}
	PulseTab[arr_length++] = pulseWidth;
	PulseTab[arr_length] = 0xffff;
    Flag_LearnState = 1;
	
    if(Flag_LearnState==1)
    {
        uint8_t NetParam_Str[1024];
        uint8_t tx_buf[200];
        Flag_LearnState=0;
		
		AT24C512_Read2Byte(98, Last_DataEAddr, 1);
        AT24C512_Write2Byte(Last_DataEAddr[0] + 1, PulseTab, arr_length);
        sprintf((char *)NetParam_Str, "Learn Finish!\r\nFirst Address:%d\r\nCode Length:%d\r\n", Last_DataEAddr[0]+1, arr_length);
        memcpy(tx_buf, NetParam_Str, strlen((const char*)NetParam_Str));
		
        if(USART_Channel == 1)
        {
            USARTx_SendArray(USART1, tx_buf,  strlen((const char *)NetParam_Str));
		}
		else
		{
			
			Write_SOCK_Data_Buffer(0, tx_buf, strlen((const char*)NetParam_Str));//指定Socket(0~7)发送数据处理 
		}
		Last_DataEAddr[0] += arr_length * 2;
        AT24C512_Write2Byte(98, Last_DataEAddr, 1);
        arr_length = 0;
		
        memset(Send_DataTab, 0, sizeof(Send_DataTab));
    }
    

    EXTI_ClearITPendingBit(EXTI_Line8);
	EXTI->IMR &= ~(EXTI_Line8);

	return;
}


/*********************************************************
 * @brief 红外信号发射头初始化
 * @return None
 *********************************************************/
void Remote_Send1_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);	//使能定时器2时钟
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA  | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOA, &GPIO_InitStructure);					

	//Remote_CH1
	TIM_TimeBaseStructure.TIM_Period = 1895; 
	TIM_TimeBaseStructure.TIM_Prescaler =0; 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
 	TIM_OCInitStructure.TIM_Pulse = 948;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM5, &TIM_OCInitStructure); 
	TIM_OC1PreloadConfig(TIM5, TIM_OCPreload_Enable);
	TIM_Cmd(TIM5, DISABLE);  
	TIM_ForcedOC1Config(TIM5, TIM_ForcedAction_InActive);//PWM强制输出低电平
}

void Remote_Send2_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA  | RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
	GPIO_Init(GPIOA, &GPIO_InitStructure);					


	//Remote_CH2
	TIM_TimeBaseStructure.TIM_Period = 1895; 
	TIM_TimeBaseStructure.TIM_Prescaler =0;  
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; 
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); 
 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 
 	TIM_OCInitStructure.TIM_Pulse = 948;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 
	TIM_OC2Init(TIM2, &TIM_OCInitStructure); 
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);
	TIM_Cmd(TIM2, DISABLE); 
	TIM_ForcedOC2Config(TIM2, TIM_ForcedAction_InActive);//PWM强制输出低电平
}


/*********************************************************
 * @brief 红外信号发射函数
 * @param {TIM_TypeDef} *TIMx —— 发射通道，TIM2:通过LED1发射；TIM2:通过LED2发射
 * @return None
 *********************************************************/
void Remote_Send(TIM_TypeDef *TIMx,u16 data[],uint16_t length)
{
	uint16_t i;
	if(TIMx == TIM2)
	{
		TIM_SelectOCxM(TIM2, TIM_Channel_2, TIM_OCMode_PWM1);
    	TIM_CCxCmd(TIM2, TIM_Channel_2, TIM_CCx_Enable);
	}
    else if(TIMx == TIM5)
	{
		TIM_SelectOCxM(TIM5, TIM_Channel_1, TIM_OCMode_PWM1);
    	TIM_CCxCmd(TIM5, TIM_Channel_1, TIM_CCx_Enable);
	}
	
	for (i=0; i<length; i++)
	{
		if(i % 2 == 0)
		{
			TIM_Cmd(TIMx, ENABLE);
			Delay_us(data[i]);
			TIM_Cmd(TIMx, DISABLE);
			if(TIMx == TIM2)
			{
				GPIO_SetBits(GPIOA,GPIO_Pin_1);
			}
			else if(TIMx == TIM5)
			{
				GPIO_SetBits(GPIOA,GPIO_Pin_0);
			}
		}
		else
		{
			if(TIMx == TIM2)
			{
				GPIO_SetBits(GPIOA,GPIO_Pin_1);
			}
			else if(TIMx == TIM5)
			{
				GPIO_SetBits(GPIOA,GPIO_Pin_0);
			}
			Delay_us(data[i]);
		}
	}
	memset(Send_DataTab, 0, sizeof(Send_DataTab));
	TIM_Cmd(TIMx, DISABLE);
	if(TIMx == TIM2)
	{
		TIM_ForcedOC2Config(TIM2, TIM_ForcedAction_InActive);//PWM强制输出低电平
	}
	else if(TIMx == TIM5)
	{
		TIM_ForcedOC1Config(TIM5, TIM_ForcedAction_InActive);//PWM强制输出低电平
	}
}

/*********************************************************
 * @brief 开始红外学习 
 * @param {uint8_t} *BUF——W5500接收到的数据
 * @return None
 *********************************************************/
void funcNet_StartLearn(uint8_t *BUF)
{

    if((strstr((const char *)BUF+8, "funcNetStartLearn")))
    {
        
        EXTI->IMR |= EXTI_Line8;    
	}
}


/*********************************************************
 * @brief 开始红外学习 
 * @param {uint8_t} *BUF——串口接收到的数据
 * @return None
 *********************************************************/
void funcWIFI_StartLearn(uint8_t *BUF)
{
	if ((strstr((const char*)BUF, "funcWIFIStartLearn")))
	{
		WIFI_CommandFlag = 1;
		EXTI->IMR |= EXTI_Line8;
	}
}

/*********************************************************
 * @brief 红外信号发射
 * @param {uint8_t} *BUF——W5500接收到的数据
 * @return None
 *********************************************************/
void funcNet_StarSend(uint8_t *BUF)
{
    uint8_t *p;

    if((strstr((const char *)BUF+8, "funcNetStartSend")))
    {
        p = (uint8_t *)((strstr((const char *)BUF+8, "funcNetStartSend")) + strlen("funcNetStartSend"));
        p = (u8 *)(strstr((const char *)p,":")+strlen(":"));//发射通道
        channel = atoi((const char *)p);

        p = (u8 *)(strstr((const char *)p,",")+strlen(","));//红外码起始地址
        start_addr = atoi((const char *)p);
		
        p = (u8 *)(strstr((const char *)p,",")+strlen(","));//红外码长度
        data_length = atoi((const char *)p);
        
        AT24C512_Read2Byte(start_addr, Send_DataTab, data_length);
        Delay_ms(100);
        if(channel == 1)
        {
            Remote_Send(TIM5,Send_DataTab,data_length);
        }
        else if(channel == 2)
        {
            Remote_Send(TIM2,Send_DataTab,data_length);
        }
		start_addr = 0;
		data_length = 0;
		GPIO_ResetBits(GPIOA,GPIO_Pin_1);
		GPIO_ResetBits(GPIOA,GPIO_Pin_0);
    }
}

/*********************************************************
 * @brief 红外信号发射
 * @param {uint8_t} *BUF——USART接收到的数据
 * @return None
 *********************************************************/
void funcWIFI_StarSend(uint8_t *BUF)
{
    uint8_t *p;

    if((strstr((const char *)BUF, "funcWIFIStartSend")))
    {
		p = (uint8_t*)((strstr((const char*)BUF, "funcWIFIStartSend")) + strlen("funcWIFIStartSend"));
        p = (u8 *)(strstr((const char *)p,":")+strlen(":"));//发射通道
        channel = atoi((const char *)p);

        p = (u8 *)(strstr((const char *)p,",")+strlen(","));//红外码起始地址
        start_addr = atoi((const char *)p);
		
        p = (u8 *)(strstr((const char *)p,",")+strlen(","));//红外码长度
        data_length = atoi((const char *)p);
        
        AT24C512_Read2Byte(start_addr, Send_DataTab, data_length);
        Delay_ms(100);
        if(channel == 1)
        {
            Remote_Send(TIM5,Send_DataTab,data_length);
        }
        else if(channel == 2)
        {
            Remote_Send(TIM2,Send_DataTab,data_length);
        }
		start_addr = 0;
		data_length = 0;
		GPIO_ResetBits(GPIOA,GPIO_Pin_1);
		GPIO_ResetBits(GPIOA,GPIO_Pin_0);
    }
}
