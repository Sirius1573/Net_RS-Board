/*********************************************************
 * @description:该文件包含W5500相关函数及参量
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2023-04-11 20:56:56
 *********************************************************/

#include "stm32f10x.h"
#include "stm32f10x_spi.h"				
#include "W5500.h"	
#include <string.h>
#include "delay.h"
#include "24cxx.h"



// unsigned int Timer2_Counter=0; //Timer2定时器计数变量(ms)
// unsigned int W5500_Send_Delay_Counter=0; //W5500发送延时计数变量(ms)

/***************----- 网络参数变量定义 -----***************/
unsigned char Gateway_IP[4] = {192, 168, 3, 1};//网关IP地址 
unsigned char Sub_Mask[4] = {255, 255, 255, 0};	//子网掩码 
unsigned char Phy_Addr[6] = {0x00, 0x08, 0xdc, 0x11, 0x11, 0x11};	//物理地址(MAC) 
unsigned char IP_Addr[4] = {192, 168, 3, 2};  //本机IP地址 

unsigned char S0_Port[2] = {0x04, 0xB0};	//端口0的端口号 
unsigned char S0_DIP[4] = {192, 168, 3, 100};	//端口0目的IP地址 
unsigned char S0_DPort[2] = {0x1E, 0x6B};	//端口0目的端口号

unsigned char UDP_DIPR[4] = {192, 168, 3, 100};	//UDP(广播)模式,目的主机IP地址
unsigned char UDP_DPORT[2] = {0x1E, 0x6B};	//UDP(广播)模式,目的主机端口号

/***************----- 默认网络参数 -----***************/
unsigned char Default_Gateway_IP[4] = {192, 168, 3, 1};//默认网关
unsigned char Default_SubMask[4] = {255, 255, 255, 0};//默认子网掩码
unsigned char Default_PhyAddr[6] = {0x00, 0x08, 0xdc, 0x11, 0x11, 0x11};//默认设备MAC
unsigned char Default_IPAddr[4] = {192, 168, 3, 2};//默认设备IP
unsigned char Default_S0Port[2] = {0x04, 0xB0};//默认设备端口号
unsigned char Default_S0DIP[4] = {192, 168, 3, 100};//默认服务器IP
unsigned char Default_S0DPort[2] = {0x1E, 0x6B};//默认服务器端口号
unsigned char w5500_rx_length = 0;

/***************----- 端口的运行模式 -----***************/
unsigned char S0_Mode =3;	//端口0的运行模式,0:TCP服务器模式,1:TCP客户端模式,2:UDP(广播)模式
#define TCP_SERVER	0x00	//TCP服务器模式
#define TCP_CLIENT	0x01	//TCP客户端模式 
#define UDP_MODE	0x02	//UDP(广播)模式 

/***************----- 端口的运行状态 -----***************/
unsigned char S0_State =0;	//端口0状态记录,1:端口完成初始化,2端口完成连接(可以正常传输数据) 
#define S_INIT		0x01	//端口完成初始化 
#define S_CONN		0x02	//端口完成连接,可以正常传输数据 

/***************----- 端口收发数据的状态 -----***************/
unsigned char S0_Data;		//端口0接收和发送数据的状态,1:端口接收到数据,2:端口发送数据完成 
#define S_RECEIVE	 0x01	//端口接收到一个数据包 
#define S_TRANSMITOK 0x02	//端口发送一个数据包完成 

/***************----- 端口数据缓冲区 -----***************/
unsigned char Rx_Buffer[2048];	//端口接收数据缓冲区 
unsigned char Tx_Buffer[2048];	//端口发送数据缓冲区 


unsigned char W5500_Interrupt;	//W5500中断标志(0:无中断,1:有中断)

