/*********************************************************
 * @description:���ļ���������ͨ�ż�����ͨ��Э��
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2023-06-06 11:44:50
 *********************************************************/
#include "DataPackage.h"
#include "stm32f10x.h"
#include "w5500.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "delay.h"
#include "USART.h"
#include "24cxx.h"
#include "OCEP.h"
#include "KEY.h"
uint8_t check = 1;
uint8_t check2 = 1;
uint8_t tset[2];
/*********************************************************
 * @brief ������IP���˿ڼ�麯��
 * @return None
 *********************************************************/
void Address_Check(void)
{
    if ((Rx_Buffer[0] == S0_DIP[0]) && (Rx_Buffer[1] == S0_DIP[1]) && (Rx_Buffer[2] == S0_DIP[2]) && (Rx_Buffer[3] == S0_DIP[3]) &&
        (Rx_Buffer[4] == S0_DPort[0]) && (Rx_Buffer[5] == S0_DPort[1]))
    {
        check = 0;  //���ʵ��IP���˿��붨��IP���˿���ͬ����0
    }
}

/*********************************************************
 * @brief ������ʾ���������豸�ϵ�󣬵���⵽������������Ϣʱ���豸�����ȴ�����ָ����յ�ָ�����������͵�"funcNetStartUp"ָ��ʱ���豸������������ʾ
 * @return None
 *********************************************************/
void Online_Reminder(void)
{
    
    do
    {
        Key_State();
        if (Key_Time == 3)
        {
            Device_Init();	//�ָ���������
            Online_Reminder();
        }
        W5500_Socket_Set();//W5500�˿ڳ�ʼ������
        W5500_Interrupt_Process();//W5500�жϴ�������� 
		Process_Socket_Data(0);		
        S0_Data &= ~S_RECEIVE;

        if (((check2 != 0) && (check != 0)) || (USART_Channel != 0))
        {
            memcpy(Tx_Buffer, "\r\nEquipment waiting to come online\r\n", 37);
            Write_SOCK_Data_Buffer(0, Tx_Buffer, 37);//ָ��Socket(0~7)�������ݴ���
        }
        Delay_ms(500);
        if ((strstr((const char*)Rx_Buffer + 8, "funcNetStartUp")))
        {
            check2 = 0;
        }
        Address_Check();    //����Ƿ�Ϊָ��������IP�˿ڷ���
        if (((check2 == 0) && (check == 0)) || (USART_Channel == 0))  //����յ�����ָ���ҷ�����IP���˿���ȷ������ʾ�豸�Ѿ�����
        {
            startup_flag = 1;
            memcpy(Tx_Buffer, "Equipment is now online\r\n", 26);
            Write_SOCK_Data_Buffer(0, Tx_Buffer, 26);//ָ��Socket(0~7)�������ݴ���
        }
		RS485rwack_1 = 1;
		RS485rwack_2 = 1;
		RS485rwack_3 = 1;
        USARTx_SendArray(USART1, Tx_Buffer, 37);
        USARTx_SendArray(USART2, Tx_Buffer, 37);
        USARTx_SendArray(USART3, Tx_Buffer, 37);
        USARTx_SendArray(UART4, Tx_Buffer, 37);
		RS485rwack_1 = 0;
		RS485rwack_2 = 0;
		RS485rwack_3 = 0;
		
        Get_NetParam();

    } while ((check2 || check) && USART_Channel);  //����յ�����ָ���ҷ�����IP���˿���ȷ���˳���ѭ����������ѭ��
}

/*********************************************************
 * @brief ����������ú���
 * @param {uint8_t} *BUF �� ��������
 * @return None
 *********************************************************/
