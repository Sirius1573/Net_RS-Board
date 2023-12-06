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
#include "usart.h"
#include "KEY.h"
uint8_t check = 1;
uint8_t check2 = 1;
uint8_t tset[2];
uint16_t Mesg_Length = 0;
uint16_t Tick_Count = 0;
uint8_t Send_Mode = 0;
/*********************************************************
 * @brief 服务器IP及端口检查函数
 * @return None
 *********************************************************/
void Address_Check(void)
{
    if ((Rx_Buffer[0] == S0_DIP[0]) && (Rx_Buffer[1] == S0_DIP[1]) && (Rx_Buffer[2] == S0_DIP[2]) && (Rx_Buffer[3] == S0_DIP[3]) &&
        (Rx_Buffer[4] == S0_DPort[0]) && (Rx_Buffer[5] == S0_DPort[1]) && (strstr((const char*)Rx_Buffer + 8, "funcNetStartUp")))
    {
        check = 0;  //如果实际IP、端口与定义IP、端口相同则置0
    }
    else if (WIFI_UpLineFlag == 1)
	{
		check = 0;
	}
}

void UpLine_Mesg(void)
{
	
	
	if (check == 0)  //如果收到上线指令且服务器IP、端口正确，则提示设备已经上线
    {
        startup_flag = 1;
		USARTx_SendString(USART1, "\r\nEquipment is now online\r\n");
        memcpy(Tx_Buffer, "\r\nEquipment is now online\r\n", 28);
        Write_SOCK_Data_Buffer(0, Tx_Buffer, 28);//指定Socket(0~7)发送数据处理
        memset(Tx_Buffer, 0x00, 28);
   
        // RS485rwack_1 = 1;
		// RS485rwack_2 = 1;
		// RS485rwack_3 = 1;
        // 
        // USARTx_SendString(USART2, "\r\nEquipment is now online\r\n");
        // USARTx_SendString(USART3, "\r\nEquipment is now online\r\n");
        // USARTx_SendString(UART4, "\r\nEquipment is now online\r\n");
        // RS485rwack_1 = 0;
		// RS485rwack_2 = 0;
		// RS485rwack_3 = 0;
	}
	else
    {
        if (Tick_Count == 80)
        {
            Tick_Count = 0;
			USARTx_SendString(USART1, "\r\nEquipment waiting to come online\r\n");
            memcpy(Tx_Buffer, "\r\nEquipment waiting to come online\r\n", 37);
            Write_SOCK_Data_Buffer(0, Tx_Buffer, 37);//指定Socket(0~7)发送数据处理
            memset(Tx_Buffer, 0x00, 37);
        }
            // RS485rwack_1 = 1;
		// RS485rwack_2 = 1;
		// RS485rwack_3 = 1;
        
        // USARTx_SendString(USART2, "\r\nEquipment waiting to come online\r\n");
		// USARTx_SendString(USART3, "\r\nEquipment waiting to come online\r\n");
		// USARTx_SendString(UART4, "\r\nEquipment waiting to come online\r\n");
		// RS485rwack_1 = 0;
		// RS485rwack_2 = 0;
		// RS485rwack_3 = 0;
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
        Delay_ms(10);
        Tick_Count++;

        Address_Check();    //检查是否为指定服务器IP端口发送
        UpLine_Mesg();
        Get_NetParam(Rx_Buffer);
        Get_USARTParam(Rx_Buffer);

    } while (check);  //如果收到上线指令且服务器IP、端口正确，退出此循环，进入主循环
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
        memset(Tx_Buffer, 0x00, 35);
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
        memset(Tx_Buffer, 0x00, 200);
        Delay_ms(50);
		__set_FAULTMASK(1);//软件复位stm32
		NVIC_SystemReset();
    }
}

/*********************************************************
 * @brief 网络参数获取函数
 * @return None
 *********************************************************/