/*******************************************************************************
* 函数名  : Load_Net_Parameters
* 描述    : 从EEPROM中读取网络参数，存储顺序：设备IP,服务器IP，网关，子网掩码，服务器端口，设备端口，MAC地址
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 网关、掩码、物理地址、本机IP地址、端口号、目的IP地址、目的端口号、端口工作模式
* 默认设备IP     :192.168.3.2
* 默认设备端口   :1200(0x04B0)
* 默认设备MAC    :00:08:dc:11:11:11
* 默认服务器IP   :192.168.3.100
* 默认服务器端口 :7787(0x1E6B)
* 默认子网掩码   :255.255.255.0
* 默认网关       :192.168.3.1
*******************************************************************************/
void Load_Net_Parameters(void)
{
	unsigned char eeprom_test[2];
	//存储顺序：设备IP,服务器IP，网关，子网掩码，服务器端口，设备端口，MAC地址!
	//存储顺序：设备IP,服务器IP，网关，子网掩码，服务器端口，设备端口，MAC地址!
	//存储顺序：设备IP,服务器IP，网关，子网掩码，服务器端口，设备端口，MAC地址!
	/****写默认参数****/
    // AT24CXX_Write(0, Default_IPAddr, 4);
	// AT24CXX_Write(4, Default_S0DIP, 4);
	// AT24CXX_Write(8, Default_Gateway_IP, 4);
	// AT24CXX_Write(12, Default_SubMask, 4);
	// AT24CXX_Write(16, Default_S0DPort, 2);
	// AT24CXX_Write(18, Default_S0Port, 2);
	// AT24CXX_Write(20, Default_PhyAddr, 6);
	
	AT24CXX_Read(26, eeprom_test, 2);
	
	if(eeprom_test[0]==0xff && eeprom_test[1]==0xff)
	{
		AT24CXX_Write(26, IP_Addr, 4);//写本机IP地址
		AT24CXX_Write(30, S0_DIP, 4);//写服务器IP地址
		AT24CXX_Write(34, Gateway_IP, 4);//写网关
		AT24CXX_Write(38, Sub_Mask, 4);//写子网掩码
		AT24CXX_Write(42, S0_DPort, 2);//写服务器端口
		AT24CXX_Write(44, S0_Port, 2);//写设备端口
		AT24CXX_Write(46, Phy_Addr, 6);//写设备MAC地址
	}

    AT24CXX_Read(26, IP_Addr, 4);//读本机IP地址
    AT24CXX_Read(30, S0_DIP, 4);//读服务器IP地址
    AT24CXX_Read(34, Gateway_IP, 4);//读网关
    AT24CXX_Read(38, Sub_Mask, 4); //读子网掩码
    AT24CXX_Read(42, S0_DPort, 2);//读服务器端口
	AT24CXX_Read(44, S0_Port, 2);//读设备端口
	AT24CXX_Read(46, Phy_Addr, 6);//读设备MAC地址

	UDP_DIPR[0] = S0_DIP[0];	//UDP(广播)模式,目的主机IP地址
	UDP_DIPR[1] = S0_DIP[1];
	UDP_DIPR[2] = S0_DIP[2];
	UDP_DIPR[3] = S0_DIP[3];

	UDP_DPORT[0] = S0_DPort[0];	//UDP(广播)模式,目的主机端口号
	UDP_DPORT[1] = S0_DPort[1];

	S0_Mode=UDP_MODE;//加载端口0的工作模式,UDP模式
}

/*******************************************************************************
* 函数名  : W5500_GPIO_Configuration
* 描述    : W5500 GPIO初始化配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void W5500_GPIO_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;	

	/* W5500_RST引脚初始化配置(PC5) */
	GPIO_InitStructure.GPIO_Pin  = W5500_RST;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(W5500_RST_PORT, &GPIO_InitStructure);
	GPIO_ResetBits(W5500_RST_PORT, W5500_RST);
}

/*******************************************************************************
* 函数名  : SPI_Configuration
* 描述    : W5500 SPI初始化配置(STM32 SPI1)
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void SPI_Configuration(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	SPI_InitTypeDef   	SPI_InitStructure;	   

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO, ENABLE);	

	/* 初始化SCK、MISO、MOSI引脚 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);

	/* 初始化CS引脚 */
	GPIO_InitStructure.GPIO_Pin = W5500_SCS;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(W5500_SCS_PORT, &GPIO_InitStructure);
	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);

	/* 初始化配置STM32 SPI1 */
	SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;	//SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode=SPI_Mode_Master;							//设置为主SPI
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;						//SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;							//时钟悬空低
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;							//数据捕获于第1个时钟沿
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;								//NSS由外部管脚管理
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;	//波特率预分频值为2
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;					//数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial=7;								//CRC多项式为7
	SPI_Init(SPI1,&SPI_InitStructure);									//根据SPI_InitStruct中指定的参数初始化外设SPI1寄存器

	SPI_Cmd(SPI1,ENABLE);	//STM32使能SPI1
}

