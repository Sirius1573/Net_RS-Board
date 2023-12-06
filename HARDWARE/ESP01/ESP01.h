/*********************************************************
 * @description: 
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2022-11-29 16:11:53
 *********************************************************/
#ifndef __ESP01_H
#define __ESP01_H
#include "sys.h"

#define WIFI_SSID_MAX_LENGTH 20
#define WIFI_PASSWORD_MAX_LENGTH 20

extern uint8_t WIFI_Config_Flag;

void Set_WIFIParam(uint8_t* UDP_RXBUF, uint8_t* WIFI_RXBUF);
void ESP_Restore(uint8_t* BUF);

#endif
