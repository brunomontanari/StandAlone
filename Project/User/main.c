/**
******************************************************************************
* @file    Examples/Blink LED/main.c 
* @author  Bruno Montanari
* @version V1.0.0
* @date    05/13/2011
* @brief   Main program body.
******************************************************************************
* @copy
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
*/ 

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
RCC_ClocksTypeDef RCC_ClockFreq;
volatile uint8_t timeout_SWIM = 100;

/* Private function prototypes -----------------------------------------------*/
void delay_ms(unsigned int milisegundos);
void alerta_regravavao_LED(void);
void Error_handler(void);

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Main program.
* @param  None
* @retval None
*/
int main(void)
{
  /* Inicializações */
  GPIO_Config();
  TIM4_Config();
  TIM3_Config();
  TIM_Cmd(TIM3, DISABLE);
  
  while(1)
  {
    while(SWIM_Estado != SWIM_IDLE)
    {
      //Rotina de gravação do STM8
      STM8_program();
      
      if(SWIM_Estado != SWIM_IDLE)
      {
        alerta_regravavao_LED();
        --timeout_SWIM;
      }
      
      if(timeout_SWIM == 0)
      {
        Error_handler();
      }
    }
    
    while(1);
  }
}

//------------------------------------------------------------------------------

/* Subrotina : delay_ms
 * Descrição : Função delay.
 */
void delay_ms(unsigned int milisegundos)
{
  cnt_ms = 0;
  while(milisegundos > cnt_ms);
}

/* Subrotina : alerta_regravavao_LED
 * Descrição : Função delay.
 */
void alerta_regravavao_LED(void)
{
  TIM_Cmd(TIM3, ENABLE);
  GPIO_WriteBit(GPIOA, GPIO_Pin_8, (BitAction)(Bit_SET));//LED LIGADO
  delay_ms(50);
  GPIO_WriteBit(GPIOA, GPIO_Pin_8, (BitAction)(Bit_RESET));//LED DESLIGADO
  delay_ms(50);
  GPIO_WriteBit(GPIOA, GPIO_Pin_8, (BitAction)(Bit_SET));//LED LIGADO
  delay_ms(50);
  GPIO_WriteBit(GPIOA, GPIO_Pin_8, (BitAction)(Bit_RESET));//LED DESLIGADO
  delay_ms(50);
  TIM_Cmd(TIM3, DISABLE);
}

/* Subrotina : Error_handler
 * Descrição :
 */
void Error_handler(void)
{
  TIM_Cmd(TIM3, ENABLE);
  
  while(1)
  {
    GPIO_WriteBit(GPIOA, GPIO_Pin_8, (BitAction)(Bit_SET));//LED LIGADO
    delay_ms(50);
    GPIO_WriteBit(GPIOA, GPIO_Pin_8, (BitAction)(Bit_RESET));//LED DESLIGADO
    delay_ms(50);
  }
}


#ifdef  USE_FULL_ASSERT
/**
* @brief  Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param  file: pointer to the source file name
* @param  line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
* @}
*/

/**
* @}
*/

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
