/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       SWIM.c                                                    *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    SWIM interface implementation file                        *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2008-11-07:     created(by SimonQian)                             *
 **************************************************************************/
#include "SWIM.h"
#include "app_type.h"
#include "main.h"
#include "syncsw.h"
#include "auxiliar.h"

//static uint8 SWIM_Inited = 0;
uint8 SWIM_Inited = 0;
static uint16 SWIM_PULSE_0;
static uint16 SWIM_PULSE_1;
static uint16 SWIM_PULSE_Threshold;
// max length is 1(header)+8(data)+1(parity)+1(ack from target)+1(empty)
static uint16 SWIM_DMA_IN_Buffer[150];
static uint16 SWIM_DMA_OUT_Buffer[15];
//static uint16 SWIM_clock_div = 0;
uint16 SWIM_clock_div = 0;

#define SWIM_MAX_DLY		0x0FFFF
//#define SWIM_MAX_RESEND_CNT	10
#define SWIM_CMD_BITLEN		3
#define SWIM_CMD_SRST		0x00
#define SWIM_CMD_ROTF		0x01
#define SWIM_CMD_WOTF		0x02

#define SWIM_SYNC_CYCLES	128

void SWIM_Init()
{
  if (!SWIM_Inited)
  {
    SWIM_Inited = 1;
    SYNCSWPWM_OUT_TIMER_INIT();
  }
}

void SWIM_Fini()
{
  SYNCSWPWM_PORT_OD_FINI();
  SYNCSWPWM_OUT_TIMER_FINI();
  SYNCSWPWM_IN_TIMER_FINI();
  SWIM_Inited = 0;
}

void SWIM_EnableClockInput(void)
{
    SWIM_clock_div = 0;
    SYNCSWPWM_IN_TIMER_INIT();
}

