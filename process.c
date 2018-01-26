///**
//  ********************************  STM32F0xx  *********************************
//  * @文件名     ： process.c
//  * @作者       ： HarryZeng
//  * @库版本     ： V1.5.0
//  * @文件版本   ： V1.0.0
//  * @日期       ： 2017年04月21日
//  * @摘要       ： 数据处理
//  ******************************************************************************/
///*----------------------------------------------------------------------------
//  更新日志:
//  2017-04-21 V1.0.0:初始版本
//  ----------------------------------------------------------------------------*/
///* 包含的头文件 --------------------------------------------------------------*/
/* Includes ------------------------------------------------------------------*/
#include "process.h"
#include "flash.h"
/*----------------------------------宏定义-------------------------------------*/


/*------------------------------------函数声明---------------------------------------*/
void 	MARK_GetRegisterAState(void);
void 	CI_GetRegisterAState(void);
uint32_t 	Read_Value(PWM_Number PWM);
uint8_t  	Read_GOODBAD(void);
void  		SetOut(uint8_t OUT);
void  CI_Mode_SelfLearning(void);
void  MARK_Mode_SelfLearning(void);
void 	scan_key(void);
void 	printFlashTest(void);
void 	ShortCircuitProtection(void);
void 	WriteFlash(uint32_t addr,uint32_t data);
extern void DelaymsSet(int16_t ms);
void SET_GOODBAD(void);
void GetEEPROM(void);
void CI_PWM_OUT(void);
uint8_t Get_FB_Flag(void);
/*------------------------------------全局变量---------------------------------------*/
uint32_t ADC_value = 0;
uint8_t 	ShortCircuit=0;
uint8_t 	ConfirmShortCircuit=0;
uint32_t 		ShortCircuitTimer=0;
uint32_t   ShortCircuitCounter = 0;
uint32_t   ShortCircuitLastTime = 0;
/*状态变量*/
uint8_t RegisterA = 0;
uint8_t RegisterA_Comfirm = 0;
uint8_t OUT;

	uint32_t SX[4],SY[4],SZ[4];
	float SXA_B[2],SYA_B[2],SZA_B[2];

	float X=0,Y=0,Z=0,BIG=0;
	uint32_t SelfLADC=0;
	uint8_t SelfGetADCWell=0;
	uint32_t temppp;
	extern uint8_t DMAIndex;
	extern int16_t selfADCValue[12];

PWM_Number CurrentPWM = PWMX; //默认当前PWM通道为X
uint32_t S_Last,S_Current,S_History,S_FINAL;
int CICurrentThreshold=500;
int MAKCurrentThreshold=500;

uint32_t RegisterACounter=0;

uint8_t KeyTime;
uint16_t key_counter=0;
uint16_t scan_tick=0;
uint8_t KeyIndex=0;
uint32_t FLASHData;
uint8_t EnterSelfFlag=0;
extern int16_t adc_dma_tab[6];
extern uint8_t sample_finish;
extern uint8_t TIM1step;

/********************************/
uint32_t CXA_B[2],CYA_B[2],CZA_B[2];
uint32_t SA_B[2];
uint8_t FB_Flag=1;
uint32_t NXSET,NYSET,NZSET,NS_SET,NXYZ_SET;

float SX_RUN,SY_RUN,SZ_RUN,S_RUN_TOTAL;
int32_t CX_RUN,CY_RUN,CZ_RUN,NS_RUN,NXYZ_RUN;
int32_t CX,CY,CZ;

int32_t SCI,SMARK;
int32_t SCI_Min,SCI_Max;
int32_t DX_Data[8];
int16_t DX_Max = 0,DX_Min=4095;
uint8_t DX_Index = 0;
int DX=0;

int32_t DX2_Data[4];
int16_t DX2_Max = 0,DX2_Min=4095;
uint8_t DX2_Index = 0;
int DX2=0;