/*******************************************************************************
* 函数名  : SPI1_Send_Byte
* 描述    : SPI1发送1个字节数据
* 输入    : dat:待发送的数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void SPI1_Send_Byte(unsigned char dat)
{
	SPI_I2S_SendData(SPI1,dat);//写1个字节数据
	while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);//等待数据寄存器空
}

/*******************************************************************************
* 函数名  : SPI1_Send_Short
* 描述    : SPI1发送2个字节数据(16位)
* 输入    : dat:待发送的16位数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void SPI1_Send_Short(unsigned short dat)
{
	SPI1_Send_Byte(dat/256);//写数据高位
	SPI1_Send_Byte(dat);	//写数据低位
}

/*******************************************************************************
* 函数名  : Write_W5500_1Byte
* 描述    : 通过SPI1向指定地址寄存器写1个字节数据
* 输入    : reg:16位寄存器地址,dat:待写入的数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_1Byte(unsigned short reg, unsigned char dat)
{
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平

	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(FDM1|RWB_WRITE|COMMON_R);//通过SPI1写控制字节,1个字节数据长度,写数据,选择通用寄存器
	SPI1_Send_Byte(dat);//写1个字节数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_2Byte
* 描述    : 通过SPI1向指定地址寄存器写2个字节数据
* 输入    : reg:16位寄存器地址,dat:16位待写入的数据(2个字节)
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_2Byte(unsigned short reg, unsigned short dat)
{
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平
		
	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(FDM2|RWB_WRITE|COMMON_R);//通过SPI1写控制字节,2个字节数据长度,写数据,选择通用寄存器
	SPI1_Send_Short(dat);//写16位数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_nByte
* 描述    : 通过SPI1向指定地址寄存器写n个字节数据
* 输入    : reg:16位寄存器地址,*dat_ptr:待写入数据缓冲区指针,size:待写入的数据长度
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_nByte(unsigned short reg, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short i;

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平	
		
	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(VDM|RWB_WRITE|COMMON_R);//通过SPI1写控制字节,N个字节数据长度,写数据,选择通用寄存器

	for(i=0;i<size;i++)//循环将缓冲区的size个字节数据写入W5500
	{
		SPI1_Send_Byte(*dat_ptr++);//写一个字节数据
	}

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_1Byte
* 描述    : 通过SPI1向指定端口寄存器写1个字节数据
* 输入    : s:端口号,reg:16位寄存器地址,dat:待写入的数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_SOCK_1Byte(SOCKET s, unsigned short reg, unsigned char dat)
{
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平	
		
	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(FDM1|RWB_WRITE|(s*0x20+0x08));//通过SPI1写控制字节,1个字节数据长度,写数据,选择端口s的寄存器
	SPI1_Send_Byte(dat);//写1个字节数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_2Byte
* 描述    : 通过SPI1向指定端口寄存器写2个字节数据
* 输入    : s:端口号,reg:16位寄存器地址,dat:16位待写入的数据(2个字节)
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_SOCK_2Byte(SOCKET s, unsigned short reg, unsigned short dat)
{
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平
			
	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(FDM2|RWB_WRITE|(s*0x20+0x08));//通过SPI1写控制字节,2个字节数据长度,写数据,选择端口s的寄存器
	SPI1_Send_Short(dat);//写16位数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_4Byte
* 描述    : 通过SPI1向指定端口寄存器写4个字节数据
* 输入    : s:端口号,reg:16位寄存器地址,*dat_ptr:待写入的4个字节缓冲区指针
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_SOCK_4Byte(SOCKET s, unsigned short reg, unsigned char *dat_ptr)
{
	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平
			
	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(FDM4|RWB_WRITE|(s*0x20+0x08));//通过SPI1写控制字节,4个字节数据长度,写数据,选择端口s的寄存器

	SPI1_Send_Byte(*dat_ptr++);//写第1个字节数据
	SPI1_Send_Byte(*dat_ptr++);//写第2个字节数据
	SPI1_Send_Byte(*dat_ptr++);//写第3个字节数据
	SPI1_Send_Byte(*dat_ptr++);//写第4个字节数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Read_W5500_1Byte
* 描述    : 读W5500指定地址寄存器的1个字节数据
* 输入    : reg:16位寄存器地址
* 输出    : 无
* 返回值  : 读取到寄存器的1个字节数据
* 说明    : 无
*******************************************************************************/
unsigned char Read_W5500_1Byte(unsigned short reg)
{
	unsigned char i;

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平
			
	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(FDM1|RWB_READ|COMMON_R);//通过SPI1写控制字节,1个字节数据长度,读数据,选择通用寄存器

	i=SPI_I2S_ReceiveData(SPI1);
	SPI1_Send_Byte(0x00);//发送一个哑数据
	i=SPI_I2S_ReceiveData(SPI1);//读取1个字节数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为高电平
	return i;//返回读取到的寄存器数据
}

