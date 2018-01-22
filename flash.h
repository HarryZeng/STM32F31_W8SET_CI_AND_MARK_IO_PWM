#/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __flash_H
#define __flash_H
	#ifdef __cplusplus
	 extern "C" {
	#endif
		 
#include  "stm32f0xx.h"
	 
#define 	FLASH_Base_ADDRESS															0x08009000

#define 	ProgramRUNcounter_Mode_FLASH_DATA_ADDRESS 	 		FLASH_Base_ADDRESS+0x000
#define 	OUT1_Mode_FLASH_DATA_ADDRESS 	 									FLASH_Base_ADDRESS+0x400
#define 	OUT1_Value_FLASH_DATA_ADDRESS 	 								FLASH_Base_ADDRESS+0x800
#define 	SV_FLASH_DATA_ADDRESS 	 												FLASH_Base_ADDRESS+0xC00
#define 	Threshold_FLASH_DATA_ADDRESS 	 									FLASH_Base_ADDRESS+0x1000
#define 	ATT100_FLASH_DATA_ADDRESS 	 										FLASH_Base_ADDRESS+0x1400
#define 	KEY_FLASH_DATA_ADDRESS 	 												FLASH_Base_ADDRESS+0x1800
#define 	RegisterB_FLASH_DATA_ADDRESS 	 									FLASH_Base_ADDRESS+0x1c00
#define 	FSV_FLASH_DATA_ADDRESS 	 												FLASH_Base_ADDRESS+0x2000
#define 	HI_FLASH_DATA_ADDRESS 	 												FLASH_Base_ADDRESS+0x2400
#define 	LO_FLASH_DATA_ADDRESS 	 												FLASH_Base_ADDRESS+0x2800
#define 	DETECT_FLASH_DATA_ADDRESS 	 										FLASH_Base_ADDRESS+0x2C00	 
#define 	PERCENTAGE_FLASH_DATA_ADDRESS 	 								FLASH_Base_ADDRESS+0x3000
#define 	S_SET_FLASH_DATA_ADDRESS 	 											FLASH_Base_ADDRESS+0x3400
#define 	DSC_FLASH_DATA_ADDRESS 	 												FLASH_Base_ADDRESS+0x3800


typedef  struct FLASH_CHANNEL
{
	uint32_t BaseAddress;
	uint16_t AddressIndex;
		
}FLASH_CHANNEL;	
	 
void WriteFlash(uint32_t addr,uint32_t data);	 
uint32_t  ReadFlash(uint32_t addr);
void EraseFlash(void);

#endif
