#include "STM32f10x.h"
#include "syncsw.h"
#include "auxiliar.h"

//uint8 DmaFlag;

void SYNCSWPWM_OUT_TIMER_INIT(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
  GPIO_InitTypeDef gpio_pwm_config;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	DMA_DeInit(SYNCSWPWM_OUT_TIMER_DMA);
	DMA_StructInit(&DMA_InitStructure);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(SYNCSWPWM_OUT_TIMER->CCR4);
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(SYNCSWPWM_OUT_TIMER_DMA, &DMA_InitStructure);
	DMA_Cmd(SYNCSWPWM_OUT_TIMER_DMA, ENABLE);
	
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 0;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(SYNCSWPWM_OUT_TIMER, &TIM_TimeBaseStructure);
	
	TIM_OCStructInit(&TIM_OCInitStructure);
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 0;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OC4Init(SYNCSWPWM_OUT_TIMER, &TIM_OCInitStructure);
	
	TIM_OC4PreloadConfig(SYNCSWPWM_OUT_TIMER, TIM_OCPreload_Enable);
	TIM_ARRPreloadConfig(SYNCSWPWM_OUT_TIMER, ENABLE);
	TIM_DMACmd(SYNCSWPWM_OUT_TIMER, TIM_DMA_CC4, ENABLE);
	TIM_Cmd(SYNCSWPWM_OUT_TIMER, ENABLE);
	//TIM_CtrlPWMOutputs(SYNCSWPWM_OUT_TIMER, ENABLE);
	
	GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, ENABLE);

  gpio_pwm_config.GPIO_Pin = SYNCSW_Out_Pin;
  gpio_pwm_config.GPIO_Mode = GPIO_Mode_AF_OD;
  gpio_pwm_config.GPIO_Speed = GPIO_Speed_50MHz; // Highest speed for max resolution
  GPIO_Init(SYNCSW_OUT_PORT, &gpio_pwm_config); // Do the setup
}

void SYNCSWPWM_PORT_FINI(void)
{
	GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, DISABLE);
}

void SYNCSWPWM_PORT_OD_FINI(void)
{
	GPIO_SetMode(SYNCSW_OUT_PORT, SYNCSW_OUT_PIN, GPIO_MODE_IN_FLOATING);
	SYNCSWPWM_PORT_FINI();
}

void SYNCSWPWM_OUT_TIMER_FINI(void)
{
	TIM_DeInit(SYNCSWPWM_OUT_TIMER);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, DISABLE);
	DMA_DeInit(SYNCSWPWM_OUT_TIMER_DMA);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);
}

void SYNCSWPWM_IN_TIMER_FINI(void)
{
	TIM_DeInit(SYNCSWPWM_IN_TIMER);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, DISABLE);
	DMA_DeInit(SYNCSWPWM_IN_TIMER_DMA);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, DISABLE);
}