void Get_NetParam(uint8_t* BUF)
{
    uint8_t NetParam_Str[110];
    //查询网络参数
    if ((strstr((const char*)BUF + 8, "funcNetGetParam")))
    {   /*输出格式：设备IP，服务器IP，网关，子网掩码，服务器端口，设备端口，设备MAC*/
        sprintf((char*)NetParam_Str, "UDPNowParam:\r\n%d.%d.%d.%d,\r\n%d.%d.%d.%d,\r\n%d.%d.%d.%d,\r\n%d.%d.%d.%d,\r\n%d,\r\n%d,\r\n%02x:%02x:%02x:%02x:%02x:%02x",
            IP_Addr[0], IP_Addr[1], IP_Addr[2], IP_Addr[3],/*设备IP*/
            S0_DIP[0], S0_DIP[1], S0_DIP[2], S0_DIP[3],/*服务器IP*/
            Gateway_IP[0], Gateway_IP[1], Gateway_IP[2], Gateway_IP[3],/*网关*/
            Sub_Mask[0], Sub_Mask[1], Sub_Mask[2], Sub_Mask[3],/*子网掩码*/
            S0_DPort[0] * 256 + S0_DPort[1],/*服务器IP端口*/
            S0_Port[0] * 256 + S0_Port[1],/*设备端口*/
            Phy_Addr[0], Phy_Addr[1], Phy_Addr[2], Phy_Addr[3], Phy_Addr[4], Phy_Addr[5]);/*设备MAC*/
        memcpy(Tx_Buffer, NetParam_Str, strlen((const char*)NetParam_Str));
        Write_SOCK_Data_Buffer(0, Tx_Buffer, strlen((const char*)NetParam_Str));//指定Socket(0~7)发送数据处理
        memset(Tx_Buffer, 0x00, 110);
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
        OCEP_OUT_1 = !data;
        OCEP_State[0] = data;

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OUT_2 = !data;
        OCEP_State[1] = data;

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OUT_3 = !data;
        OCEP_State[2] = data;

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OUT_4 = !data;
        OCEP_State[3] = data;

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OUT_5 = !data;
        OCEP_State[4] = data;

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OUT_6 = !data;
        OCEP_State[5] = data;

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OUT_7 = !data;
        OCEP_State[6] = data;

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OUT_8 = !data;
        OCEP_State[7] = data;
    }

    if ((strstr((const char*)BUF + 8, "funcNetOCEPSet")))
    {
        data = 0x00;
        p = (uint8_t*)((strstr((const char*)BUF + 8, "funcNetOCEPSet")) + strlen("funcNetOCEPSet"));

        p = (u8*)(strstr((const char*)p, ":") + strlen(":"));
        data = atoi((const char*)p);

        switch (data)
        {
        case 1:
            p = (u8*)(strstr((const char*)p, ",") + strlen(","));
            data = atoi((const char*)p);
            OCEP_OUT_1 = !data;
            OCEP_State[0] = data;
            break;
        case 2:
            p = (u8*)(strstr((const char*)p, ",") + strlen(","));
            data = atoi((const char*)p);
            OCEP_OUT_2 = !data;
            OCEP_State[1] = data;
            break;
        case 3:
            p = (u8*)(strstr((const char*)p, ",") + strlen(","));
            data = atoi((const char*)p);
            OCEP_OUT_3 = !data;
            OCEP_State[2] = data;
            break;
        case 4:
            p = (u8*)(strstr((const char*)p, ",") + strlen(","));
            data = atoi((const char*)p);
            OCEP_OUT_4 = !data;
            OCEP_State[3] = data;
            break;
        case 5:
            p = (u8*)(strstr((const char*)p, ",") + strlen(","));
            data = atoi((const char*)p);
            OCEP_OUT_5 = !data;
            OCEP_State[4] = data;
            break;
        case 6:
            p = (u8*)(strstr((const char*)p, ",") + strlen(","));
            data = atoi((const char*)p);
            OCEP_OUT_6 = !data;
            OCEP_State[5] = data;
            break;
        case 7:
            p = (u8*)(strstr((const char*)p, ",") + strlen(","));
            data = atoi((const char*)p);
            OCEP_OUT_7 = !data;
            OCEP_State[6] = data;
            break;
        case 8:
            p = (u8*)(strstr((const char*)p, ",") + strlen(","));
            data = atoi((const char*)p);
            OCEP_OUT_8 = !data;
            OCEP_State[7] = data;
            break;
        }
    }
    AT24CXX_Write(0, OCEP_State, 8);
}