uint16_t  GoodBadTime=0;
/***********************************
*FLASH 字节定义
*0x12000032
解析：32位中,最高4位代表PWM通道的选择：PWMX->1，PWMY->2，PWMZ->4
						 紧接着4位就是应差值的选择：20->1,50->2,80->4
						 剩下的24位则保存则阈值
						 如：0x12000032  代表有效通道:PWMX    应差值选:50   阈值:50
************************************/
uint32_t FLASHData = 0x12000032;
/*----------------------------------函数定义----------------------------------------*/
/*****************
*
*主数据处理函数
*
*****************/
void DataProcess(void)
{
	int First_ten_times;
	uint8_t  OUTPin_STATE;
	/*
		FALSH 读取参数
	*/
	GetEEPROM();
	
	for(First_ten_times = 0;First_ten_times<10;First_ten_times++) /*刚上电，前十组PWM只算 RegisterA*/
	{
			//GetRegisterAState();
	}
	
	while(1)
	{
		/*短路保护判断*/
		//ShortCircuitProtection();
		
//		while(ConfirmShortCircuit)
//		{
//				GPIO_WriteBit(OUT_GPIO_Port, OUT_Pin, Bit_RESET); /*发现短路，将OUT引脚拉低*/
//				if((ShortCircuitLastTime - ShortCircuitTimer)>=1000)
//				{
//						ConfirmShortCircuit = 0;
//						ShortCircuitCounter = 0;
//						ShortCircuit=0;
//				}
//		}

		//FB_Flag = 1;
			//CurrentPWM = PWMY;
		if(KeyIndex<1)   /*小于1则，KeyIndex =0 没按键响应，当KeyIndex>=1时，则按键过SET按键*/
		{
			/*根据FB电平高低判断RegisterA*/
			if(FB_Flag ==1)
			{
				CI_GetRegisterAState();
			}
			else if(FB_Flag==0)
			{
				MARK_GetRegisterAState();
			}
			
		}
		else
		{
			CI_PWM_OUT();
			IWDG_ReloadCounter();
			scan_key();
			while(TIM_GetCounter(MainTIMER)<PWM1_HIGH);//一组累加完成，等待
		}
		
		/*按键进入自学习模式*/
		if(KeyTime>0)  
		{
			OUTPin_STATE = GPIO_ReadInputDataBit(OUT_GPIO_Port,OUT_Pin); //读取OUT的值,用于写FLASH时，保持OUT的引脚电平不变
			GPIO_WriteBit(OUT_GPIO_Port, OUT_Pin, (BitAction)OUTPin_STATE);
			if(FB_Flag)
			{
				CI_Mode_SelfLearning();  //CI MODE
			}
			else
			{
				MARK_Mode_SelfLearning();//MARK MODE
			}
		}


		/*按键扫描*/
		//scan_key();
		/*FB Flag*/
		//FB_Flag = Get_FB_Flag();
		//FB_Flag = 1;
		//CurrentPWM = PWMX;
		GoodBadTime++;
	}
}

/********************
*
*判断出CI模式下PWM输出
*
**********************/
extern uint8_t ADC_Conversion_Flag;

void CI_PWM_OUT(void)
{
	TIM_SetCounter(MainTIMER,0X00);
	
	PWM1_ON;	
	/*PWMX的ADC开始*/
	PWMX_ON;
	while(TIM_GetCounter(MainTIMER)<PWMx_HIGH);// 拉高PWMX 1us
	PWMX_OFF;
	ADC_Conversion_Flag = 0;
	ADC_StartOfConversion(ADC1);
	while(ADC_Conversion_Flag==0);  //等待PWMX的ADC采集完成
	ADC_Conversion_Flag = 0;
	/*PWMX的ADC完成*/
	PWMY_ON;
	while(TIM_GetCounter(MainTIMER)<PWMy_HIGH);// 拉高PWMX 1us
	PWMY_OFF;
	ADC_Conversion_Flag = 0;
	ADC_StartOfConversion(ADC1);
	while(ADC_Conversion_Flag==0);  //等待PWMX的ADC采集完成
	ADC_Conversion_Flag = 0;
	/*PWMY的ADC完成*/
	PWMZ_ON;
	while(TIM_GetCounter(MainTIMER)<PWMz_HIGH);// 拉高PWMX 1us
	PWMZ_OFF;
	ADC_Conversion_Flag = 0;
	ADC_StartOfConversion(ADC1);
	while(ADC_Conversion_Flag==0);  //等待PWMX的ADC采集完成
	ADC_Conversion_Flag = 0;
	/*PWMY的ADC完成*/	
	PWM1_OFF;
}

/********************
*
*判断出CI模式下RegisterA状态
*
**********************/
extern int16_t  RunTime; 
extern uint8_t ADCIndex;
uint8_t CI_PWMx_y_z_Counter = 0;
uint8_t CI_PWMx_y_z_TotalCounter = 0;
uint8_t RegisterA_1Counter = 0;
uint8_t RegisterA_0Counter = 0;

