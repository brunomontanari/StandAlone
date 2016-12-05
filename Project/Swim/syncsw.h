#include "STM32f10x.h"
#include "app_type.h"

#define SYNCSW_OUT_PORT		GPIOB      	//PB11 - SWIM
#define SYNCSW_OUT_PIN		GPIO_PIN_11
#define SYNCSW_Out_Pin		GPIO_Pin_11
#define SYNCSWPWM_GPIO_PORT	GPIOB		//PB8 - SWIM
#define SYNCSWPWM_GPIO_PIN	GPIO_PIN_8
#define SYNCSWPWM_GPIO_Pin	GPIO_Pin_8
#define SYNCSWPWM_OUT_TIMER	TIM2         	//PB11 - SWIM
#define SYNCSWPWM_OUT_TIMER_DMA	DMA1_Channel7
#define SYNCSWPWM_IN_TIMER	TIM4
#define SYNCSWPWM_IN_TIMER_DMA	DMA1_Channel1   //Channel4 - 1
#define SW_RST_PORT		GPIOB
#define SW_RST_PIN		GPIO_PIN_6


void SYNCSWPWM_OUT_TIMER_INIT(void);
void SYNCSWPWM_PORT_FINI(void);
void SYNCSWPWM_PORT_OD_FINI(void);
void SYNCSWPWM_OUT_TIMER_FINI(void);
void SYNCSWPWM_IN_TIMER_FINI(void);
void SYNCSWPWM_IN_TIMER_INIT(void);
void SYNCSWPWM_IN_TIMER_DMA_INIT(uint8 load, uint16* a);
uint16 SYNCSWPWM_IN_TIMER_DMA_WAIT(uint16 dly);
void SYNCSWPWM_OUT_TIMER_SetCycle (uint16 cycle);
uint16 SYNCSWPWM_OUT_TIMER_GetCycle (void);
void SYNCSWPWM_OUT_TIMER_DMA_INIT(uint16 load, uint16* a);
void SYNCSWPWM_OUT_TIMER_DMA_WAIT(void);
void SYNCSW_DIR_INIT(void);
void SW_RST_DIR_INIT(void);
void SW_RST_SET(void);
void SW_RST_CLR(void);	

