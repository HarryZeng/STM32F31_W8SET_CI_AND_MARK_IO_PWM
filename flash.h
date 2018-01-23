#/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __flash_H
#define __flash_H
	#ifdef __cplusplus
	 extern "C" {
	#endif
		 
#include  "stm32f0xx.h"
	 
#define 	FLASH_Base_ADDRESS															0x08005000

#define 	ProgramRUNcounter_Mode_FLASH_DATA_ADDRESS 	 			FLASH_Base_ADDRESS+0x000
#define 	MAKCurrentThreshold_FLASH_DATA_ADDRESS 	 					FLASH_Base_ADDRESS+0x400
#define 	CurrentPWM_FLASH_DATA_ADDRESS 	 									FLASH_Base_ADDRESS+0x800
#define 	CICurrentThreshold_FLASH_DATA_ADDRESS 	 					FLASH_Base_ADDRESS+0xC00
#define 	SA_FLASH_DATA_ADDRESS 	 												FLASH_Base_ADDRESS+0x1000
#define 	CXA_FLASH_DATA_ADDRESS 	 												FLASH_Base_ADDRESS+0x1400
#define 	CYA_FLASH_DATA_ADDRESS 	 												FLASH_Base_ADDRESS+0x1800
#define 	CZA_FLASH_DATA_ADDRESS 	 												FLASH_Base_ADDRESS+0x1C00


typedef  struct FLASH_CHANNEL
{
	uint32_t BaseAddress;
	uint16_t AddressIndex;
		
}FLASH_CHANNEL;	
	 
void WriteFlash(uint32_t addr,uint32_t data);	 
uint32_t  ReadFlash(uint32_t addr);
void EraseFlash(void);

#endif