void CI_GetRegisterAState(void)
{
	CI_PWM_OUT();
	 
	SX[CI_PWMx_y_z_TotalCounter] = selfADCValue[CI_PWMx_y_z_Counter++];
	SY[CI_PWMx_y_z_TotalCounter] = selfADCValue[CI_PWMx_y_z_Counter++];
	SZ[CI_PWMx_y_z_TotalCounter] = selfADCValue[CI_PWMx_y_z_Counter++];
	CI_PWMx_y_z_TotalCounter++;
	if(CI_PWMx_y_z_TotalCounter<4)
	{
		//scan_key(); //按键扫面
		while(TIM_GetCounter(MainTIMER)<PWM1_HIGH);//一组累加完成，等待
	}
	else if(CI_PWMx_y_z_TotalCounter>3)  //4组，12个数据完成
		{
			
			ADCIndex = 0;
			CI_PWMx_y_z_TotalCounter = 0;
			CI_PWMx_y_z_Counter = 0;
			
			SX_RUN = (SX[0]+SX[1]+SX[2]+SX[3])/4; //累加求平均
			SY_RUN = (SY[0]+SY[1]+SY[2]+SY[3])/4;
			SZ_RUN = (SZ[0]+SZ[1]+SZ[2]+SZ[3])/4;
		
			S_RUN_TOTAL = SX_RUN+SY_RUN+SZ_RUN;
			
			CX = 1024*SX_RUN/S_RUN_TOTAL;   //1->SXA,2->SXB
			CY = 1024*SY_RUN/S_RUN_TOTAL;   //1->SXA,2->SXB
			CZ = 1024*SZ_RUN/S_RUN_TOTAL;   //1->SXA,2->SXB
			
			if(S_RUN_TOTAL>SA_B[0])
				NS_RUN = S_RUN_TOTAL - SA_B[0];  /*NSR_RUN = S-SA 绝对值*/
			else
				NS_RUN = SA_B[0] - S_RUN_TOTAL;
			
			if(CX>CXA_B[0])
				CX_RUN =	CX - CXA_B[0];  /*CX_RUN=CX-CXB的绝对值*/
			else
				CX_RUN =	CXA_B[0] - CX;
			
			if(CY > CYA_B[0])
				CY_RUN = 	CY - CYA_B[0];
			else
				CY_RUN = 	CYA_B[0] - CY;
			
			if(CZ > CZA_B[0] )
				CZ_RUN = 	CZ - CZA_B[0]; 
			else
				CZ_RUN = 	CZA_B[0] - CZ; 
			
			NXYZ_RUN = CX_RUN+CY_RUN+CZ_RUN;
			
			/************SCI**********/
			SCI = 1000 - (NS_RUN + NXYZ_RUN);  //2018-1-17  change
			if(SCI<=0)
				SCI = 0;
			else if(SCI>=1000)
				SCI  = 1000;
			/************DX**********/
			DX_Data[DX_Index] = SCI;
			if(DX_Data[DX_Index]>DX_Max)
				DX_Max = DX_Data[DX_Index];
			if(DX_Data[DX_Index] < DX_Min)
				DX_Min = DX_Data[DX_Index];
			DX_Index++;
			if(DX_Index>7)
			{
				DX_Index = 0;
				DX = DX_Max - DX_Min;
				DX_Max = 0;
				DX_Min = 4095;
			}
			
			/************DX2*************/
			DX2_Data[DX2_Index] = SCI;
			if(DX2_Data[DX2_Index]>DX2_Max)
				DX2_Max = DX2_Data[DX2_Index];
			if(DX2_Data[DX2_Index] < DX2_Min)
				DX2_Min = DX2_Data[DX2_Index];
			DX2_Index++;
			if(DX2_Index>3)
			{
				DX2_Index = 0;
				DX2 = DX2_Max - DX2_Min;
				DX2_Max = 0;
				DX2_Min = 4095;
			}			

			/***********RegisterA***********/
			
			SCI_Max = CICurrentThreshold + DX/2;
			SCI_Min = CICurrentThreshold - DX - 90; 
			
			if(SCI_Min<10)
				 SCI_Min= 10;
			
			/*判断SCI的范围，并输出RegisterA的值*/
			if(SCI >= SCI_Max)
			{
				RegisterA_0Counter = 0;
				RegisterA_1Counter++;
				if(RegisterA_1Counter>=4)
				{
					RegisterA_1Counter = 0;
					RegisterA = 1;
				}
			}
			else if(SCI <= SCI_Min)
			{
				RegisterA_1Counter = 0;
				RegisterA_0Counter++;
				if(RegisterA_0Counter>=4)
				{
					RegisterA_0Counter = 0;
					RegisterA = 0;
				}
			}
			IWDG_ReloadCounter();//看门狗喂狗
			SET_GOODBAD();
			FB_Flag = Get_FB_Flag();
			scan_key();
			while(TIM_GetCounter(MainTIMER)<PWM1_HIGH){}//一组累加完成，等待
			
			//RunTime = TIM_GetCounter(MainTIMER);
		}
}

/********************
*
*判断出MARK模式下RegisterA状态
*
**********************/

uint8_t MAK_PWMx_y_z_Counter = 0;
uint8_t MAK_PWMx_y_z_TotalCounter = 0;

