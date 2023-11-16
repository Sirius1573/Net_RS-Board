/*********************************************************
 * @description: 该文件包含按键初始化函数
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2023-04-11 20:56:09
 *********************************************************/
#include "KEY.h"
#include "delay.h"
#include "sys.h"
#include "stm32f10x.h"
#include "USART.h"
#include "w5500.h"
#include "Remote.h"
#include "DataPackage.h"
#include "24cxx.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

uint8_t Key_Time = 0;
uint8_t NetParam_Str[1024];
extern u16 IR_CodeStartAddr[];
/*********************************************************
 * @brief 按键初始化
 * @return None
 *********************************************************/
void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    
    TIM_DeInit(TIM3);//复位  
    TIM_TimeBaseInitStructure.TIM_Prescaler = 7199;//72MHz
    TIM_TimeBaseInitStructure.TIM_Period = 9999;// 1S
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;//自动重载计数周期值
    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);//更新中断
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置 NVIC 中断分组 2:2 位抢占优先级，2 位响应优先级
    
    //中断优先级 NVIC 设置
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; //TIM3 中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //先占优先级 0 级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; //从优先级 3 级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ 通道被使能
    NVIC_Init(&NVIC_InitStructure); //初始化 NVIC 寄存器

    TIM_Cmd(TIM3,ENABLE);
}

uint8_t Key_State(void)
{
    if(PBin(1)==0)
    {
        return 1; 
    }
    else
    {
        return 0;
    }
}

void TIM3_IRQHandler(void)
{
    Key_State();
    if (TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET) //检查  更新中断发生与否
    {
        Key_State();
        switch (Key_State())
        {
            case 0:
                    Key_Time = 0;
                    break;
            case 1:
                    ++Key_Time;
                    break;
        }
    }

    TIM_ClearITPendingBit(TIM3,TIM_IT_Update ); //清除 TIM6 更新中断 
}

/*********************************************************
 * @brief 恢复出厂设置函数
 * @return None
 *********************************************************/
