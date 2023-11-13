/*********************************************************
 * @description: delay.c Header File
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2023-03-15 09:51:42
 *********************************************************/
#ifndef __DELAY_H
#define __DELAY_H 			   
#include "sys.h"  

	 
void Delay_init(void);
void Delay_ms(u16 nms);
void Delay_us(u32 nus);

#endif
