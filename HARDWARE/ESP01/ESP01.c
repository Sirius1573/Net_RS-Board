#include "stm32f10x.h"
#include "ESP01.h"
#include "USART.h"
#include "delay.h"
#include "Timer.h"
#include "w5500.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

char WIFI_SSID[WIFI_SSID_MAX_LENGTH];
char WIFI_PASSWORD[WIFI_PASSWORD_MAX_LENGTH];
char ESP_IP[15];
char ESP_MAC[17];
uint16_t ESP_Port;
uint8_t WIFI_IP[4];
uint16_t WIFI_Port;
char temp_buf[20];
char temp_buf_two[4];
uint8_t Config_State = 0;
uint8_t WIFI_Config_Flag = 0;
char* ptr = NULL;

void ESP_Restore(uint8_t* BUF)
{
    char* p;
    char send_buf[20];

    if (strstr((const char*)BUF + 8, "funcNetWIFIRestore"))
    {
		Config_State=0;
        USARTx_SendString(USART1, (char*)"+++");
        Delay_ms(50);
        USARTx_SendString(USART1, (char*)"+++");
        Delay_ms(50);
        USARTx_SendString(USART1, (char*)"AT+RESTORE\r\n");
    }
}

void Set_WIFIParam(uint8_t* UDP_RXBUF,uint8_t* WIFI_RXBUF)
{
    
    char send_buf[150];
    _Bool SwitchFlag = 0;
    char* p;
	char* p1;

    if (strstr((const char*)UDP_RXBUF + 8, "funcNetSetWIFIParam"))
    {
        WIFI_Config_Flag = 1;
		Config_State=0;
		
        p = (char *)strtok_r((char*)UDP_RXBUF+8, (const char*)":",&ptr);

		p = (char *)strtok_r(NULL, (const char*)",",&ptr);//��ȡWIFI SSID
		strcpy(WIFI_SSID, p);

		p = (char *)strtok_r(NULL, (const char*)",",&ptr);//��ȡWIFI PassWord
		strcpy(WIFI_PASSWORD, p);

        p = (char*)strtok_r(NULL, (const char*)".", &ptr);//��ȡWIFI IP
        WIFI_IP[0] = atoi((const char*)p);
        p = (char*)strtok_r(NULL, (const char*)".", &ptr);//��ȡWIFI IP
        WIFI_IP[1] = atoi((const char*)p);
        p = (char*)strtok_r(NULL, (const char*)".", &ptr);//��ȡWIFI IP
        WIFI_IP[2] = atoi((const char*)p);
        p = (char*)strtok_r(NULL, (const char*)",", &ptr);//��ȡWIFI IP
        WIFI_IP[3] = atoi((const char*)p);

		p = (char *)strtok_r(NULL, (const char*)",",&ptr);//��ȡWIFI PORT
        WIFI_Port = atoi((const char*)p);

		p = (char *)strtok_r(NULL, (const char*)",",&ptr);//��ȡESP PORT
        ESP_Port = atoi((const char*)p);
    }
    if (WIFI_Config_Flag == 1)//���յ�WIFI��������
    {
        if (Config_State == 0)//״̬0������+++�˳�͸��
        {
            if (strstr((const char*)WIFI_RXBUF, "+++"))//���յ���+++������ATģʽ
            {
				USARTx_SendString(USART1, (char*)"AT+RESTORE\r\n");
                Timer_Count = 10;
				Config_State = 1;//����״̬1
            }
            else
            {
                if (Timer_Count >= 2)//���ͼ��10ms
                {
                    Timer_Count = 0;
                    USARTx_SendString(USART1, (char*)"+++");
					Delay_ms(100);
					USARTx_SendString(USART1, (char*)"+++");
                }
            }
        }
        if (Config_State == 1)//״̬1������ģ��ΪSTAģʽ
        {
            if (strstr((const char*)WIFI_RXBUF, "AT+CWMODE=1\r\n\r\nOK\r\n"))//�յ����ţ����óɹ�
            {
                Config_State = 2;//����״̬2
                Timer_Count = 800;//����ֵ��Ϊ500�Ա������һ״̬
            }
            else
            {
                if (Timer_Count >= 10)//���ͼ��100ms
                {
					Timer_Count = 0;
					Delay_ms(800);
                    USARTx_SendString(USART1, (char*)"AT+CWMODE=1\r\n");
                    memset(send_buf, 0x00, 14);
                }
            }
        }
        if (Config_State == 2)//״̬2�����ӵ�ָ��WIFI
        {
            if (strstr((const char*)WIFI_RXBUF, "WIFI GOT IP\r\n"))//��ȡIP
            {
                Config_State = 3;//����״̬3
            }
            else
            {
                if (Timer_Count >= 800)//���ͼ��8s
                {
                    Timer_Count = 0;
                    sprintf((char*)send_buf, (const char *)"AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_SSID, WIFI_PASSWORD);
                    USARTx_SendString(USART1, (char*)send_buf);
                    memset(send_buf, 0x00, 50);
                }
            }
        }
        if (Config_State == 3)//״̬3�����ӵ�ָ��WIFI�ɹ�
        {
            if (strstr((const char*)WIFI_RXBUF, "\r\nOK\r\n"))//���ӳɹ�
            {
                Config_State = 4;
                Timer_Count = 10;
                sprintf((char*)send_buf, "WIFI Connect Finish\r\n");
                memcpy(Tx_Buffer, send_buf, strlen((const char*)send_buf));
                Write_SOCK_Data_Buffer(0, Tx_Buffer, strlen((const char*)send_buf));//ָ��Socket(0~7)�������ݴ���
                memset(Tx_Buffer, 0x00, 50);
            }
        }
        if (Config_State == 4)//״̬4����ѯģ��IP
        {
            if (strstr((const char*)WIFI_RXBUF, "AT+CIFSR"))//�յ����ţ���ѯ�ɹ�
            {
                Config_State = 5;
            }
            else
            {
                if (Timer_Count >= 10)//���ͼ��100ms
                {
                    Timer_Count = 0;
                    USARTx_SendString(USART1, (char*)"AT+CIFSR\r\n");
                    memset(send_buf, 0x00, 10);
                }
            }
        }
        if (Config_State == 5)
        {
            if (strstr((const char*)WIFI_RXBUF, "\r\nOK\r\n"))//�յ����ţ���ѯ�ɹ�
            {
                
                p1 = (char*)strtok_r((char*)WIFI_RXBUF, (const char*)":", &ptr);

                p1 = (char*)strtok_r(NULL, (const char*)"\"", &ptr);//��ȡWIFI SSID
                p1 = (char*)strtok_r(NULL, (const char*)"\"", &ptr);//��ȡWIFI PassWord
                strcpy(ESP_IP, p1);

                p1 = (char*)strtok_r(NULL, (const char*)"\"", &ptr);//��ȡWIFI PassWord
                p1 = (char*)strtok_r(NULL, (const char*)"\"", &ptr);//��ȡWIFI PassWord
				strcpy(ESP_MAC, p1);

                Config_State = 6;
                Timer_Count = 25;
            }
        }
        if (Config_State == 6)
        {
            if (strstr((const char*)WIFI_RXBUF, "CONNECT"))//�յ����ţ���ѯ�ɹ�
            {
                Config_State = 7;
                sprintf((char*)send_buf, "UDP Connect Finish\r\n");
                memcpy(Tx_Buffer, send_buf, strlen((const char*)send_buf));
                Write_SOCK_Data_Buffer(0, Tx_Buffer, strlen((const char*)send_buf));//ָ��Socket(0~7)�������ݴ���
                memset(Tx_Buffer, 0x00, 50);
                Delay_ms(10);
                Timer_Count = 10;
            }
            else
            {
                if (Timer_Count >= 25)//���ͼ��100ms
                {
                    Timer_Count = 0;
                    sprintf((char*)send_buf, (const char*)"AT+CIPSTART=\"UDP\",\"%d.%d.%d.%d\",%d,%d\r\n", WIFI_IP[0], WIFI_IP[1], WIFI_IP[2], WIFI_IP[3], WIFI_Port, ESP_Port);
                    USARTx_SendString(USART1, (char*)send_buf);
                    memset(send_buf, 0x00, 80);
                }
            }
        }
        if (Config_State == 7)
        {
            if (strstr((const char*)WIFI_RXBUF, "\r\nOK\r\n"))//���ӳɹ�
            {
                Config_State = 8;
                Timer_Count = 10;
            }
        }
        if (Config_State == 8)
        {
            if (strstr((const char*)WIFI_RXBUF, "AT+CIPMODE=1"))//�յ����ţ���ѯ�ɹ�
            {
                Config_State = 9;
                Timer_Count = 10;
            }
            else
            {
                if (Timer_Count >= 10)//���ͼ��100ms
                {
                    Timer_Count = 0;
                    USARTx_SendString(USART1, "AT+CIPMODE=1\r\n");
                    memset(send_buf, 0x00, 14);
                }
            }
        }
        if (Config_State == 9)
        {
            if (strstr((const char*)WIFI_RXBUF, "\r\nOK\r\n"))//���ӳɹ�
            {
                Config_State = 10;
                Timer_Count = 25;
            }
        }
        if (Config_State == 10)
        {
            if (strstr((const char*)WIFI_RXBUF, "AT+SAVETRANSLINK"))//
            {
                Config_State = 11;
                Delay_ms(10);
                Timer_Count = 10;
            }
            else
            {
                if (Timer_Count >= 25)
                {
                    Timer_Count = 0;
                    sprintf((char*)send_buf, (const char*)"AT+SAVETRANSLINK=1,\"%d.%d.%d.%d\",%d,\"UDP\",%d\r\n", WIFI_IP[0], WIFI_IP[1], WIFI_IP[2], WIFI_IP[3], WIFI_Port, ESP_Port);
                    USARTx_SendString(USART1, (char*)send_buf);
                    memset(send_buf, 0x00, 80);
                }
            }
        }
        if (Config_State == 11)
        {
            if (strstr((const char*)WIFI_RXBUF, "\r\nOK\r\n"))
            {
                Config_State = 12;
                Timer_Count = 10;
            }
        }
        if (Config_State == 12)
        {
            if (strstr((const char*)WIFI_RXBUF, "AT+CIPSEND"))//�յ����ţ���ѯ�ɹ�
            {
                Config_State = 13;
            }
            else
            {
                if (Timer_Count >= 10)//���ͼ��100ms
                {
                    Timer_Count = 0;
                    USARTx_SendString(USART1, "AT+CIPSEND\r\n");
                    memset(send_buf, 0x00, 12);
                }
            }
        }
        if (Config_State == 13)
        {
            if (strstr((const char*)WIFI_RXBUF, "\r\nOK\r\n\r\n>"))
            {
                Config_State = 14;
                Timer_Count = 10;
            }
        }
        if (Config_State == 14)
        {
            sprintf((char*)send_buf,
                "Set WIFIParam Finish!\r\nLocal IP:%s\r\nLocal Port:%d\r\nLocal MAC:%s\r\nServer IP:%d.%d.%d.%d\r\nServer Port:%d",
                ESP_IP, ESP_Port, ESP_MAC, WIFI_IP[0], WIFI_IP[1], WIFI_IP[2], WIFI_IP[3], WIFI_Port);
            memcpy(Tx_Buffer, send_buf, strlen((const char*)send_buf));
            Write_SOCK_Data_Buffer(0, Tx_Buffer, strlen((const char*)send_buf));//ָ��Socket(0~7)�������ݴ���
            memset(Tx_Buffer, 0x00, 150);
            memset(send_buf, 0x00, 150);
            WIFI_Config_Flag = 0;
            Config_State = 0;
        }
    }
}
