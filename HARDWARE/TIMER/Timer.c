#include "Timer.h"
uint16_t Timer_Count = 0;

void TIM2_Int_Init(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); //ʱ��ʹ��

    //��ʱ��TIM2��ʼ��
    TIM_TimeBaseStructure.TIM_Period = 999; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
    TIM_TimeBaseStructure.TIM_Prescaler = 719; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); //ʹ��ָ����TIM2�ж�,���������ж�

    //�ж����ȼ�NVIC����
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2�ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  //��ռ���ȼ�0��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;  //�����ȼ�3��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
    NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���


    TIM_Cmd(TIM2, ENABLE);  //ʹ��TIMx					 
}
//��ʱ��2�жϷ������
void TIM2_IRQHandler(void)   //TIM2�ж�
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //���TIM2�����жϷ������
    {
        Timer_Count++;
        if (Timer_Count == 10000)
        {
            Timer_Count = 0;
        }
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  //���TIMx�����жϱ�־ 
    }
}