/*******************************************************************************
* 函数名  : Read_W5500_SOCK_1Byte
* 描述    : 读W5500指定端口寄存器的1个字节数据
* 输入    : s:端口号,reg:16位寄存器地址
* 输出    : 无
* 返回值  : 读取到寄存器的1个字节数据
* 说明    : 无
*******************************************************************************/
unsigned char Read_W5500_SOCK_1Byte(SOCKET s, unsigned short reg)
{
	unsigned char i;

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平
			
	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(FDM1|RWB_READ|(s*0x20+0x08));//通过SPI1写控制字节,1个字节数据长度,读数据,选择端口s的寄存器

	i=SPI_I2S_ReceiveData(SPI1);
	SPI1_Send_Byte(0x00);//发送一个哑数据
	i=SPI_I2S_ReceiveData(SPI1);//读取1个字节数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为高电平
	return i;//返回读取到的寄存器数据
}

/*******************************************************************************
* 函数名  : Read_W5500_SOCK_2Byte
* 描述    : 读W5500指定端口寄存器的2个字节数据
* 输入    : s:端口号,reg:16位寄存器地址
* 输出    : 无
* 返回值  : 读取到寄存器的2个字节数据(16位)
* 说明    : 无
*******************************************************************************/
unsigned short Read_W5500_SOCK_2Byte(SOCKET s, unsigned short reg)
{
	unsigned short i;

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平
			
	SPI1_Send_Short(reg);//通过SPI1写16位寄存器地址
	SPI1_Send_Byte(FDM2|RWB_READ|(s*0x20+0x08));//通过SPI1写控制字节,2个字节数据长度,读数据,选择端口s的寄存器

	i=SPI_I2S_ReceiveData(SPI1);
	SPI1_Send_Byte(0x00);//发送一个哑数据
	i=SPI_I2S_ReceiveData(SPI1);//读取高位数据
	SPI1_Send_Byte(0x00);//发送一个哑数据
	i*=256;
	i+=SPI_I2S_ReceiveData(SPI1);//读取低位数据

	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为高电平
	return i;//返回读取到的寄存器数据
}

/*******************************************************************************
* 函数名  : Read_SOCK_Data_Buffer
* 描述    : 从W5500接收数据缓冲区中读取数据
* 输入    : s:端口号,*dat_ptr:数据保存缓冲区指针
* 输出    : 无
* 返回值  : 读取到的数据长度,rx_size个字节
* 说明    : 无
*******************************************************************************/
unsigned short Read_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr)
{
	unsigned short rx_size;
	unsigned short offset, offset1;
	unsigned short i;
	unsigned char j;

	rx_size=Read_W5500_SOCK_2Byte(s,Sn_RX_RSR);
	if(rx_size==0) return 0;//没接收到数据则返回
	if (rx_size > 1460) rx_size = 1460;
	w5500_rx_length = rx_size;

	offset=Read_W5500_SOCK_2Byte(s,Sn_RX_RD);
	offset1=offset;
	offset&=(S_RX_SIZE-1);//计算实际的物理地址

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平

	SPI1_Send_Short(offset);//写16位地址
	SPI1_Send_Byte(VDM|RWB_READ|(s*0x20+0x18));//写控制字节,N个字节数据长度,读数据,选择端口s的寄存器
	j=SPI_I2S_ReceiveData(SPI1);
	
	if((offset+rx_size)<S_RX_SIZE)//如果最大地址未超过W5500接收缓冲区寄存器的最大地址
	{
		for(i=0;i<rx_size;i++)//循环读取rx_size个字节数据
		{
			SPI1_Send_Byte(0x00);//发送一个哑数据
			j=SPI_I2S_ReceiveData(SPI1);//读取1个字节数据
			*dat_ptr=j;//将读取到的数据保存到数据保存缓冲区
			dat_ptr++;//数据保存缓冲区指针地址自增1
		}
	}
	else//如果最大地址超过W5500接收缓冲区寄存器的最大地址
	{
		offset=S_RX_SIZE-offset;
		for(i=0;i<offset;i++)//循环读取出前offset个字节数据
		{
			SPI1_Send_Byte(0x00);//发送一个哑数据
			j=SPI_I2S_ReceiveData(SPI1);//读取1个字节数据
			*dat_ptr=j;//将读取到的数据保存到数据保存缓冲区
			dat_ptr++;//数据保存缓冲区指针地址自增1
		}
		GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平

		GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平

		SPI1_Send_Short(0x00);//写16位地址
		SPI1_Send_Byte(VDM|RWB_READ|(s*0x20+0x18));//写控制字节,N个字节数据长度,读数据,选择端口s的寄存器
		j=SPI_I2S_ReceiveData(SPI1);

		for(;i<rx_size;i++)//循环读取后rx_size-offset个字节数据
		{
			SPI1_Send_Byte(0x00);//发送一个哑数据
			j=SPI_I2S_ReceiveData(SPI1);//读取1个字节数据
			*dat_ptr=j;//将读取到的数据保存到数据保存缓冲区
			dat_ptr++;//数据保存缓冲区指针地址自增1
		}
	}
	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平

	offset1+=rx_size;//更新实际物理地址,即下次读取接收到的数据的起始地址
	Write_W5500_SOCK_2Byte(s, Sn_RX_RD, offset1);
	Write_W5500_SOCK_1Byte(s, Sn_CR, RECV);//发送启动接收命令
	return rx_size;//返回接收到数据的长度
}

