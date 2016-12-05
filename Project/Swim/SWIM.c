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

static uint8 SWIM_Inited = 0;
static uint16 SWIM_PULSE_0;
static uint16 SWIM_PULSE_1;
static uint16 SWIM_PULSE_Threshold;
// max length is 1(header)+8(data)+1(parity)+1(ack from target)+1(empty)
static uint16 SWIM_DMA_IN_Buffer[150];
static uint16 SWIM_DMA_OUT_Buffer[15];
static uint16 SWIM_clock_div = 0;

#define SWIM_MAX_DLY		0x0FFFF
#define SWIM_MAX_RESEND_CNT	10
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
  
  SWIM_W_OPTION_BYTE(0x00001000,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001001,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001002,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001003,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001004,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001005,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001006,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001007,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001008,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001009,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000100A,0x03);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000100B,0xE7);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000100C,0x27);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000100D,0x0F);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000100E,0x98);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000100F,0x96);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001010,0x7F);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001011,0x03);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001012,0xB9);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001013,0xAC);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001014,0x9F);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001015,0xF9);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001016,0x42);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001017,0x09);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001018,0xDE);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001019,0xF1);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000101A,0x97);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000101B,0x54);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000101C,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000101D,0xBE);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000101E,0x06);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000101F,0x08);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001020,0x59);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001021,0x2F);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001022,0x20);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001023,0x3C);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001024,0x3C);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001025,0x03);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001026,0x0F);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001027,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001028,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001029,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000102A,0x1E);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000102B,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000102C,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000102D,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000102E,0x78);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000102F,0x54);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001030,0x09);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001031,0xCC);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001032,0x34);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001033,0xFF);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001034,0xFF);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001035,0xFF);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001036,0xFE);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001037,0x10);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001038,0x10);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001039,0x10);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000103A,0x10);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000103B,0x10);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000103C,0x10);//EEPROM 
  SWIM_W_OPTION_BYTE(0x0000103D,0x10);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000103E,0x10);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000103F,0x10);//EEPROM
  
  SWIM_W_OPTION_BYTE(0x00001040,0x11);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001041,0x34);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001042,0xF2);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001043,0x13);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001044,0x86);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001045,0x04);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001046,0x08);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001047,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001048,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001049,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000104A,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000104B,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000104C,0x00);//EEPROM 
  SWIM_W_OPTION_BYTE(0x0000104D,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000104E,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000104F,0x00);//EEPROM
    SWIM_W_OPTION_BYTE(0x00001050,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001051,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001052,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001053,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001054,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001055,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001056,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001057,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001058,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001059,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000105A,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000105B,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000105C,0x00);//EEPROM 
  SWIM_W_OPTION_BYTE(0x0000105D,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000105E,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000105F,0x00);//EEPROM
    SWIM_W_OPTION_BYTE(0x00001060,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001061,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001062,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001063,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001064,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001065,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001066,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001067,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001068,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001069,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000106A,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000106B,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000106C,0x00);//EEPROM 
  SWIM_W_OPTION_BYTE(0x0000106D,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000106E,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000106F,0x1A);//EEPROM
    SWIM_W_OPTION_BYTE(0x00001070,0x24);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001071,0x37);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001072,0x63);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001073,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001074,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001075,0x24);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001076,0x37);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001077,0x63);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001078,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001079,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000107A,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000107B,0x5A);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000107C,0x63);//EEPROM 
  SWIM_W_OPTION_BYTE(0x0000107D,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000107E,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000107F,0x00);//EEPROM
    SWIM_W_OPTION_BYTE(0x00001080,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001081,0x43);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001082,0x63);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001083,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001084,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001085,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001086,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001087,0x43);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001088,0x5A);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001089,0x63);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000108A,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000108B,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000108C,0x00);//EEPROM 
  SWIM_W_OPTION_BYTE(0x0000108D,0x63);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000108E,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000108F,0x00);//EEPROM
    SWIM_W_OPTION_BYTE(0x00001090,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001091,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001092,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001093,0x37);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001094,0x63);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001095,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001096,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001097,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001098,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x00001099,0x5A);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000109A,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000109B,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000109C,0x00);//EEPROM 
  SWIM_W_OPTION_BYTE(0x0000109D,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000109E,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x0000109F,0x00);//EEPROM
    SWIM_W_OPTION_BYTE(0x000010A0,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010A1,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010A2,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010A3,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010A4,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010A5,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010A6,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010A7,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010A8,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010A9,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010AA,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010AB,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010AC,0x00);//EEPROM 
  SWIM_W_OPTION_BYTE(0x000010AD,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010AE,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010AF,0x00);//EEPROM
    SWIM_W_OPTION_BYTE(0x000010B0,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010B1,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010B2,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010B3,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010B4,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010B5,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010B6,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010B7,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010B8,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010B9,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010BA,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010BB,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010BC,0x00);//EEPROM 
  SWIM_W_OPTION_BYTE(0x000010BD,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010BE,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010BF,0x00);//EEPROM
    SWIM_W_OPTION_BYTE(0x000010C0,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010C1,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010C2,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010C3,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010C4,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010C5,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010C6,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010C7,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010C8,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010C9,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010CA,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010CB,0x05);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010CC,0x00);//EEPROM 
  SWIM_W_OPTION_BYTE(0x000010CD,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010CE,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010CF,0x00);//EEPROM
    SWIM_W_OPTION_BYTE(0x000010D0,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010D1,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010D2,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010D3,0x01);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010D4,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010D5,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010D6,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010D7,0x01);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010D8,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010D9,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010DA,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010DB,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010DC,0x00);//EEPROM 
  SWIM_W_OPTION_BYTE(0x000010DD,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010DE,0x00);//EEPROM
  SWIM_W_OPTION_BYTE(0x000010DF,0x00);//EEPROM