uint8 SWIM_EnterProgMode(void)
{
  uint8 i;
  uint32 dly;

  SWIM_CLR();
  DelayUS(1000);

  for (i = 0; i < 4; i++)
  {
    SWIM_SET();
    DelayUS(500);
    SWIM_CLR();
    DelayUS(500);
  }
  for (i = 0; i < 4; i++)
  {
    SWIM_SET();
    DelayUS(250);
    SWIM_CLR();
    DelayUS(250);
  }
  SWIM_SET();

  SYNCSWPWM_IN_TIMER_DMA_INIT(2, SWIM_DMA_IN_Buffer);
  dly = SYNCSWPWM_IN_TIMER_DMA_WAIT((uint16)SWIM_MAX_DLY);

  if (!dly)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

uint8 SWIM_Sync(uint8 mHz)
{
  uint32 dly;
  uint16 clock_div;
  uint16 arr_save;
  
  clock_div = _SYS_FREQUENCY / mHz;
  if ((_SYS_FREQUENCY % mHz) > (mHz / 2))
  {
    clock_div++;
  }
  
  SYNCSWPWM_IN_TIMER_DMA_INIT(2, SWIM_DMA_IN_Buffer);
  
  arr_save = SYNCSWPWM_OUT_TIMER_GetCycle();
  SYNCSWPWM_OUT_TIMER_SetCycle(SWIM_SYNC_CYCLES * clock_div + 1);

  SWIM_DMA_OUT_Buffer[0] = SWIM_SYNC_CYCLES * clock_div;
  SWIM_DMA_OUT_Buffer[1] = 0;
  SYNCSWPWM_OUT_TIMER_DMA_INIT(2, SWIM_DMA_OUT_Buffer);
  SYNCSWPWM_OUT_TIMER_DMA_WAIT();

  dly = SYNCSWPWM_IN_TIMER_DMA_WAIT((uint16)SWIM_MAX_DLY);
  SYNCSWPWM_OUT_TIMER_SetCycle(arr_save);

  if (!dly)
  {
    return 1;
  }
  else
  {
    SWIM_clock_div = SWIM_DMA_IN_Buffer[1];
    return 0;
  }
}

uint8 SWIM_SetClockParam(uint8 mHz, uint8 cnt0, uint8 cnt1)
{
  uint16 clock_div;
  
  if (SWIM_clock_div)
  {
    clock_div = SWIM_clock_div;
  }
  else
  {
    clock_div = _SYS_FREQUENCY / mHz;
    if ((_SYS_FREQUENCY % mHz) >= (mHz / 2))
    {
      clock_div++;
    }
    clock_div *= SWIM_SYNC_CYCLES;
  }

  SWIM_PULSE_0 = cnt0 * clock_div / SWIM_SYNC_CYCLES;
  if ((cnt0 * clock_div % SWIM_SYNC_CYCLES) >= SWIM_SYNC_CYCLES / 2)
  {
    SWIM_PULSE_0++;
  }
  SWIM_PULSE_1 = cnt1 * clock_div / SWIM_SYNC_CYCLES;
  if ((cnt1 * clock_div % SWIM_SYNC_CYCLES) >= SWIM_SYNC_CYCLES / 2)
  {
    SWIM_PULSE_1++;
  }
  SWIM_PULSE_Threshold = SWIM_PULSE_0 + SWIM_PULSE_1;

  // 1.125 times
  SYNCSWPWM_OUT_TIMER_SetCycle(SWIM_PULSE_Threshold + (SWIM_PULSE_Threshold >> 3));

  SWIM_PULSE_Threshold >>= 1;   // metade do periodo, sera a referencia indicando a leitura se bit eh 1 ou 0
                                                                                                                          // @8Mhz -> SWIM_PULSE_Threshold = 99
  return 0;
}

uint8 SWIM_HW_Out(uint8 cmd, uint8 bitlen, uint16 retry_cnt)
{
  int8 i, p;
  uint32 dly;
  uint16 *ptr = &SWIM_DMA_OUT_Buffer[0];

retry:

  ptr = &SWIM_DMA_OUT_Buffer[0];
  *ptr++ = SWIM_PULSE_0;
  
  p = 0;
  for (i = bitlen - 1; i >= 0; i--)
  {
    if ((cmd >> i) & 1)
    {
      *ptr++ = SWIM_PULSE_1;
      p++;
    }
    else
    {
      *ptr++ = SWIM_PULSE_0;
    }
  }
  // parity bit
  if (p & 1)
  {
    *ptr++ = SWIM_PULSE_1;
  }
  else
  {
   *ptr++ = SWIM_PULSE_0;
  }
  // wait for last waveform -- parity bit
  *ptr++ = 0;
  
  SYNCSWPWM_OUT_TIMER_DMA_INIT(bitlen + 3, SWIM_DMA_OUT_Buffer);
  SYNCSWPWM_OUT_TIMER_DMA_WAIT();
  
  SYNCSWPWM_IN_TIMER_DMA_INIT(2, SWIM_DMA_IN_Buffer);
  dly = SYNCSWPWM_IN_TIMER_DMA_WAIT((uint16)SWIM_MAX_DLY);
  
  /* Toggle JTDI pin */
  if(GPIOA->ODR & GPIO_Pin_15)
    GPIOA->BRR = GPIO_Pin_15;
  else
    GPIOA->BSRR = GPIO_Pin_15;
  
  /* Toggle JTDI pin */
  if(GPIOA->ODR & GPIO_Pin_15)
    GPIOA->BRR = GPIO_Pin_15;
  else
    GPIOA->BSRR = GPIO_Pin_15;
  
  if (!dly)
  {
    GPIO_WriteBit(GPIOA, GPIO_Pin_14, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_14)));
    // timeout
    return 1;
  }
  else 
  {
    if (SWIM_DMA_IN_Buffer[1] > SWIM_PULSE_Threshold)
    { //NACK =>bit 0
      GPIO_WriteBit(GPIOA, GPIO_Pin_14, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_14)));
      // nack
      if (retry_cnt) 
      {
        retry_cnt--;
        goto retry;
      }
      else 
      {
        //retry error
        return 2;
      }
    }
  else 
    {
      //Ack =>bit 1
      return 0;
    }
  }
}

uint8 SWIM_HW_In(uint8* data, uint8 bitpos)
{
	uint8 ret = 0;
	uint32 dly;

	SYNCSWPWM_IN_TIMER_DMA_INIT(10, SWIM_DMA_IN_Buffer);
	dly = SYNCSWPWM_IN_TIMER_DMA_WAIT((uint16)SWIM_MAX_DLY);
	*data = 0;
		
//if(SWIM_DMA_IN_Buffer[10]  < SWIM_PULSE_Threshold)
//	if(GPIOA->ODR & GPIO_Pin_15)
//		GPIOA->BRR = GPIO_Pin_15;
//	else
//		GPIOA->BSRR = GPIO_Pin_15;

	
	if (dly && (SWIM_DMA_IN_Buffer[0] < SWIM_PULSE_Threshold)){ //bit 1 (start bit=1 => Device pack)
		for (dly = 0; dly < 8; dly++){
			if (SWIM_DMA_IN_Buffer[bitpos + dly] < SWIM_PULSE_Threshold){
				*data |= 1 << (7 - dly);
//if(GPIOA->ODR & GPIO_Pin_15)
//	GPIOA->BRR = GPIO_Pin_15;
//else
//	GPIOA->BSRR = GPIO_Pin_15;
			}
		}

if(GPIOA->ODR & GPIO_Pin_15)
	GPIOA->BRR = GPIO_Pin_15;
else
	GPIOA->BSRR = GPIO_Pin_15;
			//send ACK
		SWIM_DMA_OUT_Buffer[0] = SWIM_PULSE_1;
		SWIM_DMA_OUT_Buffer[1] = 0;
		SYNCSWPWM_OUT_TIMER_DMA_INIT(2, SWIM_DMA_OUT_Buffer);
		SYNCSWPWM_OUT_TIMER_DMA_WAIT();

if(GPIOA->ODR & GPIO_Pin_15)
	GPIOA->BRR = GPIO_Pin_15;
else
	GPIOA->BSRR = GPIO_Pin_15;

	}
	else {
		ret = 1;
//SWIM_DMA_OUT_Buffer[0] = SWIM_PULSE_0;
//SWIM_DMA_OUT_Buffer[1] = 0;
//SYNCSWPWM_OUT_TIMER_DMA_INIT(2, SWIM_DMA_OUT_Buffer);
//SYNCSWPWM_OUT_TIMER_DMA_WAIT();
//		goto in_init;
	}

	return ret;
}

