/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       SWIM.h                                                    *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    SWIM interface header file                                *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2008-11-07:     created(by SimonQian)                             *
 **************************************************************************/
#include "STM32f10x.h"
#include "app_type.h"

#define SWIM_INIT		  0
#define SWIM_ENTRY_SEQUENCY	  1
#define SWIM_PGM_OUT            2
#define SWIM_SPEED_SET          3
#define SWIM_CONFIG             4
#define SWIM_CONFIG_TIME        5
#define SWIM_VERIFY_CONFIG      6
#define SWIM_ID                 7
#define SWIM_RESET              8
#define SWIM_STAL_uC            9
#define SWIM_FLASH_LOCK_VERIFY  10
#define SWIM_FLASH_UNLOCK_FASE1 11
#define SWIM_FLASH_UNLOCK_FASE2 12
#define SWIM_FLASH_WR_SET       13
#define SWIM_FLASH_WR_SET1      14
#define SWIM_FLASH_WR_SET2      15
#define SWIM_FLASH_READY_VERIFY 16
#define SWIM_FLASH_WR           17
#define SWIM_FLASH_WR_WORD      18
#define SWIM_FLASH_WR_BLOCK     19
#define SWIM_FLASH_LOCK         20
#define SWIM_WR_VERIFY_SET      21
#define SWIM_WR_VERIFY          22
#define SWIM_ERROR              23
#define SWIM_IDLE               24
#define SWIM_FLASH_ROP          25
#define SWIM_EEPROM_UNLOCK_FASE1 26
#define SWIM_EEPROM_UNLOCK_FASE2 27

#define FLASH_BYTE              0
#define FLASH_WORD              1
#define FLASH_BLOCK             2

#define BLOCK_SIZE              128	//STM8S105KX = 128 bytes, STM8S103F2 = 64 bytes
#define STM8L052R8  1
#ifdef STM8L052R8
#define FLASH_CR1   0x005050
#define FLASH_CR2   0x005051
#define FLASH_PUKR  0x005052
#define FLASH_DUKR  0x005053
#define FLASH_IAPSR 0x005054

#else
#define FLASH_CR1   0x00505A
#define FLASH_CR2   0x00505B
#define FLASH_NCR2  0x00505C
#define FLASH_PUKR  0x005062
#define FLASH_DUKR  0x005064
#define FLASH_IAPSR 0x00505F
#endif


void SWIM_Init();
void SWIM_Fini();
uint8 SWIM_Sync(uint8 mHz);
uint8 SWIM_SetClockParam(uint8 mHz, uint8 cnt0, uint8 cnt1);
uint8 SWIM_WOTF(uint32 addr, uint16 len, const uint8 *data);
uint8 SWIM_ROTF(uint32 addr, uint16 len, uint8 *data);
uint8 SWIM_SRST(void);
void SWIM_EnableClockInput(void);
uint8 SWIM_EnterProgMode(void);
void SWIM_OPTION_BYTES (void);
void SWIM_ENABLE_W_OPTION_BYTE (void);
void SWIM_W_OPTION_BYTE(u32 endereco, u8 option_byte);