void Set_NetParam(uint8_t* BUF)
{
    uint8_t* p;
    uint16_t NetParam;
    uint8_t NetParam_Str[1024];
    Read_SOCK_Data_Buffer(0, Rx_Buffer);
    UDP_DIPR[0] = Rx_Buffer[0];
    UDP_DIPR[1] = Rx_Buffer[1];
    UDP_DIPR[2] = Rx_Buffer[2];
    UDP_DIPR[3] = Rx_Buffer[3];
    UDP_DPORT[0] = Rx_Buffer[4];
    UDP_DPORT[1] = Rx_Buffer[5];

    //�����µ��������:�豸IP,������IP�����أ��������룬�������˿ڣ��豸�˿ڣ�MAC��ַ
    if ((strstr((const char*)BUF + 8, "funcNetSetParam")))
    {
        /****************�豸IP*************/
        p = (uint8_t*)((strstr((const char*)BUF + 8, "funcNetSetParam")) + strlen("funcNetSetparam"));
        p = (u8*)(strstr((const char*)p, ":") + strlen(":"));
        NetParam = atoi((const char*)p);
        IP_Addr[0] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        IP_Addr[1] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        IP_Addr[2] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        IP_Addr[3] = NetParam;
        Write_W5500_nByte(SIPR, IP_Addr, 4);//д���µ��豸IP
        AT24CXX_Write(26, IP_Addr, 4);

        /****************������IP*************/
        p = (u8*)(strstr((const char*)p, ";") + strlen(";"));
        NetParam = atoi((const char*)p);
        S0_DIP[0] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        S0_DIP[1] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        S0_DIP[2] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        S0_DIP[3] = NetParam;
        AT24CXX_Write(30, S0_DIP, 4);

        /****************����*************/
        p = (u8*)(strstr((const char*)p, ";") + strlen(";"));
        NetParam = atoi((const char*)p);
        Gateway_IP[0] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        Gateway_IP[1] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        Gateway_IP[2] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        Gateway_IP[3] = NetParam;
        Write_W5500_nByte(GAR, Gateway_IP, 4);//д���µ�����
        AT24CXX_Write(34, Gateway_IP, 4);

        /****************��������*************/
        p = (u8*)(strstr((const char*)p, ";") + strlen(";"));
        NetParam = atoi((const char*)p);
        Sub_Mask[0] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        Sub_Mask[1] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        Sub_Mask[2] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        Sub_Mask[3] = NetParam;
        Write_W5500_nByte(SUBR, Sub_Mask, 4);//д���µ���������
        AT24CXX_Write(38, Sub_Mask, 4);

        /****************�������˿�*************/
        p = (u8*)(strstr((const char*)p, ";") + strlen(";"));
        NetParam = atoi((const char*)p);
        S0_DPort[0] = (NetParam >> 8);
        S0_DPort[1] = (NetParam & 0x00ff);
        AT24CXX_Write(42, S0_DPort, 2);

        /****************�豸�˿�*************/
        p = (u8*)(strstr((const char*)p, ";") + strlen(";"));
        NetParam = atoi((const char*)p);
        S0_Port[0] = (NetParam >> 8);
        S0_Port[1] = (NetParam & 0x00ff);
        Write_W5500_SOCK_2Byte(0, Sn_PORT, S0_Port[0] * 256 + S0_Port[1]);
        AT24CXX_Write(44, S0_Port, 2);

        /****************MAC*************/
        p = (u8*)(strstr((const char*)p, ";") + strlen(";"));
        NetParam = atoi((const char*)p);
        Phy_Addr[0] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        Phy_Addr[1] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        Phy_Addr[2] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        Phy_Addr[3] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        Phy_Addr[3] = NetParam;
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        NetParam = atoi((const char*)p);
        Phy_Addr[3] = NetParam;
        Write_W5500_nByte(SHAR, Phy_Addr, 6);//д���µ�MAC
        AT24CXX_Write(46, Phy_Addr, 6);

        Delay_ms(50);
        memcpy(Tx_Buffer, "Parameter modification succeeded\r\n", 35);
        Write_SOCK_Data_Buffer(0, Tx_Buffer, 35);//ָ��Socket(0~7)�������ݴ���
        sprintf((char*)NetParam_Str, "NowParam:%d.%d.%d.%d, %d.%d.%d.%d, %d.%d.%d.%d, %d.%d.%d.%d, %d, %d, %02x:%02x:%02x:%02x:%02x:%02x",
            IP_Addr[0], IP_Addr[1], IP_Addr[2], IP_Addr[3],/*�豸IP*/
            S0_DIP[0], S0_DIP[1], S0_DIP[2], S0_DIP[3],/*������IP*/
            Gateway_IP[0], Gateway_IP[1], Gateway_IP[2], Gateway_IP[3],/*����*/
            Sub_Mask[0], Sub_Mask[1], Sub_Mask[2], Sub_Mask[3],/*��������*/
            S0_DPort[0] * 256 + S0_DPort[1],/*������IP�˿�*/
            S0_Port[0] * 256 + S0_Port[1],/*�豸�˿�*/
            Phy_Addr[0], Phy_Addr[1], Phy_Addr[2], Phy_Addr[3], Phy_Addr[4], Phy_Addr[5]);/*�豸MAC*/
        memcpy(Tx_Buffer, NetParam_Str, strlen((const char*)NetParam_Str));
        Write_SOCK_Data_Buffer(0, Tx_Buffer, strlen((const char*)NetParam_Str));//ָ��Socket(0~7)�������ݴ���
		Delay_ms(50);
		__set_FAULTMASK(1);//�����λstm32
		NVIC_SystemReset();
    }
}

