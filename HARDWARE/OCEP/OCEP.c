/*********************************************************
 * @description: 继电器控制程序
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2023-02-22 20:57:07
 *********************************************************/
#include "stm32f10x.h"
#include "OCEP.h"
#include "sys.h"
#include "24cxx.h"

uint8_t OCEP_State[8] = { 0,0,0,0,0,0,0,0 };
 /*********************************************************
 * @brief GPIO初始化，默认低电平
 * @return None
 *********************************************************/
void OCEP_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD,ENABLE);
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

//    AT24CXX_Write(0, OCEP_State, 8);
    AT24CXX_Read(0, OCEP_State, 8);
	
    OCEP_OUT_1 = !OCEP_State[0];
    OCEP_OUT_2 = !OCEP_State[1];
    OCEP_OUT_3 = !OCEP_State[2];
    OCEP_OUT_4 = !OCEP_State[3];
    OCEP_OUT_5 = !OCEP_State[4];
    OCEP_OUT_6 = !OCEP_State[5];
    OCEP_OUT_7 = !OCEP_State[6];
    OCEP_OUT_8 = !OCEP_State[7];
}