/*******************************************************************************
* 函数名  : Write_SOCK_Data_Buffer
* 描述    : 将数据写入W5500的数据发送缓冲区
* 输入    : s:端口号,*dat_ptr:数据保存缓冲区指针,size:待写入数据的长度
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size)
{
	unsigned short offset,offset1;
	unsigned short i;

	//如果是UDP模式,可以在此设置目的主机的IP和端口号
	if((Read_W5500_SOCK_1Byte(s,Sn_MR)&0x0f) != SOCK_UDP)//如果Socket打开失败
	{		
		Write_W5500_SOCK_4Byte(s, Sn_DIPR, UDP_DIPR);//设置目的主机IP  		
		Write_W5500_SOCK_2Byte(s, Sn_DPORTR, UDP_DPORT[0]*256+UDP_DPORT[1]);//设置目的主机端口号				
	}

	offset=Read_W5500_SOCK_2Byte(s,Sn_TX_WR);
	offset1=offset;
	offset&=(S_TX_SIZE-1);//计算实际的物理地址

	GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平

	SPI1_Send_Short(offset);//写16位地址
	SPI1_Send_Byte(VDM|RWB_WRITE|(s*0x20+0x10));//写控制字节,N个字节数据长度,写数据,选择端口s的寄存器

	if((offset+size)<S_TX_SIZE)//如果最大地址未超过W5500发送缓冲区寄存器的最大地址
	{
		for(i=0;i<size;i++)//循环写入size个字节数据
		{
			SPI1_Send_Byte(*dat_ptr++);//写入一个字节的数据		
		}
	}
	else//如果最大地址超过W5500发送缓冲区寄存器的最大地址
	{
		offset=S_TX_SIZE-offset;
		for(i=0;i<offset;i++)//循环写入前offset个字节数据
		{
			SPI1_Send_Byte(*dat_ptr++);//写入一个字节的数据
		}
		GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平

		GPIO_ResetBits(W5500_SCS_PORT, W5500_SCS);//置W5500的SCS为低电平

		SPI1_Send_Short(0x00);//写16位地址
		SPI1_Send_Byte(VDM|RWB_WRITE|(s*0x20+0x10));//写控制字节,N个字节数据长度,写数据,选择端口s的寄存器

		for(;i<size;i++)//循环写入size-offset个字节数据
		{
			SPI1_Send_Byte(*dat_ptr++);//写入一个字节的数据
		}
	}
	GPIO_SetBits(W5500_SCS_PORT, W5500_SCS); //置W5500的SCS为高电平

	offset1+=size;//更新实际物理地址,即下次写待发送数据到发送数据缓冲区的起始地址
	Write_W5500_SOCK_2Byte(s, Sn_TX_WR, offset1);
	Write_W5500_SOCK_1Byte(s, Sn_CR, SEND);//发送启动发送命令				
}

/*******************************************************************************
* 函数名  : W5500_Hardware_Reset
* 描述    : 硬件复位W5500
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : W5500的复位引脚保持低电平至少500us以上,才能重围W5500
*******************************************************************************/
void W5500_Hardware_Reset(void)
{
	GPIO_ResetBits(W5500_RST_PORT, W5500_RST);//复位引脚拉低
	Delay_ms(50);
	GPIO_SetBits(W5500_RST_PORT, W5500_RST);//复位引脚拉高
	Delay_ms(200);
	// while((Read_W5500_1Byte(PHYCFGR)&LINK)==0);//等待以太网连接完成
}