uint8 SWIM_SRST(void)
{
	return SWIM_HW_Out(SWIM_CMD_SRST, SWIM_CMD_BITLEN, SWIM_MAX_RESEND_CNT);
}

uint8 SWIM_WOTF(uint32 addr, uint16 len, const uint8 *data)
{
	uint16 processed_len;
	uint8 cur_len, i;
	uint32 cur_addr, addr_tmp;

	if ((0 == len) || ((uint8*)0 == data))
	{
		return 1;
	}

	processed_len = 0;
	cur_addr = addr;
	while (processed_len < len)
	{
		if ((len - processed_len) > 255) {
			cur_len = 255;
		}
		else {
			cur_len = len - processed_len;
		}

		SET_LE_U32(&addr_tmp, cur_addr);

		if(SWIM_HW_Out(SWIM_CMD_WOTF, SWIM_CMD_BITLEN, SWIM_MAX_RESEND_CNT)) {
			return 1;
		}

		if (SWIM_HW_Out(cur_len, 8, 0)) {
			return 1;
		}
		if (SWIM_HW_Out((addr_tmp >> 16) & 0xFF, 8, 0)) {
			return 1;
		}
		if (SWIM_HW_Out((addr_tmp >> 8) & 0xFF, 8, 0)) {
			return 1;
		}
		if (SWIM_HW_Out((addr_tmp >> 0) & 0xFF, 8, 0)) {
			return 1;
		}
		for (i = 0; i < cur_len; i++)	{
			if (SWIM_HW_Out(data[processed_len + i], 8, SWIM_MAX_RESEND_CNT))	{
				return 1;
			}
		}

		cur_addr += cur_len;
		processed_len += cur_len;
	}

	return 0;
}

uint8 SWIM_ROTF(uint32 addr, uint16 len, uint8 *data)
{
	uint16 processed_len;
	uint8 cur_len, i;
	uint32 cur_addr, addr_tmp;

	if ((0 == len) || ((uint8*)0 == data)) {
		return 1;
	}

	processed_len = 0;
	cur_addr = addr;
	while (processed_len < len)	{
		if ((len - processed_len) > 255)	{
			cur_len = 255;
		}
		else	{
			cur_len = len - processed_len;
		}

		SET_LE_U32(&addr_tmp, cur_addr);

		if(SWIM_HW_Out(SWIM_CMD_ROTF, SWIM_CMD_BITLEN, SWIM_MAX_RESEND_CNT)) {
			return 1;
		}
		if (SWIM_HW_Out(cur_len, 8, 0)) {
			return 1;
		}
		if (SWIM_HW_Out((addr_tmp >> 16) & 0xFF, 8, 0))	{
			return 1;
		}
		if (SWIM_HW_Out((addr_tmp >> 8) & 0xFF, 8, 0)) {
			return 1;
		}
		if (SWIM_HW_Out((addr_tmp >> 0) & 0xFF, 8, 0)) {
			return 1;
		}
		i = 0;
		if (SWIM_HW_In(&data[processed_len + i], 1)) {
			return 1;
		}
		i++;
		for (; i < cur_len; i++) {
			if (SWIM_HW_In(&data[processed_len + i], 2)) {
				return 1;
			}
		}

		cur_addr += cur_len;
		processed_len += cur_len;
	}

	return 0;
}

