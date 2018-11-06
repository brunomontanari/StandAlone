/**
  ******************************************************************************
  * @file    RCC/main.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    09/13/2010
  * @brief   Header for main.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "gpio.h"
#include "tim.h"
#include "swim.h"
#include "auxiliar.h"
#include "stlink.h"
#include "syncsw.h"
#include "STM8_program.h"

/* ---------------------------- CONFIGURAÇÕES ------------------------------- */

/* Habilitar escrita na EEPROM
   1 - Habilita a escrita do vetor EEPROMInit[EEP_SIZE] na EEPROM
   0 - Não habilita escrita na EEPROM
 */
#define W_EEPs        1

/* Habilitar Proteção de leitura (Read Out Protection)
   1 - Protege a memória FLASH contra leitura
   0 - Não protege a FLASH contra leitura
 */
#define ATIVAR_ROP    1

/* Família de microcontrolador utilizado
   Descomente a linha que contém a família do microcontrolador a ser gravado
 */
#define STM8S001_STM8S003_STM8S103_STM8S903
//#define STM8S005_STM8S105_STM8S207_STM8S208

/* -------------------------------------------------------------------------- */
    
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define ONE_KBYTE       1024
#define EEP_SIZE        (640)

#ifdef STM8S005_STM8S105_STM8S207_STM8S208
  #define FILE_SIZE (32*ONE_KBYTE)
#else
  #define FILE_SIZE (8*ONE_KBYTE)
#endif

/* Exported macro ------------------------------------------------------------*/
#define HSE_VALUE                       ((uint32_t)8000000)

#define _SYS_FREQUENCY                  72      // in MHz
#define _SYS_FLASH_VECTOR_TABLE_SHIFT	0x2000	// application will locate at 0x08002000
/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