/*******************************************************************************
* 函数名  : W5500_Init
* 描述    : 初始化W5500寄存器函数
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 在使用W5500之前，先对W5500初始化
*******************************************************************************/
void W5500_Init(void)
{
	u8 i=0;

	Write_W5500_1Byte(MR, RST);//软件复位W5500,置1有效,复位后自动清0
	Delay_ms(10);//延时10ms,自己定义该函数

	//设置网关(Gateway)的IP地址,Gateway_IP为4字节unsigned char数组,自己定义 
	//使用网关可以使通信突破子网的局限，通过网关可以访问到其它子网或进入Internet
	Write_W5500_nByte(GAR, Gateway_IP, 4);
			
	//设置子网掩码(MASK)值,SUB_MASK为4字节unsigned char数组,自己定义
	//子网掩码用于子网运算
	Write_W5500_nByte(SUBR,Sub_Mask,4);		
	
	//设置物理地址,PHY_ADDR为6字节unsigned char数组,自己定义,用于唯一标识网络设备的物理地址值
	//该地址值需要到IEEE申请，按照OUI的规定，前3个字节为厂商代码，后三个字节为产品序号
	//如果自己定义物理地址，注意第一个字节必须为偶数
	Write_W5500_nByte(SHAR,Phy_Addr,6);		

	//设置本机的IP地址,IP_ADDR为4字节unsigned char数组,自己定义
	//注意，网关IP必须与本机IP属于同一个子网，否则本机将无法找到网关
	Write_W5500_nByte(SIPR,IP_Addr,4);		
	
	//设置发送缓冲区和接收缓冲区的大小，参考W5500数据手册
	for(i=0;i<8;i++)
	{
		Write_W5500_SOCK_1Byte(i,Sn_RXBUF_SIZE, 0x02);//Socket Rx memory size=2k
		Write_W5500_SOCK_1Byte(i,Sn_TXBUF_SIZE, 0x02);//Socket Tx mempry size=2k
	}

	//设置重试时间，默认为2000(200ms) 
	//每一单位数值为100微秒,初始化时值设为2000(0x07D0),等于200毫秒
	Write_W5500_2Byte(RTR, 0x07d0);

	//设置重试次数，默认为8次 
	//如果重发的次数超过设定值,则产生超时中断(相关的端口中断寄存器中的Sn_IR 超时位(TIMEOUT)置“1”)
	Write_W5500_1Byte(RCR,8);
}

/*******************************************************************************
* 函数名  : Detect_Gateway
* 描述    : 检查网关服务器
* 输入    : 无
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 无
*******************************************************************************/
unsigned char Detect_Gateway(void)
{
	unsigned char ip_adde[4];
	ip_adde[0]=IP_Addr[0]+1;
	ip_adde[1]=IP_Addr[1]+1;
	ip_adde[2]=IP_Addr[2]+1;
	ip_adde[3]=IP_Addr[3]+1;

	//检查网关及获取网关的物理地址
	Write_W5500_SOCK_4Byte(0,Sn_DIPR,ip_adde);//向目的地址寄存器写入与本机IP不同的IP值
	Write_W5500_SOCK_1Byte(0,Sn_MR,MR_TCP);//设置socket为TCP模式
	Write_W5500_SOCK_1Byte(0,Sn_CR,OPEN);//打开Socket	
	Delay_ms(5);//延时5ms 	
	
	if(Read_W5500_SOCK_1Byte(0,Sn_SR) != SOCK_INIT)//如果socket打开失败
	{
		Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);//打开不成功,关闭Socket
		return FALSE;//返回FALSE(0x00)
	}

	Write_W5500_SOCK_1Byte(0,Sn_CR,CONNECT);//设置Socket为Connect模式						

	do
	{
		u8 j=0;
		j=Read_W5500_SOCK_1Byte(0,Sn_IR);//读取Socket0中断标志寄存器
		if(j!=0)
		Write_W5500_SOCK_1Byte(0,Sn_IR,j);
		Delay_ms(5);//延时5ms 
		if((j&IR_TIMEOUT) == IR_TIMEOUT)
		{
			return FALSE;	
		}
		else if(Read_W5500_SOCK_1Byte(0,Sn_DHAR) != 0xff)
		{
			Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);//关闭Socket
			return TRUE;							
		}
	}while(1);
}