void MARK_PWM_OUT(PWM_Number PWM)
{
	int32_t SMARK_Min;
	/*开启对应的PWM通道*/

	if(PWM ==  PWMX)
	{
		TIM_SetCounter(MainTIMER,0X00);
		PWM1_ON;	
		PWMY_OFF;
		PWMZ_OFF;
		PWMX_ON;
		/*PWMX的ADC开始*/
		while(TIM_GetCounter(MainTIMER)<PWMx_HIGH);// 拉高PWMX 1us
		PWMX_OFF;
		ADC_Conversion_Flag = 0;
		ADC_StartOfConversion(ADC1);
		while(ADC_Conversion_Flag==0);  //等待PWMX的ADC采集完成
		ADC_Conversion_Flag = 0;
		while(TIM_GetCounter(MainTIMER)<PWM1_LOW);// 拉高PWMX 1us
		PWM1_OFF;	
		SX[MAK_PWMx_y_z_TotalCounter] = selfADCValue[MAK_PWMx_y_z_Counter++];
		MAK_PWMx_y_z_TotalCounter++;
		if(MAK_PWMx_y_z_TotalCounter<4)
		{
			while(TIM_GetCounter(MainTIMER)<PWM1_HIGH);//一组累加完成，等待
		}
		else if(MAK_PWMx_y_z_TotalCounter>3)  //4组，12个数据完成
		{
			ADCIndex = 0;
			MAK_PWMx_y_z_TotalCounter = 0;
			MAK_PWMx_y_z_Counter = 0;
			SMARK = (SX[0]+SX[1]+SX[2]+SX[3])/4; //累加求平均
			if(SMARK<10)
				SMARK= 10;	
			if(SMARK>=4095)
				SMARK = 4095;
			
			DX_Data[DX_Index] = SMARK;
			if(DX_Data[DX_Index]>DX_Max)
				DX_Max = DX_Data[DX_Index];
			if(DX_Data[DX_Index] < DX_Min)
				DX_Min = DX_Data[DX_Index];
			DX_Index++;
			if(DX_Index>7)
			{
				DX_Index = 0;
				DX = DX_Max - DX_Min;
				DX_Max = 0;
				DX_Min = 4095;
			}
			/************DX2*************/
			DX2_Data[DX2_Index] = SMARK;
			if(DX2_Data[DX2_Index]>DX2_Max)
				DX2_Max = DX2_Data[DX2_Index];
			if(DX2_Data[DX2_Index] < DX2_Min)
				DX2_Min = DX2_Data[DX2_Index];
			DX2_Index++;
			if(DX2_Index>3)
			{
				DX2_Index = 0;
				DX2 = DX2_Max - DX2_Min;
				DX2_Max = 0;
				DX2_Min = 4095;
			}	
			
			SMARK_Min = MAKCurrentThreshold*7/8- DX -50 ;
			if(SMARK_Min<0)
				SMARK_Min = 0;
			
				if(SMARK > MAKCurrentThreshold*7/8 && SMARK < MAKCurrentThreshold*9/8 )
				{
						RegisterA_0Counter = 0;
						RegisterA_1Counter++;
						if(RegisterA_1Counter>=4)
						{
							RegisterA_1Counter = 0;
							RegisterA = 1;
						}
				}
				else if(SMARK <= SMARK_Min || SMARK>=MAKCurrentThreshold*9/8+DX+50)
				{
						RegisterA_1Counter = 0;
						RegisterA_0Counter++;
						if(RegisterA_0Counter>=4)
						{
							RegisterA_0Counter = 0;
							RegisterA = 0;
						}
				}
				IWDG_ReloadCounter();
				SET_GOODBAD();
				FB_Flag = Get_FB_Flag();
				scan_key();
				while(TIM_GetCounter(MainTIMER)<PWM1_HIGH);//一组累加完成，等待
		}
	}
		
	else if(PWM == PWMY)
	{
		TIM_SetCounter(MainTIMER,0X00);
		PWM1_ON;	
		PWMX_OFF;
		PWMZ_OFF;
		PWMY_ON;
		/*PWMX的ADC开始*/
		while(TIM_GetCounter(MainTIMER)<PWMx_HIGH);// 拉高PWMX 1us
		PWMY_OFF;
		ADC_Conversion_Flag = 0;
		ADC_StartOfConversion(ADC1);
		while(ADC_Conversion_Flag==0);  //等待PWMX的ADC采集完成
		ADC_Conversion_Flag = 0;
		while(TIM_GetCounter(MainTIMER)<PWM1_LOW);// 拉高PWMX 1us
		PWM1_OFF;	
		SX[MAK_PWMx_y_z_TotalCounter] = selfADCValue[MAK_PWMx_y_z_Counter++];
		MAK_PWMx_y_z_TotalCounter++;
		if(MAK_PWMx_y_z_TotalCounter<4)
		{
			while(TIM_GetCounter(MainTIMER)<PWM1_HIGH);//一组累加完成，等待
		}
		else if(MAK_PWMx_y_z_TotalCounter>3)  //4组，12个数据完成
		{
			ADCIndex = 0;
			MAK_PWMx_y_z_TotalCounter = 0;
			MAK_PWMx_y_z_Counter = 0;
			SMARK = (SX[0]+SX[1]+SX[2]+SX[3])/4; //累加求平均
			if(SMARK<10)
				SMARK= 10;	
			if(SMARK>=4095)
				SMARK = 4095;
			
			DX_Data[DX_Index] = SMARK;
			if(DX_Data[DX_Index]>DX_Max)
				DX_Max = DX_Data[DX_Index];
			if(DX_Data[DX_Index] < DX_Min)
				DX_Min = DX_Data[DX_Index];
			DX_Index++;
			if(DX_Index>7)
			{
				DX_Index = 0;
				DX = DX_Max - DX_Min;
				DX_Max = 0;
				DX_Min = 4095;
			}
			/************DX2*************/
			DX2_Data[DX2_Index] = SMARK;
			if(DX2_Data[DX2_Index]>DX2_Max)
				DX2_Max = DX2_Data[DX2_Index];
			if(DX2_Data[DX2_Index] < DX2_Min)
				DX2_Min = DX2_Data[DX2_Index];
			DX2_Index++;
			if(DX2_Index>3)
			{
				DX2_Index = 0;
				DX2 = DX2_Max - DX2_Min;
				DX2_Max = 0;
				DX2_Min = 4095;
			}	
			
			SMARK_Min = MAKCurrentThreshold*7/8- DX -50 ;
			if(SMARK_Min<0)
				SMARK_Min = 0;
			
				if(SMARK > MAKCurrentThreshold*7/8 && SMARK < MAKCurrentThreshold*9/8 )
				{
						RegisterA_0Counter = 0;
						RegisterA_1Counter++;
						if(RegisterA_1Counter>=4)
						{
							RegisterA_1Counter = 0;
							RegisterA = 1;
						}
				}
				else if(SMARK <= SMARK_Min || SMARK>=MAKCurrentThreshold*9/8+DX+50)
				{
						RegisterA_1Counter = 0;
						RegisterA_0Counter++;
						if(RegisterA_0Counter>=4)
						{
							RegisterA_0Counter = 0;
							RegisterA = 0;
						}
				}
				IWDG_ReloadCounter();
				SET_GOODBAD();
				FB_Flag = Get_FB_Flag();
				scan_key();
				while(TIM_GetCounter(MainTIMER)<PWM1_HIGH);//一组累加完成，等待
		}
	}
	
	else if(PWM ==  PWMZ)
	{
		TIM_SetCounter(MainTIMER,0X00);
		PWM1_ON;	
		PWMX_OFF;
		PWMY_OFF;
		PWMZ_ON;
		/*PWMX的ADC开始*/
		while(TIM_GetCounter(MainTIMER)<PWMx_HIGH);// 拉高PWMX 1us
		PWMZ_OFF;
		ADC_Conversion_Flag = 0;
		ADC_StartOfConversion(ADC1);
		while(ADC_Conversion_Flag==0);  //等待PWMX的ADC采集完成
		ADC_Conversion_Flag = 0;
		while(TIM_GetCounter(MainTIMER)<PWM1_LOW);// 拉高PWMX 1us
		PWM1_OFF;	
		SX[MAK_PWMx_y_z_TotalCounter] = selfADCValue[MAK_PWMx_y_z_Counter++];
		MAK_PWMx_y_z_TotalCounter++;
		if(MAK_PWMx_y_z_TotalCounter<4)
		{
			while(TIM_GetCounter(MainTIMER)<PWM1_HIGH);//一组累加完成，等待
		}
		else if(MAK_PWMx_y_z_TotalCounter>3)  //4组，12个数据完成
		{
			ADCIndex = 0;
			MAK_PWMx_y_z_TotalCounter = 0;
			MAK_PWMx_y_z_Counter = 0;
			SMARK = (SX[0]+SX[1]+SX[2]+SX[3])/4; //累加求平均
			if(SMARK<10)
				SMARK= 10;	
			if(SMARK>=4095)
				SMARK = 4095;
			
			DX_Data[DX_Index] = SMARK;
			if(DX_Data[DX_Index]>DX_Max)
				DX_Max = DX_Data[DX_Index];
			if(DX_Data[DX_Index] < DX_Min)
				DX_Min = DX_Data[DX_Index];
			DX_Index++;
			if(DX_Index>7)
			{
				DX_Index = 0;
				DX = DX_Max - DX_Min;
				DX_Max = 0;
				DX_Min = 4095;
			}	
			/************DX2*************/
			DX2_Data[DX2_Index] = SMARK;
			if(DX2_Data[DX2_Index]>DX2_Max)
				DX2_Max = DX2_Data[DX2_Index];
			if(DX2_Data[DX2_Index] < DX2_Min)
				DX2_Min = DX2_Data[DX2_Index];
			DX2_Index++;
			if(DX2_Index>3)
			{
				DX2_Index = 0;
				DX2 = DX2_Max - DX2_Min;
				DX2_Max = 0;
				DX2_Min = 4095;
			}	
			
			SMARK_Min = MAKCurrentThreshold*7/8- DX -50 ;
			if(SMARK_Min<0)
				SMARK_Min = 0;
			
				if(SMARK > MAKCurrentThreshold*7/8 && SMARK < MAKCurrentThreshold*9/8 )
				{
						RegisterA_0Counter = 0;
						RegisterA_1Counter++;
						if(RegisterA_1Counter>=4)
						{
							RegisterA_1Counter = 0;
							RegisterA = 1;
						}
				}
				else if(SMARK <= SMARK_Min || SMARK>=MAKCurrentThreshold*9/8+DX+50)
				{
						RegisterA_1Counter = 0;
						RegisterA_0Counter++;
						if(RegisterA_0Counter>=4)
						{
							RegisterA_0Counter = 0;
							RegisterA = 0;
						}
				}
				IWDG_ReloadCounter();
				SET_GOODBAD();
				FB_Flag = Get_FB_Flag();
				scan_key();
				while(TIM_GetCounter(MainTIMER)<PWM1_HIGH);//一组累加完成，等待
		}
	}
	
}


