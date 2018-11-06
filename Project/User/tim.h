/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private variables ---------------------------------------------------------*/
extern volatile uint32_t cnt_ms;

/* Private function prototypes -----------------------------------------------*/
void TIM3_Config(void);
void TIM4_Config(void);
void TIM3_Update_Callback(void);