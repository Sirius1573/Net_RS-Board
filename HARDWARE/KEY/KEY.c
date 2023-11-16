/*********************************************************
 * @description: ���ļ�����������ʼ������
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
 * @brief ������ʼ��
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

    
    TIM_DeInit(TIM3);//��λ  
    TIM_TimeBaseInitStructure.TIM_Prescaler = 7199;//72MHz
    TIM_TimeBaseInitStructure.TIM_Period = 9999;// 1S
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//���ϼ���
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;//�Զ����ؼ�������ֵ
    TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
    TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);//�����ж�
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//���� NVIC �жϷ��� 2:2 λ��ռ���ȼ���2 λ��Ӧ���ȼ�
    
    //�ж����ȼ� NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; //TIM3 �ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; //��ռ���ȼ� 0 ��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; //�����ȼ� 3 ��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ ͨ����ʹ��
    NVIC_Init(&NVIC_InitStructure); //��ʼ�� NVIC �Ĵ���

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
    if (TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET) //���  �����жϷ������
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

    TIM_ClearITPendingBit(TIM3,TIM_IT_Update ); //��� TIM6 �����ж� 
}

/*********************************************************
 * @brief �ָ��������ú���
 * @return None
 *********************************************************/
uint8_t Device_Init(void)
{
	uint8_t i;
	uint16_t dataToWrite[2];
	
	AT24C512_Write2Byte(98,IR_CodeStartAddr,1);
	AT24C512_Read2Byte(98, Last_DataEAddr, 1);
	
    Write_W5500_nByte(GAR, Default_Gateway_IP, 4);
    AT24CXX_Write(34, Default_Gateway_IP, 4);//���Ĵ洢��ǰ����ΪĬ������
    AT24CXX_Read(34, Gateway_IP, 4);//��������

    Write_W5500_nByte(SUBR,Default_SubMask,4);
    AT24CXX_Write(38, Default_SubMask, 4);//���Ĵ洢��ǰ��������ΪĬ����������
    AT24CXX_Read(38, Sub_Mask, 4);//������������

    Write_W5500_nByte(SHAR,Default_PhyAddr,6);
    AT24CXX_Write(46, Default_PhyAddr, 6);//���Ĵ洢��ǰMACΪĬ��MAC
    AT24CXX_Read(46, Phy_Addr, 6);//����MAC

    Write_W5500_nByte(SIPR,Default_IPAddr,4);
    AT24CXX_Write(26, Default_IPAddr, 4);//���Ĵ洢��ǰ�豸IPΪĬ���豸IP
    AT24CXX_Read(26, IP_Addr, 4);//�����豸IP

    S0_DIP[0]=Default_S0DIP[0];
    S0_DIP[1]=Default_S0DIP[1];
    S0_DIP[2]=Default_S0DIP[2];
    S0_DIP[3]=Default_S0DIP[3];
    AT24CXX_Write(30, Default_S0DIP, 4);//���Ĵ洢��ǰ������IPΪĬ�Ϸ�����IP
    AT24CXX_Read(30, S0_DIP, 4);//����������IP
                            
    S0_DPort[0] = Default_S0DPort[0];
    S0_DPort[1] = Default_S0DPort[1];
    AT24CXX_Write(42, S0_DPort, 2);//���Ĵ洢��ǰ�������˿�ΪĬ�Ϸ������˿�
    AT24CXX_Read(42, S0_DPort, 2);//�����������˿�

    S0_Port[0] = Default_S0Port[0];
    S0_Port[1] = Default_S0Port[1];
    Write_W5500_SOCK_2Byte(0, Sn_PORT, S0_Port[0]*256+S0_Port[1]);
    AT24CXX_Write(44, Default_S0Port, 2);//���Ĵ洢��ǰ�豸�˿�ΪĬ��Ĭ�϶˿�
    AT24CXX_Read(44, S0_Port, 2);//�����豸�˿�
	
	AT24CXX_Write(55, USART2_Config.USART_Param_Index, 4);
	AT24CXX_Write(59, USART3_Config.USART_Param_Index, 4);
	AT24CXX_Write(63, UART4_Config.USART_Param_Index, 4);
	for (i = 0;i <= 2;i++)
	{
		dataToWrite[0] = (uint16_t)(9600 & 0xFFFF);         // ��� 16 λ
		dataToWrite[1] = (uint16_t)((9600 >> 16) & 0xFFFF);  // �� 16 λ
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
    Write_SOCK_Data_Buffer(0, Tx_Buffer, 34);//ָ��Socket(0~7)�������ݴ���
    sprintf((char *)NetParam_Str, "UDPNowParam:\r\n%d.%d.%d.%d\r\n%d.%d.%d.%d\r\n%d.%d.%d.%d\r\n%d.%d.%d.%d\r\n%d\r\n%d\r\n%02x:%02x:%02x:%02x:%02x:%02x\r\n",
                IP_Addr[0],IP_Addr[1],IP_Addr[2],IP_Addr[3],/*�豸IP*/
                S0_DIP[0],S0_DIP[1],S0_DIP[2],S0_DIP[3],/*������IP*/
                Gateway_IP[0],Gateway_IP[1],Gateway_IP[2],Gateway_IP[3],/*����*/
                Sub_Mask[0],Sub_Mask[1],Sub_Mask[2],Sub_Mask[3],/*��������*/
                S0_DPort[0]*256+S0_DPort[1],/*������IP�˿�*/
                S0_Port[0]*256+S0_Port[1],/*�豸�˿�*/
                Phy_Addr[0],Phy_Addr[1],Phy_Addr[2],Phy_Addr[3],Phy_Addr[4],Phy_Addr[5]);/*�豸MAC*/

    memcpy(Tx_Buffer,NetParam_Str, strlen((const char *)NetParam_Str));	
    Write_SOCK_Data_Buffer(0, Tx_Buffer, strlen((const char *)NetParam_Str));//ָ��Socket(0~7)�������ݴ���
	sprintf((char*)NetParam_Str,
            "RSNowParam:\r\nRS_CH1{%d,%d,%d,%d,%d}\r\nRS_CH2{%d,%d,%d,%d,%d}\r\nRS_CH3{%d,%d,%d,%d,%d}\r\n",
            Buad_Tab[0], USART2_Config.USART_Param_Index[0], USART2_Config.USART_Param_Index[1], USART2_Config.USART_Param_Index[2], USART2_Config.USART_Param_Index[3],
            Buad_Tab[1], USART3_Config.USART_Param_Index[0], USART3_Config.USART_Param_Index[1], USART3_Config.USART_Param_Index[2], USART3_Config.USART_Param_Index[3],
            Buad_Tab[2], UART4_Config.USART_Param_Index[0], UART4_Config.USART_Param_Index[1], UART4_Config.USART_Param_Index[2], UART4_Config.USART_Param_Index[3]);
	memcpy(Tx_Buffer, NetParam_Str, strlen((const char*)NetParam_Str));
	Write_SOCK_Data_Buffer(0, Tx_Buffer, strlen((const char*)NetParam_Str));//ָ��Socket(0~7)�������ݴ���
	memset(Tx_Buffer, 0x00, 100);
    Delay_ms(50);
    __set_FAULTMASK(1);//�����λstm32
    NVIC_SystemReset();
    Key_Time = 0;
    return 1;
}
