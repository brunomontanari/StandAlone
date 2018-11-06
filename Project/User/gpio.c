/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* Private variables ---------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStructure;

void GPIO_Config(void)
{
  /* Enable GPIOA, GPIOB, RCC_APB2Periph_GPIO_KEY_BUTTON and AFIO clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
  
  /* Configure as output push-pull:
	  PA.13 (JTMS/SWDAT)
	  PA.14 (JTCK/SWCLK)
	  PA.15 (JTDI)
	  PA.8  (LED)
   */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15 | GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* Configure PB.03 (JTDO) and PB.04 (JTRST) as output push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}