/*********************************************************
 * @brief 继电器控制函数
 * @param {uint8_t} *BUF —— 控制命令，串口接收
 *        示例：funcNetOCEPCtrl,1,0,1,1,0,1,0,0 —— 1号继电器开，2号继电器关......
 * @return None
 *********************************************************/
void funcWIFI_OCEPCtrl(uint8_t* BUF)
{
    uint8_t* p;
    uint8_t data;

    if ((strstr((const char*)BUF, "funcWIFIOCEPCtrl")))
    {
        data = 0x00;
        p = (uint8_t*)((strstr((const char*)BUF, "funcWIFIOCEPCtrl")) + strlen("funcWIFIOCEPCtrl"));

        p = (u8*)(strstr((const char*)p, ":") + strlen(":"));
        data = atoi((const char*)p);
        OCEP_OUT_1 = !data;
        OCEP_State[0] = data;

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OUT_2 = !data;
        OCEP_State[1] = data;

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OUT_3 = !data;
        OCEP_State[2] = data;

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OUT_4 = !data;
        OCEP_State[3] = data;

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OUT_5 = !data;
        OCEP_State[4] = data;

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OUT_6 = !data;
        OCEP_State[5] = data;

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OUT_7 = !data;
        OCEP_State[6] = data;

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        data = atoi((const char*)p);
        OCEP_OUT_8 = !data;
        OCEP_State[7] = data;
    }

    if ((strstr((const char*)BUF, "funcWIFIOCEPSet")))
    {
        data = 0x00;
        p = (uint8_t*)((strstr((const char*)BUF, "funcWIFIOCEPSet")) + strlen("funcWIFIOCEPSet"));

        p = (u8*)(strstr((const char*)p, ":") + strlen(":"));
        data = atoi((const char*)p);

        switch (data)
        {
        case 1:
            p = (u8*)(strstr((const char*)p, ",") + strlen(","));
            data = atoi((const char*)p);
            OCEP_OUT_1 = !data;
            OCEP_State[0] = data;
            break;
        case 2:
            p = (u8*)(strstr((const char*)p, ",") + strlen(","));
            data = atoi((const char*)p);
            OCEP_OUT_2 = !data;
            OCEP_State[1] = data;
            break;
        case 3:
            p = (u8*)(strstr((const char*)p, ",") + strlen(","));
            data = atoi((const char*)p);
            OCEP_OUT_3 = !data;
            OCEP_State[2] = data;
            break;
        case 4:
            p = (u8*)(strstr((const char*)p, ",") + strlen(","));
            data = atoi((const char*)p);
            OCEP_OUT_4 = !data;
            OCEP_State[3] = data;
            break;
        case 5:
            p = (u8*)(strstr((const char*)p, ",") + strlen(","));
            data = atoi((const char*)p);
            OCEP_OUT_5 = !data;
            OCEP_State[4] = data;
            break;
        case 6:
            p = (u8*)(strstr((const char*)p, ",") + strlen(","));
            data = atoi((const char*)p);
            OCEP_OUT_6 = !data;
            OCEP_State[5] = data;
            break;
        case 7:
            p = (u8*)(strstr((const char*)p, ",") + strlen(","));
            data = atoi((const char*)p);
            OCEP_OUT_7 = !data;
            OCEP_State[6] = data;
            break;
        case 8:
            p = (u8*)(strstr((const char*)p, ",") + strlen(","));
            data = atoi((const char*)p);
            OCEP_OUT_8 = !data;
            OCEP_State[7] = data;
            break;
        }
    }
    AT24CXX_Write(0, OCEP_State, 8);
}

/**
 * @brief UDP转发至指定RS232
 * @param {uint8_t*} BUF —— 控制命令，网络接收
 * @return None
 */