/*********************************************************
 * @brief ���������ȡ����
 * @return None
 *********************************************************/
void Get_NetParam(void)
{
    uint8_t NetParam_Str[1024];
    //��ѯ�������
    if ((strstr((const char*)Rx_Buffer + 8, "funcNetGetParam")))
    {   /*�����ʽ���豸IP��������IP�����أ��������룬�������˿ڣ��豸�˿ڣ��豸MAC*/
        sprintf((char*)NetParam_Str, "NowParam:%d.%d.%d.%d, %d.%d.%d.%d, %d.%d.%d.%d, %d.%d.%d.%d, %d, %d, %02x:%02x:%02x:%02x:%02x:%02x",
            IP_Addr[0], IP_Addr[1], IP_Addr[2], IP_Addr[3],/*�豸IP*/
            S0_DIP[0], S0_DIP[1], S0_DIP[2], S0_DIP[3],/*������IP*/
            Gateway_IP[0], Gateway_IP[1], Gateway_IP[2], Gateway_IP[3],/*����*/
            Sub_Mask[0], Sub_Mask[1], Sub_Mask[2], Sub_Mask[3],/*��������*/
            S0_DPort[0] * 256 + S0_DPort[1],/*������IP�˿�*/
            S0_Port[0] * 256 + S0_Port[1],/*�豸�˿�*/
            Phy_Addr[0], Phy_Addr[1], Phy_Addr[2], Phy_Addr[3], Phy_Addr[4], Phy_Addr[5]);/*�豸MAC*/
        memcpy(Tx_Buffer, NetParam_Str, strlen((const char*)NetParam_Str));
        Write_SOCK_Data_Buffer(0, Tx_Buffer, strlen((const char*)NetParam_Str));//ָ��Socket(0~7)�������ݴ���
    }
}

/*********************************************************
 * @brief �̵������ƺ���
 * @param {uint8_t} *BUF ���� ��������������
 *        ʾ����funcNetOCEPCtrl,1,0,1,1,0,1,0,0 ���� 1�ż̵�������2�ż̵�����......
 * @return None
 *********************************************************/
void funcNet_OCEPCtrl(uint8_t* BUF)
{
    uint8_t* p;
    uint8_t data;

    if ((strstr((const char*)BUF + 8, "funcNetOCEPCtrl")))
    {
        data = 0x00;
        p = (uint8_t*)((strstr((const char*)BUF + 8, "funcNetOCEPCtrl")) + strlen("funcNetOCEPCtrl"));

        p = (u8*)(strstr((const char*)p, ":") + strlen(":"));
        data = atoi((const char*)p);
        OCEP_OC(1, data);

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OC(2, data);

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OC(3, data);

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OC(4, data);

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OC(5, data);

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OC(6, data);

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OC(7, data);

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OC(8, data);
    }
}

/*********************************************************
 * @brief �̵������ƺ���
 * @param {uint8_t} *BUF ���� ����������ڽ���
 *        ʾ����funcNetOCEPCtrl,1,0,1,1,0,1,0,0 ���� 1�ż̵�������2�ż̵�����......
 * @return None
 *********************************************************/
void funcRS_OCEPCtrl(uint8_t* BUF)
{
    uint8_t* p;
    uint8_t data;

    if ((strstr((const char*)BUF + 1, "funcRSOCEPCtrl")))
    {
        data = 0x00;
        data = BUF[16] - '0';
        OCEP_OC(1, data);

        data = BUF[18] - '0';
        OCEP_OC(2, data);

        data = BUF[20] - '0';
        OCEP_OC(3, data);

        data = BUF[22] - '0';
        OCEP_OC(4, data);

        data = BUF[24] - '0';
        OCEP_OC(5, data);

        data = BUF[26] - '0';
        OCEP_OC(6, data);

        data = BUF[28] - '0';
        OCEP_OC(7, data);

        data = BUF[30] - '0';
        OCEP_OC(8, data);
    }
}

void funcNet_MesgToRS(uint8_t* BUF)
{
   uint8_t* p;
   uint8_t data;

   if ((strstr((const char*)BUF + 1, "funcNetMesgToRS")))
   {
       
   }
}