/*******************************************************************************
* 函数名  : Socket_Init
* 描述    : 指定Socket(0~7)初始化
* 输入    : s:待初始化的端口
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Socket_Init(SOCKET s)
{
	//设置分片长度，参考W5500数据手册，该值可以不修改	
	Write_W5500_SOCK_2Byte(0, Sn_MSSR, 1460);//最大分片字节数=1460(0x5b4)
	//设置指定端口
	switch(s)
	{
		case 0:
			//设置端口0的端口号
			Write_W5500_SOCK_2Byte(0, Sn_PORT, S0_Port[0]*256+S0_Port[1]);
						
			break;

		case 1:
			break;

		case 2:
			break;

		case 3:
			break;

		case 4:
			break;

		case 5:
			break;

		case 6:
			break;

		case 7:
			break;

		default:
			break;
	}
}

/*******************************************************************************
* 函数名  : Socket_Connect
* 描述    : 设置指定Socket(0~7)为客户端与远程服务器连接
* 输入    : s:待设定的端口
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 当本机Socket工作在客户端模式时,引用该程序,与远程服务器建立连接
*			如果启动连接后出现超时中断，则与服务器连接失败,需要重新调用该程序连接
*			该程序每调用一次,就与服务器产生一次连接
*******************************************************************************/
unsigned char Socket_Connect(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_TCP);//设置socket为TCP模式
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);//打开Socket
	Delay_ms(5);//延时5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_INIT)//如果socket打开失败
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//打开不成功,关闭Socket
		return FALSE;//返回FALSE(0x00)
	}
	Write_W5500_SOCK_1Byte(s,Sn_CR,CONNECT);//设置Socket为Connect模式
	return TRUE;//返回TRUE,设置成功
}

/*******************************************************************************
* 函数名  : Socket_Listen
* 描述    : 设置指定Socket(0~7)作为服务器等待远程主机的连接
* 输入    : s:待设定的端口
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 当本机Socket工作在服务器模式时,引用该程序,等等远程主机的连接
*			该程序只调用一次,就使W5500设置为服务器模式
*******************************************************************************/
unsigned char Socket_Listen(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_TCP);//设置socket为TCP模式 
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);//打开Socket	
	Delay_ms(5);//延时5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_INIT)//如果socket打开失败
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//打开不成功,关闭Socket
		return FALSE;//返回FALSE(0x00)
	}	
	Write_W5500_SOCK_1Byte(s,Sn_CR,LISTEN);//设置Socket为侦听模式	
	Delay_ms(5);//延时5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_LISTEN)//如果socket设置失败
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//设置不成功,关闭Socket
		return FALSE;//返回FALSE(0x00)
	}

	return TRUE;

	//至此完成了Socket的打开和设置侦听工作,至于远程客户端是否与它建立连接,则需要等待Socket中断，
	//以判断Socket的连接是否成功。参考W5500数据手册的Socket中断状态
	//在服务器侦听模式不需要设置目的IP和目的端口号
}

/*******************************************************************************
* 函数名  : Socket_UDP
* 描述    : 设置指定Socket(0~7)为UDP模式
* 输入    : s:待设定的端口
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 如果Socket工作在UDP模式,引用该程序,在UDP模式下,Socket通信不需要建立连接
*			该程序只调用一次，就使W5500设置为UDP模式
*******************************************************************************/
unsigned char Socket_UDP(SOCKET s)
{
	Write_W5500_SOCK_1Byte(s,Sn_MR,MR_UDP);//设置Socket为UDP模式*/
	Write_W5500_SOCK_1Byte(s,Sn_CR,OPEN);//打开Socket*/
	Delay_ms(5);//延时5ms
	if(Read_W5500_SOCK_1Byte(s,Sn_SR)!=SOCK_UDP)//如果Socket打开失败
	{
		Write_W5500_SOCK_1Byte(s,Sn_CR,CLOSE);//打开不成功,关闭Socket
		return FALSE;//返回FALSE(0x00)
	}
	else
		return TRUE;

	//至此完成了Socket的打开和UDP模式设置,在这种模式下它不需要与远程主机建立连接
	//因为Socket不需要建立连接,所以在发送数据前都可以设置目的主机IP和目的Socket的端口号
	//如果目的主机IP和目的Socket的端口号是固定的,在运行过程中没有改变,那么也可以在这里设置
}