void MARK_GetRegisterAState(void)
{
		MARK_PWM_OUT(CurrentPWM);
}

/***************************************
*
*读取KG拨码开关的值
*
**************************************/
uint8_t Get_FB_Flag(void)
{
	
	return GPIO_ReadInputDataBit(FB_GPIO_Port,FB_Pin);
	
}


/***************************************
*
*Good Bad的LED显示
*
**************************************/
void  SET_GOODBAD(void)
{
	uint8_t  GOODBAD_STATE;
	if(FB_Flag==1)
	{
		if(DX2<=80)
		{
			if(S_RUN_TOTAL>=500)
			{
				SetOut(RegisterA);
				GPIO_WriteBit(GOODBAD_GPIO_Port,GOODBAD_Pin,Bit_SET); //读取KG的值
			}
			else if(S_RUN_TOTAL<500)
			{
				if(GoodBadTime>=4100)
				{
					GoodBadTime = 0;
					GPIO_WriteBit(GOODBAD_GPIO_Port, GOODBAD_Pin, (BitAction)!GPIO_ReadOutputDataBit(GOODBAD_GPIO_Port, GOODBAD_Pin));
				}
				GPIO_WriteBit(OUT_GPIO_Port, OUT_Pin, Bit_RESET);
			}
		}
	}
	else if(FB_Flag==0)
	{
		if(DX2<=80)
		{
			if(SMARK>140)
			{
				SetOut(RegisterA);
				GPIO_WriteBit(GOODBAD_GPIO_Port,GOODBAD_Pin,Bit_SET); 
			}
			else if(SMARK<=140)
			{
				if(GoodBadTime>=4100)
				{
					GoodBadTime = 0;
					GPIO_WriteBit(GOODBAD_GPIO_Port, GOODBAD_Pin, (BitAction)!GPIO_ReadOutputDataBit(GOODBAD_GPIO_Port, GOODBAD_Pin));
				}
				GPIO_WriteBit(OUT_GPIO_Port, OUT_Pin, Bit_RESET);
			}
		}
	}
}

