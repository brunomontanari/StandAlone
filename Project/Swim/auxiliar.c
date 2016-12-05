#include "auxiliar.h"
#include "syncsw.h"
#include "main.h"

void GPIO_SetMode(GPIO_TypeDef* GPIOx, uint8 pin, uint8 mode)
{
	uint32 tmp_reg;

	if(pin < 8)
	{
		tmp_reg = GPIOx->CRL;
		tmp_reg &= ~(((u32)0x0F) << ((pin - 0) * 4));
		tmp_reg |= (u32)(mode & 0x0F) << ((pin - 0) * 4);
		GPIOx->CRL = tmp_reg;
	}
	else
	{
		tmp_reg = GPIOx->CRH;
		tmp_reg &= ~(((u32)0x0F) << ((pin - 8) * 4));
		tmp_reg |= (u32)(mode & 0x0F) << ((pin - 8) * 4);
		GPIOx->CRH = tmp_reg;
	}

	if(mode & 0x20)
	{
		if(mode & 0x10)
		{
			GPIOx->BSRR = (((u32)0x01) << pin);
		}
		else
		{
			GPIOx->BRR = (((u32)0x01) << pin);
		}
	}
}

void GPIO_SetPins(GPIO_TypeDef* port, uint16 pin)
{
	(port)->BSRR = (((uint32)1) << (pin));
}

void GPIO_ClrPins(GPIO_TypeDef* port, uint16 pin)
{
	(port)->BRR = (((uint32)1) << (pin));
}

void SWIM_SET(void)
{
	GPIO_SetMode(SYNCSWPWM_GPIO_PORT, SYNCSWPWM_GPIO_PIN, GPIO_MODE_IPU);
}

void SWIM_CLR(void)
{
	GPIO_ClrPins(SYNCSWPWM_GPIO_PORT, SYNCSWPWM_GPIO_PIN);
	GPIO_SetMode(SYNCSWPWM_GPIO_PORT, SYNCSWPWM_GPIO_PIN, GPIO_MODE_OUT_PP);
}

uint32 SWIM_GET(void)
{
	return GPIO_GetInPins(SYNCSWPWM_GPIO_PORT, SYNCSWPWM_GPIO_PIN);
}

uint32 GPIO_GetInPins(GPIO_TypeDef* port, uint16 pin)
{
	return ((port)->IDR & (((uint32)1) << (pin)));
}

// Delay
void DelayUS(volatile uint32 dly)
{
	uint32 dly_tmp;

	while (dly)
	{
		if (dly > DELAYTIMER_MAXDELAY_US)
		{
			dly_tmp = DELAYTIMER_MAXDELAY_US;
		}
		else
		{
			dly_tmp = dly;
		}
		DELAYTIMER_DelayUS(dly_tmp);
		dly -= dly_tmp;
	}
}

void DELAYTIMER_INIT(void)
{
	SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
	SysTick->CTRL |= SysTick_CLKSource_HCLK;
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	SysTick->VAL = 0;
}

void DELAYTIMER_DelayUS_Start(uint32 us)
{
	SysTick->LOAD = (us) * _SYS_FREQUENCY;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

uint32 DELAYTIMER_DelayUS_IsReady()
{
	 return (SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk);
}

void DELAYTIMER_DelayUS_End()
{
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	SysTick->VAL = 0;
}

void DELAYTIMER_DelayUS(uint32 us)
{
	DELAYTIMER_DelayUS_Start(us);
	while (!DELAYTIMER_DelayUS_IsReady())
		;
	DELAYTIMER_DelayUS_End();
}