void SYNCSWPWM_IN_TIMER_INIT(void)
{
  uint16_t tmpccmr1 = 0, tmpccer = 0, tmpsmcr = 0;

	DMA_InitTypeDef DMA_InitStructure;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	DMA_DeInit(SYNCSWPWM_IN_TIMER_DMA);
	DMA_StructInit(&DMA_InitStructure);
			//Link DMA1
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(SYNCSWPWM_IN_TIMER->CCR1);
			//Input from Peripheral
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 0;
			//Mesmo periferico
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
			//Dados colocados em buffer ponteiro auto incremental
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
			//16bits
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
			//Buffer (circular normal) Normal
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
			//Alta prioridade
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//	DMA_ITConfig(SYNCSWPWM_IN_TIMER_DMA,DMA_IT_TE,ENABLE);
	DMA_Init(SYNCSWPWM_IN_TIMER_DMA, &DMA_InitStructure);
	DMA_Cmd(SYNCSWPWM_IN_TIMER_DMA, ENABLE);
	
//	TIM_ICStructInit(&TIM_ICInitStructure);
		//canal 2	-> Selection TI2
//	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
		//edge positivo
//	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
		//TI2 -> IC2
//	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
//	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
//	TIM_ICInitStructure.TIM_ICFilter = 0;
//	TIM_PWMIConfig(SYNCSWPWM_IN_TIMER, &TIM_ICInitStructure);

		// TI1 Configuration
		// TI2 Configuration
		// Disable the Channel 1,2 : Reset the CC1E Bit, CC2E Bit
  SYNCSWPWM_IN_TIMER->CCER &= (uint16_t)~((uint16_t)(TIM_CCER_CC1E|TIM_CCER_CC2E));
  tmpccmr1 = SYNCSWPWM_IN_TIMER->CCMR1;
  tmpccer = SYNCSWPWM_IN_TIMER->CCER;

  	// Select the Input and set the filter, prescale
  tmpccmr1 &= (uint16_t)(((uint16_t)~((uint16_t)(TIM_CCMR1_CC1S|TIM_CCMR1_CC2S|TIM_CCMR1_IC1F|TIM_CCMR1_IC2F|TIM_ICPSC_DIV8|(TIM_ICPSC_DIV8<<8)))));
		//IC1->TI2 (indirect)
		//IC2->TI2 (direct)
  tmpccmr1 |= (uint16_t)((TIM_ICSelection_DirectTI << 8)|(TIM_ICSelection_IndirectTI));
		//prescale - sem prescala
	tmpccmr1 |= (uint16_t)((TIM_ICPSC_DIV1<<8)|(TIM_ICPSC_DIV1));

	  // Select the Polarity
  tmpccer &= (uint16_t)(~(uint16_t)(TIM_CCER_CC1P)); 		//CC1 rising 0
  tmpccer |= (uint16_t)((uint16_t)(TIM_CCER_CC2P));		//CC2 falling 1

		// Write to TIMx CCMR1 and CCER registers
  SYNCSWPWM_IN_TIMER->CCMR1 = tmpccmr1 ;
  SYNCSWPWM_IN_TIMER->CCER = tmpccer;

		//Trigger TI2FP2
  tmpsmcr = SYNCSWPWM_IN_TIMER->SMCR;
	  /* Reset the TS, SMS, MSM Bits */
  tmpsmcr &= (uint16_t)(~((uint16_t)TIM_SMCR_TS|TIM_SMCR_SMS|TIM_SMCR_MSM));
  	/* Set the Input Trigger source */
  	/* Select the Slave Mode */
	tmpsmcr |= (TIM_TS_TI2FP2|TIM_SlaveMode_Reset|TIM_MasterSlaveMode_Enable);

		/* Write to TIMx SMCR */
  SYNCSWPWM_IN_TIMER->SMCR = tmpsmcr;

		//Set the CC1E Bit, CC2E Bit
  SYNCSWPWM_IN_TIMER->CCER |=  (uint16_t)(TIM_CCER_CC1E | TIM_CCER_CC2E);

//	TIM_DMACmd(SYNCSWPWM_IN_TIMER, TIM_DMA_CC2, ENABLE);
	SYNCSWPWM_IN_TIMER->DIER |= TIM_DMA_CC1;
	
	TIM_Cmd(SYNCSWPWM_IN_TIMER, ENABLE);

}

void __INLINE SYNCSWPWM_IN_TIMER_DMA_INIT(uint8 load, uint16* a)
{
	SYNCSWPWM_IN_TIMER_DMA->CCR &= ~1;
	SYNCSWPWM_IN_TIMER_DMA->CNDTR = (load);
	SYNCSWPWM_IN_TIMER_DMA->CMAR = (uint32_t)(a);
	SYNCSWPWM_IN_TIMER_DMA->CCR |= 1;
}

uint16 SYNCSWPWM_IN_TIMER_DMA_WAIT(uint16 dly)
{
	while((!(DMA1->ISR & DMA1_FLAG_TC1)) && --dly)
		 ;
	DMA1->IFCR = DMA1_FLAG_TC1;
	return dly;
}

void SYNCSWPWM_OUT_TIMER_SetCycle (uint16 cycle)
{
	SYNCSWPWM_OUT_TIMER->ARR = (cycle);
	SYNCSWPWM_OUT_TIMER->EGR = TIM_PSCReloadMode_Immediate;
}

uint16 SYNCSWPWM_OUT_TIMER_GetCycle (void)
{	
	return SYNCSWPWM_OUT_TIMER->ARR;
}

void SYNCSWPWM_OUT_TIMER_DMA_INIT(uint16 load, uint16* a)
{
	SYNCSWPWM_OUT_TIMER->EGR = TIM_PSCReloadMode_Immediate;
	SYNCSWPWM_OUT_TIMER_DMA->CCR &= ~1;
	SYNCSWPWM_OUT_TIMER_DMA->CNDTR = load;
	SYNCSWPWM_OUT_TIMER_DMA->CMAR = (uint32_t)(a);
	SYNCSWPWM_OUT_TIMER_DMA->CCR |= 1;
}

void __INLINE SYNCSWPWM_OUT_TIMER_DMA_WAIT(void)
{
	while(!(DMA1->ISR & DMA1_FLAG_TC7))		//channel 7
		;
	DMA1->IFCR = DMA1_FLAG_TC7;
}

void SYNCSW_DIR_INIT(void)
{
	GPIO_SetMode(SYNCSW_OUT_PORT, SYNCSW_OUT_PIN, GPIO_MODE_IN_FLOATING);
	GPIO_SetMode(SYNCSWPWM_GPIO_PORT, SYNCSWPWM_GPIO_PIN, GPIO_MODE_IPU);
}

void SW_RST_DIR_INIT(void)
{
	GPIO_SetMode(SW_RST_PORT, SW_RST_PIN, GPIO_MODE_IPD);
}

void SW_RST_SET(void)
{
	GPIO_SetPins(SW_RST_PORT, SW_RST_PIN);
}

void SW_RST_CLR(void)	
{
	GPIO_ClrPins(SW_RST_PORT, SW_RST_PIN);
}
