/*********************************************************
 * @description: OCEP.C Header File
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2022-10-15 09:45:21
 *********************************************************/
#ifndef __OCEP_H
#define __OCEP_H
#include "sys.h"

#define OCEP_OUT_8  PDout(7)
#define OCEP_OUT_7  PDout(6)
#define OCEP_OUT_6  PDout(5)
#define OCEP_OUT_5  PDout(4)
#define OCEP_OUT_4  PDout(3)
#define OCEP_OUT_3  PBout(5)
#define OCEP_OUT_2  PBout(4)
#define OCEP_OUT_1  PBout(3)


extern uint8_t OCEP_State[8];
void OCEP_Init(void);
void OCEP_OC(uint8_t x, uint8_t ack);

#endif