void funcNet_MesgToRS(uint8_t* BUF)
{
    uint8_t* udp_rx_buf;
	uint16_t i;
    uint8_t data;
    uint8_t mesg_Str[20];

    if ((strstr((const char*)BUF + 8, "funcNetMesgToRS")))//命令检测
    {
        data = BUF[24] - '0';//获取232通道

        if (data == 1 || data == 2 || data == 3)
        {
            udp_rx_buf = (uint8_t*)malloc((w5500_rx_length-8) * sizeof(uint8_t));//创建动态内存
            for (i = 26;i <= (w5500_rx_length-8) + 25;i++)//将UDP接收数据存入转发数组中
            {
                udp_rx_buf[i - 26] = BUF[i];
            }
            if (data == 1)
            {
                USARTx_SendArray(USART2, udp_rx_buf, w5500_rx_length-26);//通过232发送 
            }
            if (data == 2)
            {
                USARTx_SendArray(USART3, udp_rx_buf, w5500_rx_length-26);//通过232发送
            }
            if (data == 3)
            {
                USARTx_SendArray(UART4, udp_rx_buf, w5500_rx_length-26);//通过232发送
            }
        }
        else
        {
            sprintf((char*)mesg_Str, "Channel is not exist");
            memcpy(Tx_Buffer, mesg_Str, strlen((const char*)mesg_Str));
            Write_SOCK_Data_Buffer(0, Tx_Buffer, strlen((const char*)mesg_Str));//指定Socket(0~7)发送数据处理
            memset(Tx_Buffer, 0x00, 50);
        }
        
        memset(Rx_Buffer,0,2048);//清空UDP缓存
        free(udp_rx_buf);//释放内存
    }
}

void funcWIFI_MesgToRS(uint8_t* BUF)
{
    uint8_t* wifi_rx_buf;
    uint16_t i;
    uint8_t data;
    uint8_t mesg_Str[20];

    if ((strstr((const char*)BUF, "funcWIFIMesgToRS")))//命令检测
    {
        data = BUF[17] - '0';//获取232通道

        if (data == 1 || data == 2 || data == 3)
        {
            wifi_rx_buf = (uint8_t*)malloc(rx1_cnt);//创建动态内存
            for (i = 19;i <= (rx1_cnt)+18;i++)//将UDP接收数据存入转发数组中
            {
                wifi_rx_buf[i - 19] = BUF[i];
            }
            if (data == 1)
            {
                USARTx_SendArray(USART2, wifi_rx_buf, rx1_cnt - 19);//通过232发送 
            }
            if (data == 2)
            {
                USARTx_SendArray(USART3, wifi_rx_buf, rx1_cnt - 19);//通过232发送
            }
            if (data == 3)
            {
                USARTx_SendArray(UART4, wifi_rx_buf, rx1_cnt - 19);//通过232发送
            }
        }
        else
        {
            sprintf((char*)mesg_Str, "Channel is not exist");
            memcpy(Tx_Buffer, mesg_Str, strlen((const char*)mesg_Str));
            USARTx_SendString(USART1, (char*)wifi_rx_buf);
            memset(Tx_Buffer, 0x00, 50);
        }
        free(wifi_rx_buf);//释放内存
    }
}

