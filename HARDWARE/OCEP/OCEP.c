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

    GPIO_SetBits(GPIOD,GPIO_Pin_7);
    GPIO_SetBits(GPIOD,GPIO_Pin_6);
    GPIO_SetBits(GPIOD,GPIO_Pin_5);
    GPIO_SetBits(GPIOD,GPIO_Pin_4);
    GPIO_SetBits(GPIOD,GPIO_Pin_3);
    GPIO_SetBits(GPIOB,GPIO_Pin_5);
    GPIO_SetBits(GPIOB,GPIO_Pin_4);
    GPIO_SetBits(GPIOB,GPIO_Pin_3);
}

/*********************************************************
 * @brief 继电器通断控制
 * @param {uint8_t} x 第x个继电器,取值范围1~8
 * @param {uint8_t} ack 通断标志位,0输出低电平,1输出高电平
 * @return None
 *********************************************************/
void OCEP_OC(uint8_t x,uint8_t ack)
{
    if(ack==0)
    {
        switch (x)
        {
            case 8:GPIO_SetBits(GPIOD,GPIO_Pin_7);break;
            case 7:GPIO_SetBits(GPIOD,GPIO_Pin_6);break;
            case 6:GPIO_SetBits(GPIOD,GPIO_Pin_5);break;
            case 5:GPIO_SetBits(GPIOD,GPIO_Pin_4);break;
            case 4:GPIO_SetBits(GPIOD,GPIO_Pin_3);break;
            case 3:GPIO_SetBits(GPIOB,GPIO_Pin_5);break;
            case 2:GPIO_SetBits(GPIOB,GPIO_Pin_4);break;
            case 1:GPIO_SetBits(GPIOB,GPIO_Pin_3);break;
        }
    }
    else if (ack==1)
    {
        switch (x)
        {
            case 8:GPIO_ResetBits(GPIOD,GPIO_Pin_7);break;
            case 7:GPIO_ResetBits(GPIOD,GPIO_Pin_6);break;
            case 6:GPIO_ResetBits(GPIOD,GPIO_Pin_5);break;
            case 5:GPIO_ResetBits(GPIOD,GPIO_Pin_4);break;
            case 4:GPIO_ResetBits(GPIOD,GPIO_Pin_3);break;
            case 3:GPIO_ResetBits(GPIOB,GPIO_Pin_5);break;
            case 2:GPIO_ResetBits(GPIOB,GPIO_Pin_4);break;
            case 1:GPIO_ResetBits(GPIOB,GPIO_Pin_3);break;
        }
    }
    
}