/*******************************************************************************
* 函数名  : W5500_Interrupt_Process
* 描述    : W5500中断处理程序框架
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void W5500_Interrupt_Process(void)
{
	unsigned char i,j;

	IntDispose:

	i=Read_W5500_1Byte(SIR);//读取端口中断标志寄存器	
	if((i & S0_INT) == S0_INT)//Socket0事件处理 
	{
		j=Read_W5500_SOCK_1Byte(0,Sn_IR);//读取Socket0中断标志寄存器
		Write_W5500_SOCK_1Byte(0,Sn_IR,j);
		if(j&IR_CON)//在TCP模式下,Socket0成功连接 
		{
			S0_State|=S_CONN;//网络连接状态0x02,端口完成连接，可以正常传输数据
		}
		if(j&IR_DISCON)//在TCP模式下Socket断开连接处理
		{
			Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);//关闭端口,等待重新打开连接 
			Socket_Init(0);		//指定Socket(0~7)初始化,初始化端口0
			S0_State=0;//网络连接状态0x00,端口连接失败
		}
		if(j&IR_SEND_OK)//Socket0数据发送完成,可以再次启动S_tx_process()函数发送数据 
		{
			S0_Data|=S_TRANSMITOK;//端口发送一个数据包完成 
		}
		if(j&IR_RECV)//Socket接收到数据,可以启动S_rx_process()函数 
		{
			S0_Data|=S_RECEIVE;//端口接收到一个数据包
		}
		if(j&IR_TIMEOUT)//Socket连接或数据传输超时处理 
		{
			Write_W5500_SOCK_1Byte(0,Sn_CR,CLOSE);// 关闭端口,等待重新打开连接 			
			S0_State=0;//网络连接状态0x00,端口连接失败
		}
	}

	if(Read_W5500_1Byte(SIR) != 0) 
		goto IntDispose;
}


/*******************************************************************************
* 函数名  : W5500_Socket_Set
* 描述    : W5500端口初始化配置
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 分别设置4个端口,根据端口工作模式,将端口置于TCP服务器、TCP客户端或UDP模式.
*			从端口状态字节Socket_State可以判断端口的工作情况
*******************************************************************************/
void W5500_Socket_Set(void)
{
	if(S0_State==0)//端口0初始化配置
	{
		if(S0_Mode==TCP_SERVER)//TCP服务器模式 
		{
			if(Socket_Listen(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else if(S0_Mode==TCP_CLIENT)//TCP客户端模式 
		{
			if(Socket_Connect(0)==TRUE)
				S0_State=S_INIT;
			else
				S0_State=0;
		}
		else//UDP模式 
		{
			if(Socket_UDP(0)==TRUE)
				S0_State=S_INIT|S_CONN;
			else
				S0_State=0;
		}
	}
}

/*******************************************************************************
* 函数名  : Process_Socket_Data
* 描述    : W5500接收并发送接收到的数据
* 输入    : s:端口号
* 输出    : 无
* 返回值  : 无
* 说明    : 本过程先调用S_rx_process()从W5500的端口接收数据缓冲区读取数据,
*			然后将读取的数据从Rx_Buffer拷贝到Temp_Buffer缓冲区进行处理。
*			处理完毕，将数据从Temp_Buffer拷贝到Tx_Buffer缓冲区。调用S_tx_process()
*			发送数据。
*******************************************************************************/
void Process_Socket_Data(SOCKET s)
{
	Read_SOCK_Data_Buffer(s, Rx_Buffer);
	UDP_DIPR[0] = S0_DIP[0];
	UDP_DIPR[1] = S0_DIP[1];
	UDP_DIPR[2] = S0_DIP[2];
	UDP_DIPR[3] = S0_DIP[3];

	UDP_DPORT[0] = S0_DPort[0];
	UDP_DPORT[1] = S0_DPort[1];
	
}

