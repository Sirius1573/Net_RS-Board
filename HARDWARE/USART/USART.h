/*********************************************************
 * @description: USART.C Header File
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2022-11-13 12:16:34
 *********************************************************/
#ifndef __USART_H
#define __USART_H
#include "sys.h"

#define RS485rwack_1 PEout(7)   //RS485_1读写标志位
#define RS485rwack_2 PEout(8)   //RS485_2读写标志位
#define RS485rwack_3 PEout(9)   //RS485_3读写标志位

extern uint8_t USART2_RxFlag;
extern uint8_t USART3_RxFlag;
extern uint8_t UART4_RxFlag;
extern uint8_t UART5_RxFlag;
extern uint8_t rx1_cnt;
extern uint8_t rx1_buf[50];
extern uint8_t rx2_cnt;
extern uint8_t rx2_buf[50];
extern uint8_t rx3_cnt;
extern uint8_t rx3_buf[50];
extern uint8_t rx4_cnt;
extern uint8_t rx4_buf[50];
extern uint8_t rx5_cnt;
extern uint8_t rx5_buf[50];
extern uint8_t USART_Channel;
extern uint8_t startup_flag;

void USART1_Init(uint32_t bound);
void USART2_Init(uint32_t bound);
void USART3_Init(uint32_t bound);
void UART4_Init(uint32_t bound);
void UART5_Init(uint32_t bound);

void USARTx_SendByte(USART_TypeDef *x, uint8_t Byte);
void USARTx_SendArray(USART_TypeDef* x, uint8_t *Array, uint16_t Length);
void USARTx_SendString(USART_TypeDef* x, char *String);
void USARTx_SendNumber(USART_TypeDef* x, uint32_t Number, uint8_t Length);
void USART5_Printf(char *format, ...);

#endif
