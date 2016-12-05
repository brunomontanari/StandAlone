/**
******************************************************************************
* @file    main.c 
* @author  Bruno Montanari
* @version V1.0.0
* @date    05/13/2015
* @brief   Main program body.
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "swim.h"
#include "auxiliar.h"
#include "stlink.h"

#include "syncsw.h"


/** @addtogroup Examples
* @{
*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//#define ATIVAR_ROP 1
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStructure;
RCC_ClocksTypeDef RCC_ClockFreq;
ErrorStatus HSEStartUpStatus;
uint16 byte_num;
uint16 byte_num2;
uint32 addr;
uint32 addr2;
uint8 iapsr;
uint8 rop;
uint8 SWIM_Estado;
uint8 FlashWrMode=FLASH_BYTE;
uint8 block_num;
extern const uint8 ProgInit[FILE_SIZE];
uint8	OptionBytesRead[11];


/* Private function prototypes -----------------------------------------------*/
void NVIC_Configuration(void);
/* Private functions ---------------------------------------------------------*/
/**
* @brief  Configures NVIC.
* @param  None
* @retval None
*/
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Configure the NVIC Preemption Priority Bits */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /* Enable the TIM2 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable the DMA1 global Interrupt */
  //  NVIC_InitStructure.NVIC_IRQChannel =  DMA1_Channel1_IRQn;
  //  NVIC_Init(&NVIC_InitStructure);
  
  
  /* 1 bits for pre-emption priority and 3 bits for subpriority */
  //  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  
  /* Set Button EXTI Interrupt priority to 0 (highest) */
  // NVIC_SetPriority(TIM4_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0,0));
  
  /* Set WWDG interrupt vector Preemption Priority to 1 */
  //  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  //  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  //  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  //  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  //  NVIC_Init(&NVIC_InitStructure);
}
/**
* @brief  Main program.
* @param  None
* @retval None
*/
int main(void)
{
  uint8	TxTeste[129];
  uint8 t2;
  
  /* Configure the System clock frequency, HCLK, PCLK2 and PCLK1 prescalers */
  //SetSysClock();
  
  /* Enable GPIOA, GPIOB, RCC_APB2Periph_GPIO_KEY_BUTTON and AFIO clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
  
  /* Configure PA.13 (JTMS/SWDAT), PA.14 (JTCK/SWCLK) and PA.15 (JTDI) as
  output push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15|GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  /* Configure PB.03 (JTDO) and PB.04 (JTRST) as output push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  /* Init Debug pins */
  GPIO_WriteBit(GPIOA, GPIO_Pin_14, (BitAction)(Bit_RESET));
  GPIO_WriteBit(GPIOA, GPIO_Pin_15, (BitAction)(Bit_RESET));
  
  NVIC_Configuration();
  GPIO_WriteBit(GPIOA, GPIO_Pin_8, (BitAction)(Bit_SET));
  SWIM_Estado=SWIM_INIT;
  
  while (SWIM_Estado!=SWIM_IDLE){	
    switch (SWIM_Estado){
    case SWIM_INIT:{
      DELAYTIMER_INIT();	
      SW_RST_DIR_INIT();
      SYNCSW_DIR_INIT();
      SW_RST_CLR();
      SWIM_EnableClockInput();
      SWIM_Estado=SWIM_ENTRY_SEQUENCY;
      
      break;}
    case SWIM_ENTRY_SEQUENCY:{
      if (SWIM_EnterProgMode())	{
        //Time-Out try again
        SW_RST_SET();
        SWIM_Estado = SWIM_INIT;
        SWIM_Estado = SWIM_PGM_OUT;
        
      }
      else {
        SWIM_Estado = SWIM_PGM_OUT;
        
      }
      break;}
    case SWIM_PGM_OUT:{
      SWIM_Init();
      SWIM_Estado=SWIM_SPEED_SET;
      break;}
    case SWIM_SPEED_SET:{	
      
      //				t1=SWIM_Sync(8);
      t2=SWIM_SetClockParam(8, 18, 2);//19,2 -> Verde/Azul
      if ( 0 || t2)	{
        //				if ( t1 || t2)	{
        //Time-Out try again
        SWIM_Estado = SWIM_SPEED_SET;
      }
      else{
        SWIM_Estado = SWIM_CONFIG;
      }
      break;}
    case SWIM_CONFIG:{		// 7,12ms 		// 8,55ms			//9,99ms
      byte_num = 3;//GET_BE_U16(&cmd[1]);
      addr = 0x00000000;//GET_BE_U32(&cmd[3]);
      
      //write 0xA0 to SWIM_CSR -> SWIM_DM=1, SAFE_MASK=1
      byte_num = 1;
      addr = 0x007F80;		//SWIM_CSR
      TxTeste[0]=0xA0;
      if (SWIM_WOTF(addr, byte_num, TxTeste)) {
        //Time-Out try again
        SWIM_Estado =SWIM_CONFIG ;
      }
      else {
        SWIM_Estado = SWIM_CONFIG_TIME;
      }
      break;}
    case SWIM_CONFIG_TIME:{		//7,3ms			//8,77ms		//10,16ms
      
      SW_RST_SET();
      //Wait 1ms to reach pass 8
      DELAYTIMER_DelayUS(1000);
      SWIM_Estado = SWIM_VERIFY_CONFIG;
      break;}
    case SWIM_VERIFY_CONFIG:{	//8,3ms			//9,77			//11,17ms
      //Read SWIM_CSR
      byte_num = 1;
      addr = 0x007F80;		//SWIM_CSR
      TxTeste[0]=0x00;
      
      if (SWIM_ROTF(addr, byte_num, TxTeste)) {
        //Time-Out try again
        SWIM_Estado = SWIM_VERIFY_CONFIG;
      }
      else {
        if((TxTeste[0]==0xA0)||(TxTeste[0]==0xA2)){
          //configured
          GPIO_WriteBit(GPIOA, GPIO_Pin_14, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_14)));
          //SWIM_Estado = SWIM_ID;//BFM
          SWIM_Estado = SWIM_RESET;
          
        }
        else{
          //Not configured, so config
          SWIM_Estado = SWIM_CONFIG;
        }
      }
      break;}
    case SWIM_ID:{		//8,54ms
      //Verify Device ID
      byte_num = 12;
      addr = 0x0048CD;
      TxTeste[0]=0xFF;		//0x00  //8,71
      TxTeste[1]=0xFF;		//0x20	//8,75.....0x2E?
      TxTeste[2]=0x00;		//0x80	//8,78.....0x00?
      TxTeste[3]=0x00;		//0x04	//8,81.....0x26?
      TxTeste[4]=0xFF;		//0x00	//8,85
      TxTeste[5]=0xFF;		//0x00	//8,88
      TxTeste[6]=0x00;		//0x2C	//8,91
      TxTeste[7]=0x00;		//0x28	//8,95
      TxTeste[8]=0x00;		//0x70	//8,99
      TxTeste[9]=0x00;		//0x64	//9,01
      TxTeste[10]=0x00;		//0x31	//9,05
      TxTeste[11]=0x00;		//0x29	//9,09
      TxTeste[12]=0x00;
      
      GPIO_WriteBit(GPIOA, GPIO_Pin_14, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_14)));
      if (SWIM_ROTF(addr, byte_num, TxTeste)) {
        //Time-Out try again
        SWIM_Estado = SWIM_ID;
      }
      else {
        if	( //(TxTeste[0]==0x00) &&
                 //(TxTeste[1]==0x20) &&
                 //(TxTeste[2]==0x80) &&
                 //(TxTeste[3]==0x04) &&
                 //(TxTeste[4]==0x00) &&
                 //(TxTeste[5]==0x00) &&
                 //(TxTeste[6]==0x2C) &&
                 //(TxTeste[7]==0x28) &&
                 (TxTeste[8]==0x70) &&
                   (TxTeste[9]==0x64) &&
                     (TxTeste[10]==0x31)&&
                       (TxTeste[11]==0x29)){		//9,14ms
                         GPIO_WriteBit(GPIOA, GPIO_Pin_14, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_14)));
                         SWIM_Estado = SWIM_RESET;
                       }
        else{
          SWIM_Estado = SWIM_ERROR;
        }
      }
      break;}
    case SWIM_RESET:{		//9,155ms
      //Send SWIM SRST
      SWIM_SRST();
      SWIM_Estado = SWIM_STAL_uC;
      
      break;}
    case SWIM_STAL_uC:{		//9,178ms
      //Stal uC
      byte_num = 1;
      addr = 0x007F99;		//DM_CSR2
      TxTeste[0]=0x08;
      if (SWIM_WOTF(addr, byte_num, TxTeste)) {
        //Time-Out try again
        SWIM_Estado = SWIM_STAL_uC;
      }
      else {
        SWIM_Estado = SWIM_FLASH_LOCK_VERIFY;
      }
      break;}
    case SWIM_FLASH_LOCK_VERIFY:{		//9,53ms		//10,14ms
      //Read the PUL bit
      byte_num = 1;
      addr = FLASH_IAPSR;//0x00505F;		//FLASH_IAPSR
      TxTeste[0]=0x00;
      
      if (SWIM_ROTF(addr, byte_num, TxTeste)) {
        //Time-Out try again
        SWIM_Estado = SWIM_FLASH_LOCK_VERIFY;
      }
      else {
        //PARAR AQUI
        
        if((TxTeste[0]&0x08)&&(TxTeste[0]&0x02)){
          //PUL and DUL set -> unlocked
//          SWIM_OPTION_BYTES();
          SWIM_Estado = SWIM_FLASH_WR_SET;
          SWIM_ROTF(0x00004800, 11, OptionBytesRead);
          
          
        }
        else{
          //DUL reset -> locked
          SWIM_Estado = SWIM_EEPROM_UNLOCK_FASE1;
        }
      }
      break;}
    case SWIM_FLASH_UNLOCK_FASE1:{	//9,744ms
      //Unlock memory (Unlock MASS)
      byte_num = 1;
      addr = FLASH_PUKR;//0x00005062;		//FLASH_PUKR
      TxTeste[0]=0x56;
      if (SWIM_WOTF(addr, byte_num, TxTeste)) {
        //Time-Out try again, in this case  re init all process
        SW_RST_SET();
        SWIM_Estado = SWIM_INIT;
      }
      else {
        SWIM_Estado = SWIM_FLASH_UNLOCK_FASE2;
      }
      break;}
    case SWIM_FLASH_UNLOCK_FASE2:{	//9,93ms
      
      byte_num = 1;
      addr = FLASH_PUKR;//0x00005062;		//FLASH_PUKR
      TxTeste[0]=0xAE;
      if (SWIM_WOTF(addr, byte_num, TxTeste)) {
        GPIO_WriteBit(GPIOA, GPIO_Pin_14, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_14)));
        //Time-Out try again, in this case  re init all process
        SW_RST_SET();
        SWIM_Estado = SWIM_INIT;
      }
      else {
        SWIM_Estado = SWIM_FLASH_LOCK_VERIFY;
      }
      break;}
      //BFM BEGIN
    case SWIM_EEPROM_UNLOCK_FASE1:{	//9,744ms
      //Unlock memory (Unlock MASS)
      
      byte_num = 1;
      addr = FLASH_DUKR;//0x00005064;		//FLASH_DUKR
      TxTeste[0]=0xAE;
      if (SWIM_WOTF(addr, byte_num, TxTeste)) {
        //Time-Out try again, in this case  it is not necessary to reinit the uC
        SWIM_Estado = SWIM_EEPROM_UNLOCK_FASE1;
      }
      else {
        SWIM_Estado = SWIM_EEPROM_UNLOCK_FASE2;
      }
      break;}
    case SWIM_EEPROM_UNLOCK_FASE2:{	//9,93ms
      
      byte_num = 1;
      addr = FLASH_DUKR;//0x00005064;		//FLASH_DUKR
      TxTeste[0]=0x56;
      if (SWIM_WOTF(addr, byte_num, TxTeste)) {
        GPIO_WriteBit(GPIOA, GPIO_Pin_14, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_14)));
        //Time-Out try again, in this case  it is not necessary to reinit the uC
        SWIM_Estado = SWIM_EEPROM_UNLOCK_FASE2;
      }
      else {
        //SWIM_Estado = SWIM_FLASH_LOCK_VERIFY;
        
        //BFM BEGIN
        byte_num = 1;
        addr = FLASH_IAPSR;//0x00505F;		//FLASH_IAPSR
        TxTeste[0]=0x00;
        
        if (SWIM_ROTF(addr, byte_num, TxTeste)) {
          //Time-Out try again
          SWIM_Estado = SWIM_EEPROM_UNLOCK_FASE1;
        }
        else {
          if(TxTeste[0]&0x08){
            //DUL set -> unlocked
            SWIM_Estado = SWIM_FLASH_UNLOCK_FASE1;
            addr2 = FLASH_CR2;//0x0000505B;		//FLASH_CR2
            iapsr = 0x80;
            SWIM_WOTF(addr2, 1, &iapsr);
#ifndef STM8L052R8
            addr2 = 0x0000505C;		//FLASH_NCR2
            iapsr = 0x7F;
            SWIM_WOTF(addr2, 1, &iapsr);
#endif            
            
            addr2 = 0x00004800;		//OptionByte ROP disable
#ifndef STM8L052R8
            iapsr = 0x00;
#else
            iapsr = 0xAA;
#endif
            if(SWIM_WOTF(addr2, 1, &iapsr))
            {
              SWIM_Estado = SWIM_EEPROM_UNLOCK_FASE2;
            }   
            while((iapsr & 0x04) == 0x00)
            { 
              SWIM_ROTF (FLASH_IAPSR, 1, &iapsr); // 0x00505F
            }
            SWIM_ROTF (0x00004800, 1, &iapsr);
#ifndef STM8L052R8         
            if (iapsr != 0x00)
            {
              SWIM_Estado = SWIM_EEPROM_UNLOCK_FASE2;
            }
#else
            if (iapsr != 0xAA)
            {
              SWIM_Estado = SWIM_EEPROM_UNLOCK_FASE2;
            }          
#endif
            //DELAYTIMER_DelayUS(10000);
            
            
            SWIM_OPTION_BYTES();
            SWIM_ROTF(0x00004800, 11, OptionBytesRead);
            DELAYTIMER_DelayUS(1000);
//            if(OptionBytesRead[2] != 0x40)
//            {
//              SWIM_Estado = SWIM_EEPROM_UNLOCK_FASE2;
//            }
//            else
//            {
//              SWIM_ROTF(0x00004804, 1, &OptionBytesRead[3]);
//              DELAYTIMER_DelayUS(1000);
//              if(OptionBytesRead[2] != 0xBF)
//              {
//                SWIM_Estado = SWIM_EEPROM_UNLOCK_FASE2;
//              }
//            }
            
          }
          else{
            //DUL reset -> locked
            SWIM_Estado = SWIM_EEPROM_UNLOCK_FASE1;
          }
        }
        //BFM END
        
      }
      break;}
      //BFM END                        
    case SWIM_FLASH_WR_SET:{			//10,35ms
      
      byte_num = 0;
      addr = 0x00008000;//bfm 8080
      byte_num2 = FILE_SIZE;
      //	FlashWrMode=FLASH_BYTE;	//Byte
      //FlashWrMode=FLASH_WORD;	//Word BFM
      FlashWrMode=FLASH_BLOCK;	//Block, verificar o endereco
      SWIM_Estado = SWIM_FLASH_READY_VERIFY;
      break;}
    case SWIM_FLASH_WR_SET1:{					//10,38ms
      addr2 = FLASH_CR2;//0x0000505B;		//FLASH_CR2
      if(FlashWrMode==FLASH_BYTE)
        TxTeste[0]=0x00;
      if(FlashWrMode==FLASH_WORD)
        TxTeste[0]=0x40;
      if(FlashWrMode==FLASH_BLOCK)
        TxTeste[0]=0x01;
      
      if (SWIM_WOTF(addr2, 1, TxTeste)) {
        //Time-Out try again
        SWIM_Estado = SWIM_FLASH_WR_SET1;
      }
      else {
#ifndef STM8L052R8
        SWIM_Estado = SWIM_FLASH_WR_SET2;
#else
        if(FlashWrMode==FLASH_BYTE)
          SWIM_Estado = SWIM_FLASH_WR;
        if(FlashWrMode==FLASH_WORD)
          SWIM_Estado = SWIM_FLASH_WR_WORD;
        if(FlashWrMode==FLASH_BLOCK)
          SWIM_Estado = SWIM_FLASH_WR_BLOCK;
#endif
      }
      break;}
    case SWIM_FLASH_WR_SET2:{		//10,597ms
      addr2 = 0x0000505C;		//FLASH_NCR2
      if(FlashWrMode==FLASH_BYTE)
        TxTeste[0]=0xFF;
      if(FlashWrMode==FLASH_WORD)
        TxTeste[0]=0xBF;
      if(FlashWrMode==FLASH_BLOCK)
        TxTeste[0]=0xFE;
      
      if (SWIM_WOTF(addr2, 1, TxTeste)) {
        //Time-Out try again
        SWIM_Estado = SWIM_FLASH_WR_SET2;
      }
      else {
        if(FlashWrMode==FLASH_BYTE)
          SWIM_Estado = SWIM_FLASH_WR;
        if(FlashWrMode==FLASH_WORD)
          SWIM_Estado = SWIM_FLASH_WR_WORD;
        if(FlashWrMode==FLASH_BLOCK)
          SWIM_Estado = SWIM_FLASH_WR_BLOCK;
      }
      break;}
    case SWIM_FLASH_READY_VERIFY:{		
      //byte: 10,814ms
      //word: 10,841ms	//11,35ms	//
      addr2 = FLASH_IAPSR;//0x00505F;		//FLASH_IAPSR
      iapsr=0x00;
      
      if (SWIM_ROTF (addr2, 1, &iapsr)) {
        //Time-Out try again
        SWIM_Estado = SWIM_FLASH_READY_VERIFY;
      }
      else {
        if (iapsr & (0x20|0x04|0x01)){	//HV_ON, EOP or WR_PG_DIS
          GPIO_WriteBit(GPIOA, GPIO_Pin_15, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_15)));
          //Not ready
          SWIM_Estado = SWIM_FLASH_READY_VERIFY;
        }
        else{
          if (iapsr & (0x40)){	//HV_OFF
            SWIM_Estado = SWIM_FLASH_WR_SET1;
          }
          else {	//HV_ON
            //Not ready
            SWIM_Estado = SWIM_FLASH_READY_VERIFY;
          }
        }
      }
      break;}
    case SWIM_FLASH_WR:{		//11,02ms
      if (SWIM_WOTF(addr, 1, &ProgInit[byte_num])) {
        //Time-Out try again
        SWIM_Estado = SWIM_FLASH_WR;
      }
      else {
        if ((byte_num+1) < byte_num2){
          SWIM_Estado = SWIM_FLASH_READY_VERIFY;
          byte_num++;
          addr++;
        }
        else{
          SWIM_Estado = SWIM_FLASH_LOCK;
        }
      }
      break;}
    case SWIM_FLASH_WR_WORD:{		
      //init:	11,02ms
      //			17,53ms
      if (SWIM_WOTF(addr, 4, &ProgInit[byte_num])) {
        //Time-Out try again
        SWIM_Estado = SWIM_FLASH_WR_WORD;
      }
      else {
        byte_num+=4;
        if (byte_num < byte_num2){
          SWIM_Estado = SWIM_FLASH_READY_VERIFY;
          if ((byte_num2-byte_num)<4){
            FlashWrMode=FLASH_BYTE;
            SWIM_Estado = SWIM_FLASH_WR_SET1;
          }
          addr+=4;
        }
        else{
          SWIM_Estado = SWIM_FLASH_LOCK;
        }
      }
      break;}
    case SWIM_FLASH_WR_BLOCK:{	//11,02ms
      if (SWIM_WOTF(addr, BLOCK_SIZE, &ProgInit[byte_num])) {
        //Time-Out try again
        SWIM_Estado = SWIM_FLASH_WR_BLOCK;
      }
      else {
        byte_num+=BLOCK_SIZE;
        if (byte_num < byte_num2){
          SWIM_Estado = SWIM_FLASH_READY_VERIFY;
          if ((byte_num2-byte_num)<BLOCK_SIZE){
            FlashWrMode=FLASH_WORD;
            SWIM_Estado = SWIM_FLASH_WR_SET1;
          }
          addr+=BLOCK_SIZE;
        }
        else{
          //BFM SWIM_Estado = SWIM_FLASH_LOCK;
          SWIM_Estado = SWIM_WR_VERIFY_SET;
        }
      }
      break;}
    case SWIM_FLASH_LOCK:{ //MASS Lock
      //write the PUL and DUL bit
      addr2 = FLASH_IAPSR;//0x00505F;		//FLASH_IAPSR
      iapsr&=~(0x02|0x08);
      
      if (SWIM_WOTF(addr2, 1, &iapsr)) {
        //Time-Out try again
        SWIM_Estado = SWIM_FLASH_LOCK;
      }
      else {
        SWIM_Estado = SWIM_IDLE;
      }
      break;}
    case SWIM_WR_VERIFY_SET:{
      byte_num = 0;
      addr = 0x00008000;
      byte_num2 = FILE_SIZE;
      block_num=0;
      SWIM_Estado = SWIM_WR_VERIFY;
      break;}
    case SWIM_WR_VERIFY:{
      //Verify Wrote Data
      for(byte_num = 0; byte_num < BLOCK_SIZE; byte_num++)
        TxTeste[byte_num]=0x00;
      
      if (SWIM_ROTF(addr, BLOCK_SIZE, TxTeste)) {
        //Time-Out try again
        SWIM_Estado = SWIM_WR_VERIFY;
      }
      else {
        for(byte_num = 0; (byte_num < BLOCK_SIZE) && (byte_num < byte_num2); byte_num++){
          if(TxTeste[byte_num]!=ProgInit[(block_num*BLOCK_SIZE)+byte_num]){
            SWIM_Estado = SWIM_ERROR;
          }
        }
        if (SWIM_Estado!= SWIM_ERROR){
          if (byte_num2>=BLOCK_SIZE){
            byte_num2-=BLOCK_SIZE;
            if (byte_num2){
              addr+=BLOCK_SIZE;
              block_num++;
              SWIM_Estado = SWIM_WR_VERIFY;
            }
          }
          else{
            byte_num2=0;
            GPIO_WriteBit(GPIOA, GPIO_Pin_15, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_15)));
             SWIM_Estado = SWIM_IDLE;
            //SWIM_Estado = SWIM_FLASH_ROP;//BFM 
          }
        }
      }
      break;}
    case SWIM_FLASH_ROP:{ //Read Out Protection
      
      SWIM_OPTION_BYTES();
      addr2 = FLASH_CR2;// 0x0000505B;		//FLASH_CR2
      rop = 0x80;
      SWIM_WOTF(addr2, 1, &rop);
#ifndef STM8L052R8      
      addr2 = FLASH_NCR2;// 0x0000505C;		//FLASH_NCR2
      rop = 0x7F;	
      SWIM_WOTF(addr2, 1, &rop);
      
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
#endif
#ifdef ATIVAR_ROP  
      addr2 = 0x00004800;		//OptionByte ROP
#ifndef STM8L052R8     
      rop = 0xAA;// retirar para não gravar o ROP
#else
      rop = 0x00;// retirar para não gravar o ROP
#endif
      if(SWIM_WOTF(addr2, 1, &rop))
      {
        SWIM_Estado = SWIM_FLASH_ROP;
      }
#endif      
      DELAYTIMER_DelayUS(1000);
      SWIM_Estado = SWIM_FLASH_LOCK;
      
      break;}                        
    case SWIM_ERROR:
    case SWIM_IDLE:{
      break;}
    }
  }
  
  while (1)
  {
    /* Toggle JTMS/SWDAT pin */
    GPIO_WriteBit(GPIOA, GPIO_Pin_13, (BitAction)(1 - GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_13)));
    GPIO_WriteBit(GPIOA, GPIO_Pin_8, (BitAction)(Bit_RESET));
  }
  
  
}


#ifdef  USE_FULL_ASSERT
/**
* @brief  Reports the name of the source file and the source line number
*         where the assert_param error has occurred.
* @param  file: pointer to the source file name
* @param  line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
* @}
*/

/**
* @}
*/

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