/***************************************
*
*设置OUT的输出电平
*
**************************************/
void  SetOut(uint8_t OUT_Value)
{
	if(OUT_Value==1)
	{
		GPIO_WriteBit(OUT_GPIO_Port, OUT_Pin, Bit_SET);
	}
	else
	{
		GPIO_WriteBit(OUT_GPIO_Port, OUT_Pin, Bit_RESET);
	}
}

/***************************************
*
*自学习计算
*
**************************************/
void  MARK_Mode_SelfLearning(void)
{
	uint8_t SelfLearn_PWMx_y_zTotalCounter = 0;
	uint8_t SelfLearn_PWMx_y_z_Counter = 0;

	while(SelfLearn_PWMx_y_zTotalCounter<3)
	{
		CI_PWM_OUT(); 
		SX[SelfLearn_PWMx_y_zTotalCounter] = selfADCValue[SelfLearn_PWMx_y_z_Counter++];
		SY[SelfLearn_PWMx_y_zTotalCounter] = selfADCValue[SelfLearn_PWMx_y_z_Counter++];
		SZ[SelfLearn_PWMx_y_zTotalCounter] = selfADCValue[SelfLearn_PWMx_y_z_Counter++];
		SelfLearn_PWMx_y_zTotalCounter++;
		//scan_key(); //按键扫面
		while(TIM_GetCounter(MainTIMER)<PWM1_HIGH);//一组累加完成，等待
	}
		CI_PWM_OUT(); 
		SX[SelfLearn_PWMx_y_zTotalCounter] = selfADCValue[SelfLearn_PWMx_y_z_Counter++];
		SY[SelfLearn_PWMx_y_zTotalCounter] = selfADCValue[SelfLearn_PWMx_y_z_Counter++];
		SZ[SelfLearn_PWMx_y_zTotalCounter] = selfADCValue[SelfLearn_PWMx_y_z_Counter++];
		SelfLearn_PWMx_y_zTotalCounter++;	
	/*4组，12个数据完成*/
			ADCIndex = 0;
			SelfLearn_PWMx_y_zTotalCounter = 0;
			SelfLearn_PWMx_y_z_Counter = 0;
		
			SXA_B[KeyIndex] = (SX[0]+SX[1]+SX[2]+SX[3])/4; //累加求平均
			SYA_B[KeyIndex] = (SY[0]+SY[1]+SY[2]+SY[3])/4;
			SZA_B[KeyIndex] = (SZ[0]+SZ[1]+SZ[2]+SZ[3])/4;
	 /*等待获取四组ADC成功*/
		
		KeyIndex++;  //记录第几次按键   1->SXA,2->SXB
		if(KeyIndex>=2) //第二次按键
		{
				KeyIndex = 0;
				/*计算大小绝对值*/
				/*----------PWMX对比大小--------*/
				if(SXA_B[0]>=SXA_B[1])
				{
						X = (SXA_B[0] - SXA_B[1]);
				}
				else
				{
						X = (SXA_B[1] - SXA_B[0]);
				}
				/*----------PWMY对比大小--------*/
				if(SYA_B[0]>=SYA_B[1])
				{
						Y = (SYA_B[0] - SYA_B[1]);
				}
				else
				{
						Y = (SYA_B[1] - SYA_B[0]);
				}
				/*----------PWMZ对比大小--------*/
				if(SZA_B[0]>=SZA_B[1])
				{
						Z = (SZA_B[0] - SZA_B[1]);
				}
				else
				{
						Z = (SZA_B[1] - SZA_B[0]);
				}
				/*找最大值,计算阈值，判断PWM通道*/
				if(X>=Y)
					BIG = X;
				else
					BIG = Y;
				if(BIG<=Z)
					BIG = Z;

//					BIG=(X>Y)?X:Y;
//					BIG=(BIG>Z)?BIG:Z;
					//BIG= Y; //Debug 2018-1-25
					if(BIG==X)
					{
						MAKCurrentThreshold = SXA_B[0];
						CurrentPWM = PWMX;
						WriteFlash(MAKCurrentThreshold_FLASH_DATA_ADDRESS,MAKCurrentThreshold);
						WriteFlash(CurrentPWM_FLASH_DATA_ADDRESS,CurrentPWM);
					}
					else if(BIG==Y)
					{
						MAKCurrentThreshold = SYA_B[0];
						CurrentPWM = PWMY;
						WriteFlash(MAKCurrentThreshold_FLASH_DATA_ADDRESS,MAKCurrentThreshold);
						WriteFlash(CurrentPWM_FLASH_DATA_ADDRESS,CurrentPWM);
					}
					else	if(BIG==Z)
					{
						MAKCurrentThreshold = SZA_B[0];
						CurrentPWM = PWMZ;
						WriteFlash(MAKCurrentThreshold_FLASH_DATA_ADDRESS,MAKCurrentThreshold);
						WriteFlash(CurrentPWM_FLASH_DATA_ADDRESS,CurrentPWM);
					}	
		}	
		KeyTime = 0; //清楚按键标记
		scan_key();
		while(TIM_GetCounter(MainTIMER)<PWM1_HIGH);//一组累加完成，等待
}