uint8_t Device_Init(void)
{
	uint8_t i;
	uint16_t dataToWrite[2];
	
	AT24C512_Write2Byte(98,IR_CodeStartAddr,1);
	AT24C512_Read2Byte(98, Last_DataEAddr, 1);
	
    Write_W5500_nByte(GAR, Default_Gateway_IP, 4);
    AT24CXX_Write(34, Default_Gateway_IP, 4);//更改存储当前网关为默认网关
    AT24CXX_Read(34, Gateway_IP, 4);//读出网关

    Write_W5500_nByte(SUBR,Default_SubMask,4);
    AT24CXX_Write(38, Default_SubMask, 4);//更改存储当前子网掩码为默认子网掩码
    AT24CXX_Read(38, Sub_Mask, 4);//读出子网掩码

    Write_W5500_nByte(SHAR,Default_PhyAddr,6);
    AT24CXX_Write(46, Default_PhyAddr, 6);//更改存储当前MAC为默认MAC
    AT24CXX_Read(46, Phy_Addr, 6);//读出MAC

    Write_W5500_nByte(SIPR,Default_IPAddr,4);
    AT24CXX_Write(26, Default_IPAddr, 4);//更改存储当前设备IP为默认设备IP
    AT24CXX_Read(26, IP_Addr, 4);//读出设备IP

    S0_DIP[0]=Default_S0DIP[0];
    S0_DIP[1]=Default_S0DIP[1];
    S0_DIP[2]=Default_S0DIP[2];
    S0_DIP[3]=Default_S0DIP[3];
    AT24CXX_Write(30, Default_S0DIP, 4);//更改存储当前服务器IP为默认服务器IP
    AT24CXX_Read(30, S0_DIP, 4);//读出服务器IP
                            
    S0_DPort[0] = Default_S0DPort[0];
    S0_DPort[1] = Default_S0DPort[1];
    AT24CXX_Write(42, S0_DPort, 2);//更改存储当前服务器端口为默认服务器端口
    AT24CXX_Read(42, S0_DPort, 2);//读出服务器端口

    S0_Port[0] = Default_S0Port[0];
    S0_Port[1] = Default_S0Port[1];
    Write_W5500_SOCK_2Byte(0, Sn_PORT, S0_Port[0]*256+S0_Port[1]);
    AT24CXX_Write(44, Default_S0Port, 2);//更改存储当前设备端口为默认默认端口
    AT24CXX_Read(44, S0_Port, 2);//读出设备端口
	
	AT24CXX_Write(55, USART2_Config.USART_Param_Index, 4);
	AT24CXX_Write(59, USART3_Config.USART_Param_Index, 4);
	AT24CXX_Write(63, UART4_Config.USART_Param_Index, 4);
	for (i = 0;i <= 2;i++)
	{
		dataToWrite[0] = (uint16_t)(9600 & 0xFFFF);         // 最低 16 位
		dataToWrite[1] = (uint16_t)((9600 >> 16) & 0xFFFF);  // 高 16 位
		AT24C512_Write2Byte(67 + 4 * i, dataToWrite, 2);
	}
	AT24CXX_Read(55, USART2_Config.USART_Param_Index, 4);
	AT24CXX_Read(59, USART3_Config.USART_Param_Index, 4);
	AT24CXX_Read(63, UART4_Config.USART_Param_Index, 4);
	for (i = 0;i <= 2;i++)
	{
		AT24C512_Read2Byte(67 + 4 * i, dataToWrite, 2);
		Buad_Tab[i] = ((uint32_t)dataToWrite[1] << 16) | (uint32_t)dataToWrite[0];
	}
    Delay_ms(50);
	
    memcpy(Tx_Buffer,"The device has been initialized\r\n", 34);	
    USARTx_SendArray(USART1,Tx_Buffer,34);
    USARTx_SendArray(USART2, Tx_Buffer, 34);
    USARTx_SendArray(USART3, Tx_Buffer, 34);
    USARTx_SendArray(UART4, Tx_Buffer, 34);
    Write_SOCK_Data_Buffer(0, Tx_Buffer, 34);//指定Socket(0~7)发送数据处理
    sprintf((char *)NetParam_Str, "UDPNowParam:\r\n%d.%d.%d.%d\r\n%d.%d.%d.%d\r\n%d.%d.%d.%d\r\n%d.%d.%d.%d\r\n%d\r\n%d\r\n%02x:%02x:%02x:%02x:%02x:%02x\r\n",
                IP_Addr[0],IP_Addr[1],IP_Addr[2],IP_Addr[3],/*设备IP*/
                S0_DIP[0],S0_DIP[1],S0_DIP[2],S0_DIP[3],/*服务器IP*/
                Gateway_IP[0],Gateway_IP[1],Gateway_IP[2],Gateway_IP[3],/*网关*/
                Sub_Mask[0],Sub_Mask[1],Sub_Mask[2],Sub_Mask[3],/*子网掩码*/
                S0_DPort[0]*256+S0_DPort[1],/*服务器IP端口*/
                S0_Port[0]*256+S0_Port[1],/*设备端口*/
                Phy_Addr[0],Phy_Addr[1],Phy_Addr[2],Phy_Addr[3],Phy_Addr[4],Phy_Addr[5]);/*设备MAC*/

    memcpy(Tx_Buffer,NetParam_Str, strlen((const char *)NetParam_Str));	
    Write_SOCK_Data_Buffer(0, Tx_Buffer, strlen((const char *)NetParam_Str));//指定Socket(0~7)发送数据处理
	sprintf((char*)NetParam_Str,
            "RSNowParam:\r\nRS_CH1{%d,%d,%d,%d,%d}\r\nRS_CH2{%d,%d,%d,%d,%d}\r\nRS_CH3{%d,%d,%d,%d,%d}\r\n",
            Buad_Tab[0], USART2_Config.USART_Param_Index[0], USART2_Config.USART_Param_Index[1], USART2_Config.USART_Param_Index[2], USART2_Config.USART_Param_Index[3],
            Buad_Tab[1], USART3_Config.USART_Param_Index[0], USART3_Config.USART_Param_Index[1], USART3_Config.USART_Param_Index[2], USART3_Config.USART_Param_Index[3],
            Buad_Tab[2], UART4_Config.USART_Param_Index[0], UART4_Config.USART_Param_Index[1], UART4_Config.USART_Param_Index[2], UART4_Config.USART_Param_Index[3]);
	memcpy(Tx_Buffer, NetParam_Str, strlen((const char*)NetParam_Str));
	Write_SOCK_Data_Buffer(0, Tx_Buffer, strlen((const char*)NetParam_Str));//指定Socket(0~7)发送数据处理
	memset(Tx_Buffer, 0x00, 100);
    Delay_ms(50);
    __set_FAULTMASK(1);//软件复位stm32
    NVIC_SystemReset();
    Key_Time = 0;
    return 1;
}
