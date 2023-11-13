/*********************************************************
 * @description: 短路输入输出
 * @author: Sirius
 * @date: File created time
 * @LastEditors: Sirius
 * @LastEditTime: 2022-11-29 15:19:50
 *********************************************************/
#include "stm32f10x.h"
#include "SCIO.h"

/*********************************************************
 * @brief 短路输入输出初始化 
 * @return None
 *********************************************************/
void SCIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_SetBits(GPIOE,GPIO_Pin_5);
    GPIO_SetBits(GPIOE,GPIO_Pin_4);
}

/********************************************************
 * @brief 短路输入、输出测试函数
 * @param None
 * @return None
 ********************************************************/
void SCIO_Ctr(void)
{
    if(SCI1 == 1)
    {
		SCO1 = 0;
    }
    else if(SCI1 == 0)
    {  
		SCO1 = 1;
    }

    if(SCI2 == 1)
    {
		SCO2 = 0;
    }
    else if(SCI2 == 0)
    {
		SCO2 = 1;
    }
}