/*CI_Mode_SelfLearning*/

void CI_Mode_SelfLearning(void)
{
	uint8_t SelfLearn_PWMx_y_zTotalCounter = 0;
	uint8_t SelfLearn_PWMx_y_z_Counter = 0;
	while(SelfLearn_PWMx_y_zTotalCounter<3)
	{
		CI_PWM_OUT(); 
		SX[SelfLearn_PWMx_y_zTotalCounter] = selfADCValue[SelfLearn_PWMx_y_z_Counter++];
		SY[SelfLearn_PWMx_y_zTotalCounter] = selfADCValue[SelfLearn_PWMx_y_z_Counter++];
		SZ[SelfLearn_PWMx_y_zTotalCounter] = selfADCValue[SelfLearn_PWMx_y_z_Counter++];
		SelfLearn_PWMx_y_zTotalCounter++;

		while(TIM_GetCounter(MainTIMER)<PWM1_HIGH){}//一组累加完成，等待
	}
		CI_PWM_OUT(); 
		SX[SelfLearn_PWMx_y_zTotalCounter] = selfADCValue[SelfLearn_PWMx_y_z_Counter++];
		SY[SelfLearn_PWMx_y_zTotalCounter] = selfADCValue[SelfLearn_PWMx_y_z_Counter++];
		SZ[SelfLearn_PWMx_y_zTotalCounter] = selfADCValue[SelfLearn_PWMx_y_z_Counter++];
		SelfLearn_PWMx_y_zTotalCounter++;
		while(TIM_GetCounter(MainTIMER)<PWM1_HIGH) {}  //一组累加完成，等待
		/*4组，12个数据完成*/
			ADCIndex = 0;
			SelfLearn_PWMx_y_zTotalCounter = 0;
			SelfLearn_PWMx_y_z_Counter = 0;
			
			SXA_B[KeyIndex] = (SX[0]+SX[1]+SX[2]+SX[3])/4; //累加求平均
			SYA_B[KeyIndex] = (SY[0]+SY[1]+SY[2]+SY[3])/4;
			SZA_B[KeyIndex] = (SZ[0]+SZ[1]+SZ[2]+SZ[3])/4;
	 /*等待获取四组ADC成功*/
		
		SA_B[KeyIndex]=SXA_B[KeyIndex]+SYA_B[KeyIndex]+SZA_B[KeyIndex];/*求得SA*/
	
		CXA_B[KeyIndex] = 1024*SXA_B[KeyIndex]/SA_B[KeyIndex];   //1->SXA,2->SXB
		CYA_B[KeyIndex] = 1024*SYA_B[KeyIndex]/SA_B[KeyIndex];
		CZA_B[KeyIndex] = 1024*SZA_B[KeyIndex]/SA_B[KeyIndex];
		
		KeyIndex++;  //记录第几次按键   1->SXA,2->SXB
		
		/****************************CXA,CYA,CZA要保存FLash********************************/
		
			if(KeyIndex>=2) //第二次按键   //
			{
				KeyIndex = 0;
				
				if(CXA_B[1]>CXA_B[0])
					NXSET =	CXA_B[1] - CXA_B[0];  /*NXSET=CXA-CXB的绝对值*/
				else
					NXSET =	CXA_B[0] - CXA_B[1];
				
				if(CYA_B[1]>CYA_B[0])
					NYSET = CYA_B[1] - CYA_B[0];
				else
					NYSET = CYA_B[0] - CYA_B[1];
				
				if(CZA_B[1]>CZA_B[0])
					NZSET = CZA_B[1] - CZA_B[0];
				else
					NZSET = CZA_B[0] - CZA_B[1];
				
				if(SA_B[1]>SA_B[0])
					NS_SET = SA_B[1] - SA_B[0];
				else
					NS_SET = SA_B[0] - SA_B[1];
				
				NXYZ_SET = (NXSET+NYSET+NZSET)*5/4;  //2018-1-26
				
				CICurrentThreshold = 1000 - (NS_SET + NXYZ_SET)/2-40;
				
				if(CICurrentThreshold<=200)
						CICurrentThreshold = 200;
				else if(CICurrentThreshold>=1000)
					CICurrentThreshold = 1000;
			
				WriteFlash(SA_FLASH_DATA_ADDRESS,SA_B[0]);
				WriteFlash(CXA_FLASH_DATA_ADDRESS,CXA_B[0]);
				WriteFlash(CYA_FLASH_DATA_ADDRESS,CYA_B[0]);
				WriteFlash(CZA_FLASH_DATA_ADDRESS,CZA_B[0]);
				WriteFlash(CICurrentThreshold_FLASH_DATA_ADDRESS,CICurrentThreshold);
								
			
			}
			KeyTime = 0; //清楚按键标记
			scan_key();
		while(TIM_GetCounter(MainTIMER)<PWM1_HIGH);//一组累加完成，等待
}

