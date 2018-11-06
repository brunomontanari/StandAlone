/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* Private variables ---------------------------------------------------------*/
volatile uint32_t cnt_ms = 0;

/**
 * @brief  Configures TIM3 at OVF at 1 ms.
 * @param  None
 * @retval None
 */
void TIM3_Config(void)
{
  TIM_TimeBaseInitTypeDef       TIM3_TimeBaseInitStruct;
  NVIC_InitTypeDef              NVIC_InitStructure;
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
  
  TIM_PrescalerConfig(TIM3, 1, TIM_PSCReloadMode_Immediate);
  TIM_ClearFlag(TIM3, TIM_FLAG_Update);
  
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
  
  TIM3_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM3_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
  TIM3_TimeBaseInitStruct.TIM_Period = 999;
  TIM3_TimeBaseInitStruct.TIM_Prescaler = 71;
  TIM3_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
  
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  
  TIM_TimeBaseInit(TIM3, &TIM3_TimeBaseInitStruct);
  NVIC_Init(&NVIC_InitStructure);
  
  TIM_Cmd(TIM3, ENABLE);
}

/**
 * @brief  Configures TIM4.
 * @param  None
 * @retval None
 */
void TIM4_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /* Enable the TIM4 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  
  NVIC_Init(&NVIC_InitStructure);
}

/* Subrotina : TIM3_Update_Callback
 * Descrição : Rotina de callback da interrupção de update do TIM3.
 */
__weak void TIM3_Update_Callback(void)
{
  ++cnt_ms;
}