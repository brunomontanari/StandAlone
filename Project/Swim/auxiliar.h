#include "STM32f10x.h"
#include "app_type.h"

#define GPIO_PIN_0					0
#define GPIO_PIN_1					1
#define GPIO_PIN_2					2
#define GPIO_PIN_3					3
#define GPIO_PIN_4					4
#define GPIO_PIN_5					5
#define GPIO_PIN_6					6
#define GPIO_PIN_7					7
#define GPIO_PIN_8					8
#define GPIO_PIN_9					9
#define GPIO_PIN_10					10
#define GPIO_PIN_11					11
#define GPIO_PIN_12					12
#define GPIO_PIN_13					13
#define GPIO_PIN_14					14
#define GPIO_PIN_15					15

#define GPIO_CNF_IN_ANALOG					(0x00 << 2)
#define GPIO_CNF_IN_FLOAT						(0x01 << 2)
#define GPIO_CNF_IN_PULL						(0x02 << 2)
#define GPIO_CNF_OUT_PUSHPULL				(0x00 << 2)
#define GPIO_CNF_OUT_OPENDRAIN			(0x01 << 2)
#define GPIO_CNF_OUT_AF_PUSHPULL		(0x02 << 2)
#define GPIO_CNF_OUT_AF_OPENDRAIN		(0x03 << 2)
#define GPIO_MODE_IN								(0x00 << 0)
#define GPIO_MODE_OUT_10M						(0x01 << 0)
#define GPIO_MODE_OUT_2M						(0x02 << 0)
#define GPIO_MODE_OUT_50M						(0x03 << 0)
#define GPIO_INPUT_PULLUP						0x30
#define GPIO_INPUT_PULLDOWN					0x20

#define GPIO_MODE_IN_FLOATING				(GPIO_MODE_IN | GPIO_CNF_IN_FLOAT)
#define GPIO_MODE_AF_OD							(GPIO_CNF_OUT_AF_OPENDRAIN | GPIO_MODE_OUT_50M)
#define GPIO_MODE_IPU								(GPIO_MODE_IN | GPIO_CNF_IN_PULL | GPIO_INPUT_PULLUP)
#define GPIO_MODE_IPD								(GPIO_MODE_IN | GPIO_CNF_IN_PULL | GPIO_INPUT_PULLDOWN)
#define GPIO_MODE_OUT_PP						(GPIO_CNF_OUT_PUSHPULL | GPIO_MODE_OUT_50M)
#define GPIO_MODE_OUT_OD						(GPIO_CNF_OUT_OPENDRAIN | GPIO_MODE_OUT_50M)

#define DELAYTIMER_MAXDELAY_US			200000
#define SWIM_TIME_BASE_TIMEOUT			10

void GPIO_SetMode(GPIO_TypeDef* GPIOx, uint8 pin, uint8 mode);
void GPIO_SetPins(GPIO_TypeDef* port, uint16 pin);
void GPIO_ClrPins(GPIO_TypeDef* port, uint16 pin);
uint32 GPIO_GetInPins(GPIO_TypeDef* port, uint16 pin);
void SWIM_SET(void);
void SWIM_CLR(void);
uint32 SWIM_GET(void);
void DelayUS(volatile uint32 dly);
void DELAYTIMER_INIT(void);
void DELAYTIMER_DelayUS_Start(uint32 us);
uint32  DELAYTIMER_DelayUS_IsReady();
void DELAYTIMER_DelayUS_End();
void DELAYTIMER_DelayUS(uint32 us);