void funcNet_MesgToUDP(uint8_t * UDP_BUF,uint8_t* BUF_FROM_RS)
{
    uint8_t data_length = 0;
    uint8_t* p;
    uint8_t wifi_tx_buf[500];
    static uint8_t net_open_channel[3] = { 0,0,0 };
    static uint8_t wifi_open_channel[3] = { 0,0,0 };

    if (strstr((const char*)UDP_BUF + 8, "funcNetOpenRStoUDP"))
    {
        Send_Mode = 1;
        p = (uint8_t*)((strstr((const char*)UDP_BUF + 8, "funcNetOpenRStoUDP")) + strlen("funcNetOpenRStoUDP"));

        p = (u8*)(strstr((const char*)p, ":") + strlen(":"));
        net_open_channel[0] = atoi((const char*)p);//通道开关

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        net_open_channel[1] = atoi((const char*)p);//通道开关

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        net_open_channel[2] = atoi((const char*)p);//通道开关
    }

    if (strstr((const char*)UDP_BUF, "funcWIFIOpenRStoUDP"))
    {
        Send_Mode = 2;
        p = (uint8_t*)((strstr((const char*)UDP_BUF, "funcWIFIOpenRStoUDP")) + strlen("funcWIFIOpenRStoUDP"));

        p = (u8*)(strstr((const char*)p, ":") + strlen(":"));
        wifi_open_channel[0] = atoi((const char*)p);//通道开关

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        wifi_open_channel[1] = atoi((const char*)p);//通道开关

        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        wifi_open_channel[2] = atoi((const char*)p);//通道开关
    }

    if (USART_Channel == 2 && (net_open_channel[0] == 1 || wifi_open_channel[0] == 1))
    {
        data_length = rx2_cnt;
    }
    else if (USART_Channel == 3 && (net_open_channel[1] == 1 || wifi_open_channel[1] == 1))
    {
        data_length = rx3_cnt;
    }
    else if (USART_Channel == 4 && (net_open_channel[2] == 1 || wifi_open_channel[2] == 1))
    {
        data_length = rx4_cnt;
    }

    if (USART_Channel && (net_open_channel[0] || net_open_channel[1] || net_open_channel[2]
        || wifi_open_channel[0] || wifi_open_channel[1] || wifi_open_channel[2]))
    {
        if (Send_Mode == 1)
        {
            sprintf((char*)Tx_Buffer, "Message from RS%d:", USART_Channel - 1);
            memcpy(Tx_Buffer + 17, BUF_FROM_RS, data_length + 17);
            Write_SOCK_Data_Buffer(0, Tx_Buffer, data_length + 17);//指定Socket(0~7)发送数据处理
            memset(Tx_Buffer, 0x00, data_length + 17);
        }
        else if (Send_Mode == 2)
        {
            sprintf((char*)wifi_tx_buf, "Message from RS%d:", USART_Channel - 1);
            memcpy(wifi_tx_buf + 17, BUF_FROM_RS, data_length + 17);
            USARTx_SendString(USART1, (char *)wifi_tx_buf);
            memset(wifi_tx_buf, 0x00, data_length + 17);
        }

	}
	
}