void SWIM_OPTION_BYTES (void)
{
  //u32 endereco = 0x00004801;
  //u8 option_byte[10] = {0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF,0x00,0xFF};//{0x00,0xFF,0x00,0xFF,0xE0,0x1F,0xF0,0x0F,0x00,0xFF};
  /*
  SWIM_WOTF(endereco, 10, option_byte);
  DELAYTIMER_DelayUS(1000);
  SWIM_ROTF(endereco, 10, option_byte);
  DELAYTIMER_DelayUS(1000);*/
  SWIM_ENABLE_W_OPTION_BYTE();
  
//  SWIM_W_OPTION_BYTE(0x00004000,0xFF);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004001,0x0E);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004002,0x1F);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004003,0x03);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004004,0x87);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004005,0x00);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004006,0x0B);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004007,0x0B);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004008,0xD7);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004009,0x0B);//EEPROM
//  SWIM_W_OPTION_BYTE(0x0000400A,0x31);//EEPROM
//  SWIM_W_OPTION_BYTE(0x0000400B,0x07);//EEPROM
//  SWIM_W_OPTION_BYTE(0x0000400C,0x6B);//EEPROM
//  SWIM_W_OPTION_BYTE(0x0000400D,0x06);//EEPROM
//  SWIM_W_OPTION_BYTE(0x0000400E,0xE8);//EEPROM
//  SWIM_W_OPTION_BYTE(0x0000400F,0x04);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004010,0x6C);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004011,0x04);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004012,0x07);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004013,0x02);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004014,0xA4);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004015,0x02);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004016,0x90);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004017,0x01);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004018,0x5F);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004019,0x01);//EEPROM
//  SWIM_W_OPTION_BYTE(0x0000401A,0x1B);//EEPROM
//  SWIM_W_OPTION_BYTE(0x0000401B,0x72);//EEPROM
//  SWIM_W_OPTION_BYTE(0x0000401C,0xA8);//EEPROM
//  SWIM_W_OPTION_BYTE(0x0000401D,0x5C);//EEPROM
//  SWIM_W_OPTION_BYTE(0x0000401E,0x00);//EEPROM
//  SWIM_W_OPTION_BYTE(0x0000401F,0x0F);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004020,0x00);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004021,0x0E);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004022,0x00);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004023,0x64);//EEPROM
//  SWIM_W_OPTION_BYTE(0x00004024,0x55);//EEPROM
  
  
  SWIM_W_OPTION_BYTE(0x00004801,0x00);//OptionByte OPT1 factory default
  SWIM_W_OPTION_BYTE(0x00004802,0xFF);//OptionByte NOPT1 factory default
  SWIM_W_OPTION_BYTE(0x00004803,0x00);//OptionByte OPT2 factory default
  SWIM_W_OPTION_BYTE(0x00004804,0xFF);//OptionByte NOPT2 factory default
  SWIM_W_OPTION_BYTE(0x00004805,0x00);//OptionByte OPT3 factory default
  SWIM_W_OPTION_BYTE(0x00004806,0xFF);//OptionByte NOPT3 factory default
  SWIM_W_OPTION_BYTE(0x00004807,0x00);//OptionByte OPT4 factory default
  SWIM_W_OPTION_BYTE(0x00004808,0xFF);//OptionByte NOPT4 factory default
  SWIM_W_OPTION_BYTE(0x00004809,0x00);//OptionByte OPT5 factory default
  SWIM_W_OPTION_BYTE(0x0000480A,0xFF);//OptionByte NOPT5 factory default
  
  #ifdef STM8S005_STM8S105_STM8S207_STM8S208
    SWIM_W_OPTION_BYTE(0x0000480B,0x00);//OptionByte OPT4 factory default
    SWIM_W_OPTION_BYTE(0x0000480C,0xFF);//OptionByte NOPT4 factory default
    SWIM_W_OPTION_BYTE(0x0000480D,0x00);//OptionByte OPT5 factory default
    SWIM_W_OPTION_BYTE(0x0000480E,0xFF);//OptionByte NOPT5 factory default
    SWIM_W_OPTION_BYTE(0x0000487E,0x00);//OptionByte OPT4 factory default
    SWIM_W_OPTION_BYTE(0x0000487F,0xFF);//OptionByte NOPT4 factory default
  #endif
}

void SWIM_ENABLE_W_OPTION_BYTE (void)
{
  u32 endereco;
  u8 option_byte;
  
  endereco = 0x0000505B;		//FLASH_CR2
  option_byte=0x80;
  SWIM_WOTF(endereco, 1, &option_byte);
  endereco = 0x0000505C;		//FLASH_NCR2
  option_byte=0x7F;
  SWIM_WOTF(endereco, 1, &option_byte); 
}
  
void SWIM_W_OPTION_BYTE(u32 endereco, u8 option_byte)
{
  u8 ReadOptionByte = 0;
  do
  {
    SWIM_WOTF(endereco, 1, &option_byte);
    DELAYTIMER_DelayUS(500);
    SWIM_ROTF(endereco, 1, &ReadOptionByte);
  }
  while(ReadOptionByte != option_byte);
  
  DELAYTIMER_DelayUS(500);
}