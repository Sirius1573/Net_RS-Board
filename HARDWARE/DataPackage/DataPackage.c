/*********************************************************
 * @description:该文件包含网络通信及串口通信协议
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
 * @brief 服务器IP及端口检查函数
 * @return None
 *********************************************************/
void Address_Check(void)
{
    if ((Rx_Buffer[0] == S0_DIP[0]) && (Rx_Buffer[1] == S0_DIP[1]) && (Rx_Buffer[2] == S0_DIP[2]) && (Rx_Buffer[3] == S0_DIP[3]) &&
        (Rx_Buffer[4] == S0_DPort[0]) && (Rx_Buffer[5] == S0_DPort[1]))
    {
        check = 0;  //如果实际IP、端口与定义IP、端口相同则置0
    }
}

/*********************************************************
 * @brief 上线提示函数——设备上电后，当检测到服务器发送消息时，设备发出等待上线指令，当收到指定服务器发送的"funcNetStartUp"指令时，设备发出已上线提示
 * @return None
 *********************************************************/
void Online_Reminder(void)
{
    
    do
    {
        Key_State();
        if (Key_Time == 3)
        {
            Device_Init();	//恢复出厂设置
            Online_Reminder();
        }
        W5500_Socket_Set();//W5500端口初始化配置
        W5500_Interrupt_Process();//W5500中断处理程序框架 
		Process_Socket_Data(0);		
        S0_Data &= ~S_RECEIVE;

        if (((check2 != 0) && (check != 0)) || (USART_Channel != 0))
        {
            memcpy(Tx_Buffer, "\r\nEquipment waiting to come online\r\n", 37);
            Write_SOCK_Data_Buffer(0, Tx_Buffer, 37);//指定Socket(0~7)发送数据处理
        }
        Delay_ms(500);
        if ((strstr((const char*)Rx_Buffer + 8, "funcNetStartUp")))
        {
            check2 = 0;
        }
        Address_Check();    //检查是否为指定服务器IP端口发送
        if (((check2 == 0) && (check == 0)) || (USART_Channel == 0))  //如果收到上线指令且服务器IP、端口正确，则提示设备已经上线
        {
            startup_flag = 1;
            memcpy(Tx_Buffer, "Equipment is now online\r\n", 26);
            Write_SOCK_Data_Buffer(0, Tx_Buffer, 26);//指定Socket(0~7)发送数据处理
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

    } while ((check2 || check) && USART_Channel);  //如果收到上线指令且服务器IP、端口正确，退出此循环，进入主循环
}

/*********************************************************
 * @brief 网络参数设置函数
 * @param {uint8_t} *BUF — 设置命令
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

    //设置新的网络参数:设备IP,服务器IP，网关，子网掩码，服务器端口，设备端口，MAC地址
    if ((strstr((const char*)BUF + 8, "funcNetSetParam")))
    {
        /****************设备IP*************/
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
        Write_W5500_nByte(SIPR, IP_Addr, 4);//写入新的设备IP
        AT24CXX_Write(26, IP_Addr, 4);

        /****************服务器IP*************/
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

        /****************网关*************/
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
        Write_W5500_nByte(GAR, Gateway_IP, 4);//写入新的网关
        AT24CXX_Write(34, Gateway_IP, 4);

        /****************子网掩码*************/
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
        Write_W5500_nByte(SUBR, Sub_Mask, 4);//写入新的子网掩码
        AT24CXX_Write(38, Sub_Mask, 4);

        /****************服务器端口*************/
        p = (u8*)(strstr((const char*)p, ";") + strlen(";"));
        NetParam = atoi((const char*)p);
        S0_DPort[0] = (NetParam >> 8);
        S0_DPort[1] = (NetParam & 0x00ff);
        AT24CXX_Write(42, S0_DPort, 2);

        /****************设备端口*************/
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
        Write_W5500_nByte(SHAR, Phy_Addr, 6);//写入新的MAC
        AT24CXX_Write(46, Phy_Addr, 6);

        Delay_ms(50);
        memcpy(Tx_Buffer, "Parameter modification succeeded\r\n", 35);
        Write_SOCK_Data_Buffer(0, Tx_Buffer, 35);//指定Socket(0~7)发送数据处理
        sprintf((char*)NetParam_Str, "NowParam:%d.%d.%d.%d, %d.%d.%d.%d, %d.%d.%d.%d, %d.%d.%d.%d, %d, %d, %02x:%02x:%02x:%02x:%02x:%02x",
            IP_Addr[0], IP_Addr[1], IP_Addr[2], IP_Addr[3],/*设备IP*/
            S0_DIP[0], S0_DIP[1], S0_DIP[2], S0_DIP[3],/*服务器IP*/
            Gateway_IP[0], Gateway_IP[1], Gateway_IP[2], Gateway_IP[3],/*网关*/
            Sub_Mask[0], Sub_Mask[1], Sub_Mask[2], Sub_Mask[3],/*子网掩码*/
            S0_DPort[0] * 256 + S0_DPort[1],/*服务器IP端口*/
            S0_Port[0] * 256 + S0_Port[1],/*设备端口*/
            Phy_Addr[0], Phy_Addr[1], Phy_Addr[2], Phy_Addr[3], Phy_Addr[4], Phy_Addr[5]);/*设备MAC*/
        memcpy(Tx_Buffer, NetParam_Str, strlen((const char*)NetParam_Str));
        Write_SOCK_Data_Buffer(0, Tx_Buffer, strlen((const char*)NetParam_Str));//指定Socket(0~7)发送数据处理
		Delay_ms(50);
		__set_FAULTMASK(1);//软件复位stm32
		NVIC_SystemReset();
    }
}