/***************************************
*
*扫描按键时间
*
**************************************/
void scan_key(void) 
{ 
	if(SETPin==Bit_SET )
	{
			key_counter++;
	}
	
	else	if (key_counter>middleKEY) 
		{ 
				KeyTime = key_counter; 
				key_counter = 0;
		}
	 else if(key_counter<middleKEY && key_counter>shortKEY)
			{ 
					KeyTime = key_counter; 
					key_counter = 0;
			}
	else	if(key_counter<shortKEY&&key_counter>2)
		{ 
				KeyTime = key_counter;  
				key_counter = 0;
		}
			
}

//void WriteFlash(uint32_t addr,uint32_t data)
//{
//FLASH_Unlock(); //解锁FLASH编程擦除控制器
//FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPERR);//清除标志位
//FLASH_ErasePage(FLASH_START_ADDR); //擦除指定地址页
//FLASH_ProgramWord(FLASH_START_ADDR+(addr*4),data); //从指定页的0地址开始写
//FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPERR);//清除标志位
//FLASH_Lock(); //锁定FLASH编程擦除控制器
//}

//FLASH读取数据测试
/*****************************
*
*初始化所有参数
*
****************************/
void ResetParameter(void)
{
		MAKCurrentThreshold=500;
		CurrentPWM = PWMX;
	
		CICurrentThreshold=500;
		SA_B[0] = 0x00;
		CXA_B[0] = 0x00;
		CYA_B[0] = 0x00;
		CZA_B[0] = 0x00;
		
		WriteFlash(MAKCurrentThreshold_FLASH_DATA_ADDRESS,MAKCurrentThreshold);

		WriteFlash(CurrentPWM_FLASH_DATA_ADDRESS,CurrentPWM);

		WriteFlash(CICurrentThreshold_FLASH_DATA_ADDRESS,CICurrentThreshold);

		WriteFlash(SA_FLASH_DATA_ADDRESS,SA_B[0]);

		WriteFlash(CXA_FLASH_DATA_ADDRESS,CXA_B[0]);

		WriteFlash(CYA_FLASH_DATA_ADDRESS,CYA_B[0]);

		WriteFlash(CZA_FLASH_DATA_ADDRESS,CZA_B[0]);
}

void GetEEPROM(void)
{
			MAKCurrentThreshold 	= ReadFlash(MAKCurrentThreshold_FLASH_DATA_ADDRESS);
			CurrentPWM 						= ReadFlash(CurrentPWM_FLASH_DATA_ADDRESS);
			CICurrentThreshold 		= ReadFlash(CICurrentThreshold_FLASH_DATA_ADDRESS);
			SA_B[0] 							= ReadFlash(SA_FLASH_DATA_ADDRESS);
			CXA_B[0] 							= ReadFlash(CXA_FLASH_DATA_ADDRESS);
			CYA_B[0] 							= ReadFlash(CYA_FLASH_DATA_ADDRESS);
			CZA_B[0] 							= ReadFlash(CZA_FLASH_DATA_ADDRESS);

}

/*******************************
*
*短路保护
*
*******************************/
void ShortCircuitProtection(void)
{
//	uint8_t SCState;
//	
//	/*读取SC引脚的状态*/
//	if(ShortCircuit!=1)
//	{
//		SCState = GPIO_ReadInputDataBit(SC_GPIO_Port ,SC_Pin);
//		if(SCState == Bit_RESET)
//		{
//			/*拉低FB_SC*/
//			ShortCircuit= 1;
//		}
//		else
//		{
//			ShortCircuit = 0;
//			ConfirmShortCircuit = 0;
//		}
//	}
//	if(ShortCircuit && ShortCircuitCounter>=5)
//	{
//		ConfirmShortCircuit=1;
//		
//		GPIO_WriteBit(OUT_GPIO_Port, OUT_Pin, Bit_RESET);/*马上拉低OUT*/
//		ShortCircuitTimer = ShortCircuitLastTime;
//	}
}

///**** Copyright (C)2017 HarryZeng. All Rights Reserved **** END OF FILE ****/
