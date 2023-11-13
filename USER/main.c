/*********************************************************
 * @description:main.c
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2023-06-03 20:56:34
 *********************************************************/
#include "stm32f10x.h"
#include "USART.h"
#include "OCEP.h"
#include "SCIO.h"
#include "delay.h"
#include "W5500.h"
#include "Remote.h"
#include "24cxx.h"
#include "DataPackage.h"
#include "KEY.h"
#include "24cxx.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


uint8_t RS485BUF_1, RS485BUF_2, RS485BUF_3;
uint16_t W5500_Send_Delay_Counter=0;
uint8_t i;
u16 IR_CodeStartAddr[]={99};
int main(void)
{
	PWR_WakeUpPinCmd(DISABLE);   //��ֹWKUP����
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	Delay_init();
	OCEP_Init();
	SCIO_Init();
	Key_Init();
	AT24CXX_Init();
	Load_USART_Param();
	USART1_Init(115200);
    USART2_Init(Buad_Tab[0]);
	USART3_Init(Buad_Tab[1]);
	UART4_Init(Buad_Tab[2]);
    UART5_Init(115200);
	Remote_Reciv_Init();
    Remote_Send1_Init();
    Remote_Send2_Init();
	SPI_Configuration();		//W5500 SPI��ʼ������(STM32 SPI1)
	W5500_GPIO_Configuration();	//W5500 GPIO��ʼ������	
	Load_Net_Parameters();		//װ��Ӳ������
	W5500_Hardware_Reset();		//Ӳ����λW5500
	W5500_Init();	
	Detect_Gateway();
	Socket_Init(0);
	Online_Reminder();
	// AT24C512_Write2Byte(98,IR_CodeStartAddr,1);
	AT24C512_Read2Byte(98, Last_DataEAddr, 1);

	while(1)
	{
		uint8_t buf[20];
		Key_State();
		W5500_Socket_Set();//W5500�˿ڳ�ʼ������
		W5500_Interrupt_Process();//W5500�жϴ��������
		Address_Check();    //����Ƿ�Ϊָ��������IP�˿ڷ���
		SCIO_Ctr();
		if(Key_Time==3)
		{
			Device_Init();	//�ָ���������
			Online_Reminder();
		}
		RS485rwack_1 = 0;
		RS485rwack_2 = 0;
		RS485rwack_3 = 0;
		if (((S0_Data & S_RECEIVE) == S_RECEIVE))//���Socket0���յ�����
		{
			Process_Socket_Data(0);
			Address_Check();
     		S0_Data&=~S_RECEIVE;
			if(check==0)
			{	

				memcpy(Tx_Buffer, "funcNetreply\r\n", 15);	
				Write_SOCK_Data_Buffer(0, Tx_Buffer, 15);//ָ��Socket(0~7)�������ݴ���
				Set_NetParam(Rx_Buffer);
				funcNet_OCEPCtrl(Rx_Buffer);
				funcNet_StartLearn(Rx_Buffer);
				funcNet_StarSend(Rx_Buffer);
				funcNet_MesgToRS(Rx_Buffer);
				funcNet_SetUARTParam(Rx_Buffer);
				Get_NetParam();
				Get_USARTParam();
			}
		}
		USART_Proce();
	}
 }