/*********************************************************
 * @brief 网络参数获取函数
 * @return None
 *********************************************************/
void Get_NetParam(void)
{
    uint8_t NetParam_Str[1024];
    //查询网络参数
    if ((strstr((const char*)Rx_Buffer + 8, "funcNetGetParam")))
    {   /*输出格式：设备IP，服务器IP，网关，子网掩码，服务器端口，设备端口，设备MAC*/
        sprintf((char*)NetParam_Str, "NowParam:%d.%d.%d.%d, %d.%d.%d.%d, %d.%d.%d.%d, %d.%d.%d.%d, %d, %d, %02x:%02x:%02x:%02x:%02x:%02x",
            IP_Addr[0], IP_Addr[1], IP_Addr[2], IP_Addr[3],/*设备IP*/
            S0_DIP[0], S0_DIP[1], S0_DIP[2], S0_DIP[3],/*服务器IP*/
            Gateway_IP[0], Gateway_IP[1], Gateway_IP[2], Gateway_IP[3],/*网关*/
            Sub_Mask[0], Sub_Mask[1], Sub_Mask[2], Sub_Mask[3],/*子网掩码*/
            S0_DPort[0] * 256 + S0_DPort[1],/*服务器IP端口*/
            S0_Port[0] * 256 + S0_Port[1],/*设备端口*/
            Phy_Addr[0], Phy_Addr[1], Phy_Addr[2], Phy_Addr[3], Phy_Addr[4], Phy_Addr[5]);/*设备MAC*/
        memcpy(Tx_Buffer, NetParam_Str, strlen((const char*)NetParam_Str));
        Write_SOCK_Data_Buffer(0, Tx_Buffer, strlen((const char*)NetParam_Str));//指定Socket(0~7)发送数据处理
    }
}

/*********************************************************
 * @brief 继电器控制函数
 * @param {uint8_t} *BUF —— 控制命令，网络接收
 *        示例：funcNetOCEPCtrl,1,0,1,1,0,1,0,0 —— 1号继电器开，2号继电器关......
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
 * @brief 继电器控制函数
 * @param {uint8_t} *BUF —— 控制命令，串口接收
 *        示例：funcNetOCEPCtrl,1,0,1,1,0,1,0,0 —— 1号继电器开，2号继电器关......
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