//  
//  SWIM_W_OPTION_BYTE(0x00004801,0x00);//OptionByte OPT1 factory default
//  SWIM_W_OPTION_BYTE(0x00004802,0xFF);//OptionByte NOPT1 factory default
//  SWIM_W_OPTION_BYTE(0x00004803,0x00);//OptionByte OPT2 factory default
//  SWIM_W_OPTION_BYTE(0x00004804,0xFF);//OptionByte NOPT2 factory default
//  SWIM_W_OPTION_BYTE(0x00004805,0x00);//OptionByte OPT3 factory default
//  SWIM_W_OPTION_BYTE(0x00004806,0xFF);//OptionByte NOPT3 factory default
//  SWIM_W_OPTION_BYTE(0x00004807,0x00);//OptionByte OPT4 factory default
//  SWIM_W_OPTION_BYTE(0x00004808,0xFF);//OptionByte NOPT4 factory default
//  SWIM_W_OPTION_BYTE(0x00004809,0x00);//OptionByte OPT5 factory default
//  SWIM_W_OPTION_BYTE(0x0000480A,0xFF);//OptionByte NOPT5 factory default
//  //lembrar de fazer ifdef para familia 105
//  
//  SWIM_W_OPTION_BYTE(0x0000480B,0x00);//OptionByte OPT4 factory default
//  SWIM_W_OPTION_BYTE(0x0000480C,0xFF);//OptionByte NOPT4 factory default
//  SWIM_W_OPTION_BYTE(0x0000480D,0x00);//OptionByte OPT5 factory default
//  SWIM_W_OPTION_BYTE(0x0000480E,0xFF);//OptionByte NOPT5 factory default
//  SWIM_W_OPTION_BYTE(0x0000487E,0x00);//OptionByte OPT4 factory default
//  SWIM_W_OPTION_BYTE(0x0000487F,0xFF);//OptionByte NOPT4 factory default
}

void SWIM_ENABLE_W_OPTION_BYTE (void)
{
  u32 endereco;
  u8 option_byte;
  
  endereco = FLASH_CR2;//0x0000505B;		//FLASH_CR2
  option_byte=0x80;
  SWIM_WOTF(endereco, 1, &option_byte);
#ifndef STM8L052R8 
  endereco = FLASH_NCR2; //0x0000505C;		//FLASH_NCR2
  option_byte=0x7F;
  SWIM_WOTF(endereco, 1, &option_byte); 
#endif   
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