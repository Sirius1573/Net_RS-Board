/*********************************************************
 * @description: 
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2023-01-13 11:12:46
 *********************************************************/
#ifndef __DATAPACKAGE_H
#define __DATAPACKAGE_H

#include "sys.h"
#include "stm32f10x.h"

extern uint8_t check;

void Address_Check(void);
void Online_Reminder(void);
void Set_NetParam(uint8_t *BUF);
void Get_NetParam(uint8_t* BUF);
void funcNet_OCEPCtrl(uint8_t* BUF);
void funcNet_MesgToRS(uint8_t* BUF);
void funcNet_MesgToUDP(uint8_t* UDP_BUF, uint8_t* RS_BUF);
void funcNet_SetUARTParam(uint8_t* BUF);
void Get_USARTParam(uint8_t* BUF);

void funcWIFI_OCEPCtrl(uint8_t* BUF);
void funcWIFI_MesgToRS(uint8_t* BUF);

void func_Rest(uint8_t* BUF);


#endif
