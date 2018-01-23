#include "flash.h"
#include  "stm32f0xx.h"


FLASH_CHANNEL OUT1_ModeFlashChannel;
FLASH_CHANNEL OUT1_ValueFlashChannel;
FLASH_CHANNEL CSVFlashChannel;
FLASH_CHANNEL Threshold_ModeFlashChannel;
FLASH_CHANNEL DACOUT_ModeFlashChannel;
FLASH_CHANNEL KEY_ModeFlashChannel;
FLASH_CHANNEL RegisterB_ModeFlashChannel;
FLASH_CHANNEL DEL_ModeFlashChannel;
FLASH_CHANNEL HI_ModeFlashChannel;
FLASH_CHANNEL LO_ModeFlashChannel;
FLASH_CHANNEL DETECT_ModeFlashChannel;

/*  读FLASH
ProgramCounter 		= *(__IO uint32_t*)(ProgramRUNcounter_Mode_FLASH_DATA_ADDRESS);
*/

void InitInternalFlash(uint32_t addr)
{
	FLASH_Unlock(); //解锁FLASH编程擦除控制器
	FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPERR);//清除标志位
	FLASH_ErasePage(addr); //擦除指定地址页
	FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPERR);//清除标志位
	FLASH_Lock(); //锁定FLASH编程擦除控制器
}

FLASH_Status WriteInternalFlash(uint32_t addr,uint32_t data)
{
	FLASH_Unlock(); //解锁FLASH编程擦除控制器
	FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPERR);//清除标志位
	//FLASH_ErasePage(addr); //擦除指定地址页
	FLASH_ProgramWord(addr ,data); //从指定页的0地址开始写
	FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPERR);//清除标志位
	FLASH_Lock(); //锁定FLASH编程擦除控制器
}


/******************************************
擦除FLASH
*******************************************/
void EraseFlash(void)
{	
	InitInternalFlash(MAKCurrentThreshold_FLASH_DATA_ADDRESS);
	InitInternalFlash(CurrentPWM_FLASH_DATA_ADDRESS);
	InitInternalFlash(CICurrentThreshold_FLASH_DATA_ADDRESS);
	InitInternalFlash(SA_FLASH_DATA_ADDRESS);
	InitInternalFlash(CXA_FLASH_DATA_ADDRESS);
	InitInternalFlash(CYA_FLASH_DATA_ADDRESS);
	InitInternalFlash(CZA_FLASH_DATA_ADDRESS);

}

/******************************************
写FLASH
*******************************************/
uint32_t tempflashdata;
void WriteFlash(uint32_t addr,uint32_t data)
{	
		uint32_t LastAddr = addr;
		tempflashdata 		= *(__IO uint32_t*)(addr);
		while(tempflashdata!=0xFFFFFFFF)
		{
			addr = addr + 4;
			if(addr-LastAddr>1024)
				InitInternalFlash(LastAddr);
			tempflashdata = *(__IO uint32_t*)(addr);
		}
		WriteInternalFlash(addr,data);
		
}

/******************************************
读FLASH
*******************************************/
//uint32_t tempflashdata;
uint32_t  ReadFlash(uint32_t addr)
{	
		uint32_t HeadAddr = addr;
		uint32_t LastAddr = addr;
		tempflashdata 		= *(__IO uint32_t*)(addr);
		while(tempflashdata!=0xFFFFFFFF)
		{
			 LastAddr = addr;
			addr = addr + 4;
			tempflashdata 		= *(__IO uint32_t*)(addr);
		}
		return tempflashdata 		= *(__IO uint32_t*)(LastAddr);
}
