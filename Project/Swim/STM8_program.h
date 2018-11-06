/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "swim.h"
#include "auxiliar.h"
#include "stlink.h"
#include "syncsw.h"

/* Private define ------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/
extern uint8 SWIM_Estado;

/* Private function prototypes -----------------------------------------------*/
void STM8_program(void);
void WriteEEPROMData(void);
extern void Error_handler(void);