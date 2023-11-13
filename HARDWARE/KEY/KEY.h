/*********************************************************
 * @description: 
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2023-01-16 17:01:08
 *********************************************************/
#ifndef __KEY_H
#define __KEY_H 
#include "sys.h"   

#define KEY_Value PBin(1)
extern uint8_t Key_Time;

void Key_Init(void);
uint8_t Key_State(void);
uint8_t Device_Init(void);

#endif















