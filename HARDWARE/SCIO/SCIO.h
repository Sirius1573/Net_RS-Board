/*********************************************************
 * @description: 
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2022-11-29 16:11:53
 *********************************************************/
#ifndef __SCIO_H
#define __SCIO_H
#include "sys.h"

#define SCI1 PEin(2)	//����1
#define SCI2 PEin(3)	//����2
#define SCO1 PEout(4)	//���1
#define SCO2 PEout(5)	//���2

void SCIO_Init(void);
void SCIO_Ctr(void);

#endif
