/*********************************************************
 * @description: 
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2023-04-11 20:29:48
 *********************************************************/
#ifndef __REMOTE_H
#define __REMOTE_H 
#include "sys.h"   

#define MAX_PULSE_LEN 600
#define IR_RDATA PBin(8)
#define INFRARED_RECEIVE
#define INFRARED_SEND

extern u8 Flag_LearnState;
extern u16 PulseTab[];
extern u16 Last_DataEAddr[1];
extern int start_addr;
extern int data_length;
extern uint16_t arr_length;
void Remote_Reciv_Init(void);//外部中断初始化
void Remote_Send1_Init(void);
void Remote_Send2_Init(void);
void funcNet_StartLearn(uint8_t *BUF);
void funcWIFI_StartLearn(uint8_t* BUF);
void Remote_Send(TIM_TypeDef *TIMx,u16 data[],uint16_t length);
void funcNet_StarSend(uint8_t *BUF);
void funcWIFI_StarSend(uint8_t *BUF);

#endif