void funcNet_SetUARTParam(uint8_t* BUF)
{
    uint8_t* p;
    uint8_t tx_buf[30];
    uint8_t channel = 0;
    uint32_t Baudrate = 0;
    uint8_t wordlength = 0;
    float stopbits = 0.0;
    uint8_t Parity = 0;
    uint8_t HardwareFlowControl = 0;
    uint8_t mesg_Str[20];
    uint16_t dataToWrite[2];
    uint16_t dataRead[2];
    uint8_t set_mode;

    if ((strstr((const char*)BUF + 8, "funcNetSetUARTParam")))
    {
        set_mode = 1;
    }
    else if ((strstr((const char*)BUF, "funcWIFISetUARTParam")))
    {
        set_mode = 2;
    }
    else
    {
        set_mode = 0;
    }
    if (set_mode!=0)
    {
        if (set_mode == 1)
        {
            p = (uint8_t*)((strstr((const char*)BUF + 8, "funcNetSetUARTParam")) + strlen("funcNetSetUARTParam"));
        }
        else if (set_mode == 2)
        {
            p = (uint8_t*)((strstr((const char*)BUF, "funcWIFISetUARTParam")) + strlen("funcWIFISetUARTParam"));
        }

        /*********************选择需要配置的串口，关闭中断************************/
        p = (u8*)(strstr((const char*)p, ":") + strlen(":"));
        channel = atoi((const char*)p);
        if (channel == 1)
        {
            USART_ITConfig(USART2, USART_IT_RXNE, DISABLE);//关闭中断
            USART_Cmd(USART2, DISABLE); //失能串口
        }
        else if (channel == 2)
        {
            USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);//关闭中断
            USART_Cmd(USART3, DISABLE); //失能串口
        }
        else if (channel == 3)
        {
            USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);//关闭中断
            USART_Cmd(UART4, DISABLE); //失能串口 
        }
        
        /*******************配置波特率********************/
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        Baudrate = atoi((const char*)p);

        /*******************配置字长********************/
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        wordlength = atoi((const char*)p);

        /*******************配置停止位********************/
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        stopbits = atof((const char*)p);

        /*******************配置检验位********************/
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        Parity = atof((const char*)p);


        /*******************配置硬件流控********************/
        p = (u8*)(strstr((const char*)p, ",") + strlen(","));
        HardwareFlowControl = atoi((const char*)p);


        if ((channel >= 1 && channel <= 3) && (wordlength >= 0 && wordlength <= 1)&&
            (stopbits >= 0 && stopbits <= 3) && (Parity >= 0 && Parity <= 2) &&
            (HardwareFlowControl >= 0 && HardwareFlowControl <= 3))
        {
            switch (channel)
            {
            case 1:
                Buad_Tab[0] = Baudrate;
                dataToWrite[0] = (uint16_t)(Buad_Tab[0] & 0xFFFF);         // 最低 16 位
                dataToWrite[1] = (uint16_t)((Buad_Tab[0] >> 16) & 0xFFFF);  // 高 16 位
                AT24C512_Write2Byte(67, dataToWrite, 2);
                USART2_Config.USART_Param_Index[0] = wordlength;
                USART2_Config.USART_Param_Index[1] = stopbits;
                USART2_Config.USART_Param_Index[2] = Parity;
                USART2_Config.USART_Param_Index[3] = HardwareFlowControl;
                AT24C512_Read2Byte(67, dataRead, 2);
                Buad_Tab[0] = ((uint32_t)dataRead[1] << 16) | (uint32_t)dataRead[0];
                AT24CXX_Write(55, USART2_Config.USART_Param_Index, 4);
                AT24CXX_Read(55, USART2_Config.USART_Param_Index, 4);
                USART2_Init(Buad_Tab[0]);
                // sprintf((char*)mesg_Str, "Set Param Finish!\r\n");
                // USARTx_SendArray(USART2, mesg_Str, 20);//通过232发送 
                break;

            case 2:
                Buad_Tab[1] = Baudrate;
                dataToWrite[0] = (uint16_t)(Buad_Tab[1] & 0xFFFF);         // 最低 16 位
                dataToWrite[1] = (uint16_t)((Buad_Tab[1] >> 16) & 0xFFFF);  // 高 16 位
                AT24C512_Write2Byte(71, dataToWrite, 2);
                USART3_Config.USART_Param_Index[0] = wordlength;
                USART3_Config.USART_Param_Index[1] = stopbits;
                USART3_Config.USART_Param_Index[2] = Parity;
                USART3_Config.USART_Param_Index[3] = HardwareFlowControl;
                AT24C512_Read2Byte(71, dataRead, 2);
                Buad_Tab[1] = ((uint32_t)dataRead[1] << 16) | (uint32_t)dataRead[0];
                AT24CXX_Write(59, USART3_Config.USART_Param_Index, 4);
                AT24CXX_Read(59, USART3_Config.USART_Param_Index, 4);
                USART3_Init(Buad_Tab[1]);
                // sprintf((char*)mesg_Str, "Set Param Finish!\r\n");
                // USARTx_SendArray(USART3, mesg_Str, 20);//通过232发送 
                break;

            case 3:
                Buad_Tab[2] = Baudrate;
                dataToWrite[0] = (uint16_t)(Buad_Tab[2] & 0xFFFF);         // 最低 16 位
                dataToWrite[1] = (uint16_t)((Buad_Tab[2] >> 16) & 0xFFFF);  // 高 16 位
                AT24C512_Write2Byte(75, dataToWrite, 2);
                UART4_Config.USART_Param_Index[0] = wordlength;
                UART4_Config.USART_Param_Index[1] = stopbits;
                UART4_Config.USART_Param_Index[2] = Parity;
                UART4_Config.USART_Param_Index[3] = HardwareFlowControl;
                AT24C512_Read2Byte(75, dataRead, 2);
                Buad_Tab[2] = ((uint32_t)dataRead[1] << 16) | (uint32_t)dataRead[0];
                AT24CXX_Write(63, UART4_Config.USART_Param_Index, 4);
                AT24CXX_Read(63, UART4_Config.USART_Param_Index, 4);
                USART2_Init(Buad_Tab[2]);
                // sprintf((char*)mesg_Str, "Set Param Finish!\r\n");
                // USARTx_SendArray(UART4, mesg_Str, 20);//通过232发送 
                break;

            default:
                break;
            }
            
            
            if (set_mode == 1)
            {
                sprintf((char*)mesg_Str, "Set Param Finish!\r\n");
                memcpy(Tx_Buffer, mesg_Str, strlen((const char*)mesg_Str));
                Write_SOCK_Data_Buffer(0, Tx_Buffer, strlen((const char*)mesg_Str));//指定Socket(0~7)发送数据处理
                memset(Tx_Buffer, 0x00, 20);
            }
            else if (set_mode == 2)
            {
                sprintf((char*)mesg_Str, "Set Param Finish!\r\n");
                memcpy(tx_buf, mesg_Str, strlen((const char*)mesg_Str));
                USARTx_SendString(USART1, (char*)tx_buf);
                memset(tx_buf, 0x00, 20);
            }
                
        }
        else
        {
            
            if (set_mode == 1)
            {
                sprintf((char*)mesg_Str, "Param Error!\r\n");
                memcpy(Tx_Buffer, mesg_Str, strlen((const char*)mesg_Str));
                Write_SOCK_Data_Buffer(0, Tx_Buffer, strlen((const char*)mesg_Str));//指定Socket(0~7)发送数据处理
                memset(Tx_Buffer, 0x00, 15);
            }
            else if (set_mode == 2)
            {
                sprintf((char*)mesg_Str, "Param Error!\r\n");
                memcpy(tx_buf, mesg_Str, strlen((const char*)mesg_Str));
                USARTx_SendString(USART1, (char*)tx_buf);
                memset(tx_buf, 0x00, 15);
            }
        }
    }
}

void Get_USARTParam(uint8_t* BUF)
{
    uint8_t NetParam_Str[100];
    uint8_t tx_buf[100];
    //查询网络参数
    if ((strstr((const char*)BUF + 8, "funcNetGetRSParam")))
    {   
        sprintf((char*)NetParam_Str,
            "RSNowParam:\r\nRS_CH1{%d,%d,%d,%d,%d}\r\nRS_CH2{%d,%d,%d,%d,%d}\r\nRS_CH3{%d,%d,%d,%d,%d}\r\n",
            Buad_Tab[0], USART2_Config.USART_Param_Index[0], USART2_Config.USART_Param_Index[1], USART2_Config.USART_Param_Index[2], USART2_Config.USART_Param_Index[3],
            Buad_Tab[1], USART3_Config.USART_Param_Index[0], USART3_Config.USART_Param_Index[1], USART3_Config.USART_Param_Index[2], USART3_Config.USART_Param_Index[3],
            Buad_Tab[2], UART4_Config.USART_Param_Index[0], UART4_Config.USART_Param_Index[1], UART4_Config.USART_Param_Index[2], UART4_Config.USART_Param_Index[3]);
        memcpy(Tx_Buffer, NetParam_Str, strlen((const char*)NetParam_Str));
        Write_SOCK_Data_Buffer(0, Tx_Buffer, strlen((const char*)NetParam_Str));//指定Socket(0~7)发送数据处理
        memset(Tx_Buffer, 0x00, 100);
    }

    if ((strstr((const char*)BUF, "funcWIFIGetRSParam")))
    {
        sprintf((char*)NetParam_Str,
            "RSNowParam:\r\nRS_CH1{%d,%d,%d,%d,%d}\r\nRS_CH2{%d,%d,%d,%d,%d}\r\nRS_CH3{%d,%d,%d,%d,%d}\r\n",
            Buad_Tab[0], USART2_Config.USART_Param_Index[0], USART2_Config.USART_Param_Index[1], USART2_Config.USART_Param_Index[2], USART2_Config.USART_Param_Index[3],
            Buad_Tab[1], USART3_Config.USART_Param_Index[0], USART3_Config.USART_Param_Index[1], USART3_Config.USART_Param_Index[2], USART3_Config.USART_Param_Index[3],
            Buad_Tab[2], UART4_Config.USART_Param_Index[0], UART4_Config.USART_Param_Index[1], UART4_Config.USART_Param_Index[2], UART4_Config.USART_Param_Index[3]);
        memcpy(tx_buf, NetParam_Str, strlen((const char*)NetParam_Str));
        USARTx_SendString(USART1, (char*)tx_buf);
        memset(tx_buf, 0x00, 100);
    }
}

void func_Rest(uint8_t* BUF)
{
    if ((strstr((const char*)BUF + 8, "funcNetRest")))
    {
        __set_FAULTMASK(1);//软件复位stm32
        NVIC_SystemReset();
    }
    if ((strstr((const char*)BUF, "funcWIFIRest")))
    {
        __set_FAULTMASK(1);//软件复位stm32
        NVIC_SystemReset();
    }
}