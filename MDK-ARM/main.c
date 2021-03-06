#include  "stm32f0xx.h"
#include "stdio.h"
#include "process.h"
#include "flash.h"

#define DMA_BUFFER_SIZE     1  


//  if ((GPIOx->ODR & GPIO_Pin) != (uint32_t)Bit_RESET)
//  {
//    bitstatus = (uint8_t)Bit_SET;
//  }
//  else
//  {
//    bitstatus = (uint8_t)Bit_RESET;
//  }


//if (BitVal != Bit_RESET)
//  {
//    GPIOx->BSRR = GPIO_Pin;
//  }
//  else
//  {
//    GPIOx->BRR = GPIO_Pin ;
//  }
#define _Gpio_7_set  GPIO_WriteBit(GPIOA, GPIO_Pin_7, (BitAction)!GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_7))

uint8_t sample_finish = 0;  
int16_t adc_dma_tab[6] = { 0 };  
uint8_t sample_index = 0;  
uint8_t PWM_Step=0;

uint8_t PWM_Start_Flag = 0;
uint8_t PWM_Start_Counter = 0;
uint8_t PWM_TimeCounter = 0;

//?????  
int16_t sample_1[128] = { 0 };  
int16_t sample_2[128] = { 0 };  
int16_t sample_3[128] = { 0 };  
int16_t sample_4[128] = { 0 };  
int16_t sample_5[128] = { 0 };  
int16_t sample_6[128] = { 0 };  
  

RCC_ClocksTypeDef   SysClock;
extern void scan_key(void);
extern uint16_t scan_tick;
extern uint8_t EnterSelfFlag;
extern uint8_t 	ShortCircuitCounter;
extern uint8_t 	ShortCircuit;
extern uint8_t FB_Flag;
void 	adc_config(void) ;
void 	user_adc_init(void)  ;
void 	adc_gpio_init(void);
void 	adc_dma_init(void) ;
void 	USART_Config(void);
void 	GPIO_PWMOUT_Config(void);
void 	TIM1_PWM_Config(void);
void 	TIM1_Base_Config(void);
void	TIM2_PWM_Config(void);
void	TIM3_Config(void);
void TIM14_Config(void);

void GPIO_DEINIT_ALL(void);
/*****************************************/
///////////////////////////////////////////////////////////////////////////////////
/**
  * @brief  //加入以下代码,支持printf函数,而不需要选择use MicroLIB
  */
int fputc(int ch, FILE *f)
{
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
		;
	USART_SendData(USART1, (unsigned char) ch);
  return (ch);
}

void BSP_init(void)  
{  
    adc_gpio_init();  
    adc_config();               // ??????????,????????????????????  
    adc_dma_init();             //  
		//TIM1_PWM_Config();
		//TIM1_Base_Config();
		//TIM2_PWM_Config();
		TIM14_Config();
		TIM3_Config();
		GPIO_PWMOUT_Config();
		GPIO_DEINIT_ALL();
    //USART_Config();
}

void USART_Config(void)
{
        USART_InitTypeDef USART_InitStructure;
        /*???USART1??*/
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
        USART_InitStructure.USART_BaudRate = 115200;//???????
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//?????
        USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;//??????
        USART_InitStructure.USART_Parity = USART_Parity_No;//?????
        USART_InitStructure.USART_StopBits = USART_StopBits_1;//?????
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;//?????
        USART_Init(USART1, &USART_InitStructure);
        USART_Cmd(USART1, ENABLE);//???? 1
}
/*GPIOA????? */
void GPIO_PWMOUT_Config(void)
{
        GPIO_InitTypeDef GPIO_InitStructure;
        /*???GPIOA??*/
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
        /* ??PA9 ,PA10*/
        GPIO_InitStructure.GPIO_Pin = PWMX_Pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; //NORMAL
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;
        GPIO_Init(PWMX_GPIO_Port, &GPIO_InitStructure);
	
				GPIO_InitStructure.GPIO_Pin = PWMY_Pin;
				GPIO_Init(PWMY_GPIO_Port, &GPIO_InitStructure);
	
				GPIO_InitStructure.GPIO_Pin = PWMZ_Pin;
				GPIO_Init(PWMZ_GPIO_Port, &GPIO_InitStructure);
	
				GPIO_InitStructure.GPIO_Pin = PWM1_Pin;
				GPIO_Init(PWM1_GPIO_Port, &GPIO_InitStructure);
}
/*?? 1 ????*/
void UART_send_byte(uint8_t byte)
{
        while(!((USART1->ISR)&(1<<7)));//?????
        USART1->TDR=byte; //??????
}
/*?????*/
void UART_Send(uint8_t *Buffer, uint32_t Length)
{
        while(Length != 0)
        {
                while(!((USART1->ISR)&(1<<7)));//?????
                USART1->TDR= *Buffer;
                Buffer++;
                Length--;
        }
}

uint8_t UART_Recive(void)
{
        while(!(USART1->ISR & (1<<5)));
        return(USART1->RDR);
}
  
void adc_config()  
{  
    ADC_InitTypeDef adc_init_structure;  

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);            //??ADC?? 
    
		ADC_DeInit(ADC1);                                               //??ADC  
    ADC_StructInit(&adc_init_structure);                            //???ADC???  
  
    adc_init_structure.ADC_ContinuousConvMode = DISABLE;            //????????  
    adc_init_structure.ADC_DataAlign = ADC_DataAlign_Right;         //???????  
    //adc_init_structure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T1_TRGO; //???????TIM1
    //adc_init_structure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_RisingFalling;//?????  
    adc_init_structure.ADC_Resolution = ADC_Resolution_12b;         //12????  
    adc_init_structure.ADC_ScanDirection = ADC_ScanDirection_Upward;//????0-18??  
		
    ADC_Init(ADC1, &adc_init_structure);  
		
    ADC_OverrunModeCmd(ADC1, ENABLE);                               //????????  
    ADC_ChannelConfig(ADC1, ADC_Channel_0, ADC_SampleTime_7_5Cycles);               //??????,????125nS  
    ADC_GetCalibrationFactor(ADC1);                                 //?????ADC  
    ADC_Cmd(ADC1, ENABLE);                                          //??ADC1  
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_ADEN) == RESET);         //??ADC1????  
  
    ADC_DMACmd(ADC1, ENABLE);                                       //??ADC_DMA  
    ADC_DMARequestModeConfig(ADC1, ADC_DMAMode_Circular);           //??DMA?????????  
    //ADC_StartOfConversion(ADC1);                                    //??????(??)  
}  
  
void adc_gpio_init()  
{
    GPIO_InitTypeDef gpio_init_structure;  
    //??GPIO??  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA , ENABLE);  
  
    GPIO_StructInit(&gpio_init_structure);  
    //GPIOA                                                         //PA-0~3??ADC  
    gpio_init_structure.GPIO_Pin = (GPIO_Pin_0);  
    gpio_init_structure.GPIO_Mode = GPIO_Mode_AN;                   //????(??)??  
    gpio_init_structure.GPIO_OType = GPIO_OType_PP;                 //????  
    gpio_init_structure.GPIO_Speed = GPIO_Speed_2MHz;              //Fast speed  
    gpio_init_structure.GPIO_PuPd= GPIO_PuPd_UP;                    //??  
    GPIO_Init(GPIOA, &gpio_init_structure);  
}  
  
void adc_dma_init()  
{  
    DMA_InitTypeDef dma_init_structure;  
    NVIC_InitTypeDef nvic_init_structure;  
  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);              //??DMA??  
  
    nvic_init_structure.NVIC_IRQChannel = DMA1_Channel1_IRQn;       //??DMA1????  
    nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;                //????  
    nvic_init_structure.NVIC_IRQChannelPriority = 0;                //?????0  
    NVIC_Init(&nvic_init_structure);  
  
    DMA_DeInit(DMA1_Channel1);                                      //复位DMA1_channel1  
    DMA_StructInit(&dma_init_structure);                            //初始化DMA结构体 
  
    dma_init_structure.DMA_BufferSize = DMA_BUFFER_SIZE;            //DMA缓存数组大小  
    dma_init_structure.DMA_DIR = DMA_DIR_PeripheralSRC;             //DMA方向:外设作为数据源 
    dma_init_structure.DMA_M2M = DISABLE;                           //内存到内存禁用
    dma_init_structure.DMA_MemoryBaseAddr = (uint32_t)&adc_dma_tab[0];//缓存数据数组起始地址
    dma_init_structure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//数据大小设置为Halfword  
    dma_init_structure.DMA_MemoryInc = DMA_MemoryInc_Enable;        //内存地址递增 
    dma_init_structure.DMA_Mode = DMA_Mode_Circular;                //DMA循环模式,完成后重新开始覆盖 
    dma_init_structure.DMA_PeripheralBaseAddr = (uint32_t) &(ADC1->DR);//取值的外设地址 
    dma_init_structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//外设取值大小设置为Halfword  
    dma_init_structure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址递增禁用
    dma_init_structure.DMA_Priority = DMA_Priority_High;             //DMA优先级设置为高  
    DMA_Init(DMA1_Channel1, &dma_init_structure);  
  
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);                  //使能DMA中断
    DMA_ClearITPendingBit(DMA_IT_TC);                                //清除一次DMA中断标志  
    DMA_Cmd(DMA1_Channel1, ENABLE);                                  //使能DMA1  
}  
  
void TIM1_Base_Config(void)
{
	  TIM_TimeBaseInitTypeDef timer_init_structure;  
    NVIC_InitTypeDef nvic_init_structure;  

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 , ENABLE);	

    nvic_init_structure.NVIC_IRQChannel = TIM1_BRK_UP_TRG_COM_IRQn;                //使能TIM1中断通道  
    nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;                //使能TIM1中断  
    nvic_init_structure.NVIC_IRQChannelPriority = 8;                //优先级为0  
		
    NVIC_Init(&nvic_init_structure); 

		/* Time ??????*/
		timer_init_structure.TIM_Prescaler = 5;  //48/6=8 ,8M->0.125us
		timer_init_structure.TIM_CounterMode = TIM_CounterMode_Up;  /* Time ????????????*/
		timer_init_structure.TIM_Period = 640;   // 960->120us  
		timer_init_structure.TIM_RepetitionCounter = 0;

		TIM_TimeBaseInit(TIM1, &timer_init_structure);
		/* TIM1 ?????*/
		//TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);							//选择TIM1的timer为触发源  
		//TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_OC1Ref);							//选择TIM1的timer为触发源  
		//TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_OC2Ref);							//选择TIM1的timer为触发源  
		//TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_OC3Ref);							//选择TIM1的timer为触发源  
		//TIM_SelectSlaveMode(TIM1, TIM_SlaveMode_Gated);//触发模式只启动，门控制启停都可以控制
		TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);                      //使能TIM1中断
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);     //清除update事件中断标志

		TIM_Cmd(TIM1, ENABLE);

		/* TIM1 ????? */
}


void TIM1_PWM_Config(void)
{
	  TIM_TimeBaseInitTypeDef timer_init_structure;  
		TIM_OCInitTypeDef timer_OCinit_structure; 
    NVIC_InitTypeDef nvic_init_structure;  
  	GPIO_InitTypeDef gpio_init_structure;
	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 , ENABLE);	
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA , ENABLE);	
	
    gpio_init_structure.GPIO_Pin = PWMX_Pin;  
    gpio_init_structure.GPIO_Mode = GPIO_Mode_AF;                   //????(??)??  
    gpio_init_structure.GPIO_Speed = GPIO_Speed_2MHz;              //Fast speed  
    gpio_init_structure.GPIO_PuPd= GPIO_PuPd_NOPULL;                    //??  
    GPIO_Init(PWMX_GPIO_Port, &gpio_init_structure);
		gpio_init_structure.GPIO_Pin = PWMY_Pin;  
		GPIO_Init(PWMY_GPIO_Port, &gpio_init_structure);
		gpio_init_structure.GPIO_Pin = PWMZ_Pin;  
		GPIO_Init(PWMZ_GPIO_Port, &gpio_init_structure);
	
		GPIO_PinAFConfig(GPIOA,GPIO_PinSource8,GPIO_AF_2);
		GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_2);
		GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_2);
	
    nvic_init_structure.NVIC_IRQChannel = TIM1_BRK_UP_TRG_COM_IRQn;                //使能TIM1中断通道  
    nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;                //使能TIM1中断  
    nvic_init_structure.NVIC_IRQChannelPriority = 2;                //优先级为0  
		
    NVIC_Init(&nvic_init_structure); 


		/* TIM1 ??? ---------------------------------------------------
		 TIM1 ????(TIM1CLK) ??? APB2 ?? (PCLK2)    
			=> TIM1CLK = PCLK2 = SystemCoreClock
		 TIM1CLK = SystemCoreClock, Prescaler = 0, TIM1 counter clock = SystemCoreClock
		 SystemCoreClock ?48 MHz 
		 
		 ??????? 4 ?PWM ???17.57 KHz:
			 - TIM1_Period = (SystemCoreClock / 17570) - 1
		 ??1??????? 50%
		 ??2??????? 37.5%
		 ??3??????? 25%
		 ??4??????? 12.5%
		 ????????????:
			 - ChannelxPulse = DutyCycle * (TIM1_Period - 1) / 100
		*/  
			/*???????,???????????????*/
		//TimerPeriod = (SystemCoreClock / 17570 ) - 1;
		//TimerPeriod = (SystemCoreClock / DEF_PWMFRE ) - 1;
		//TimerPeriod = (SystemCoreClock / DEF_PWMFRE);
		/* TIM1 ???? */


		/* Time ??????*/
		timer_init_structure.TIM_Prescaler = 5;  //48/6=8 ,8M->0.125us
		timer_init_structure.TIM_CounterMode = TIM_CounterMode_Up;  /* Time ????????????*/
		timer_init_structure.TIM_Period = 960;   // 960->120us  
		timer_init_structure.TIM_RepetitionCounter = 0;

		TIM_TimeBaseInit(TIM1, &timer_init_structure);

		/* ??1,2,3,4?PWM ???? */
		timer_OCinit_structure.TIM_OCMode = TIM_OCMode_PWM1;
		timer_OCinit_structure.TIM_OutputState = TIM_OutputState_Enable ;//TIM_OutputState_Enable; //PWM?????
		timer_OCinit_structure.TIM_OutputNState = TIM_OutputNState_Disable ;//TIM_OutputNState_Enable; //??PWM?????
		timer_OCinit_structure.TIM_OCPolarity = TIM_OCPolarity_High;  //PWM 1?????
		timer_OCinit_structure.TIM_OCNPolarity = TIM_OCNPolarity_Low; //PWM?? 0?????
		timer_OCinit_structure.TIM_OCIdleState = TIM_OCIdleState_Set;
		timer_OCinit_structure.TIM_OCNIdleState = TIM_OCIdleState_Reset;

		timer_OCinit_structure.TIM_Pulse = PWMx_HIGH; //?????
		TIM_OC1Init(TIM1, &timer_OCinit_structure);//????1??
		TIM_OC1PreloadConfig(TIM1,TIM_OCPreload_Disable);

		timer_OCinit_structure.TIM_Pulse = PWMy_HIGH; //?????
		TIM_OC2Init(TIM1, &timer_OCinit_structure);//????1??
		TIM_OC2PreloadConfig(TIM1,TIM_OCPreload_Disable);

		timer_OCinit_structure.TIM_Pulse = PWMz_HIGH; //?????
		TIM_OC3Init(TIM1, &timer_OCinit_structure);//????1??
		TIM_OC3PreloadConfig(TIM1,TIM_OCPreload_Disable);

//		TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);                      //使能TIM1中断
//		TIM_ITConfig(TIM1, TIM_IT_CC1, ENABLE);                      //使能TIM1中断
//		TIM_ITConfig(TIM1, TIM_IT_CC2, ENABLE);                      //使能TIM1中断
//		TIM_ITConfig(TIM1, TIM_IT_CC3, ENABLE);                      //使能TIM1中断

		//TIM_ARRPreloadConfig(TIM1,DISABLE);

		/* TIM1 ?????*/
		//TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_Update);							//选择TIM1的timer为触发源  
		//TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_OC1Ref);							//选择TIM1的timer为触发源  
		//TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_OC2Ref);							//选择TIM1的timer为触发源  
		//TIM_SelectOutputTrigger(TIM1, TIM_TRGOSource_OC3Ref);							//选择TIM1的timer为触发源  
		//TIM_SelectSlaveMode(TIM1, TIM_SlaveMode_Gated);//触发模式只启动，门控制启停都可以控制
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);     //清除update事件中断标志
		TIM_ClearITPendingBit(TIM1, TIM_IT_CC1);     //清除update事件中断标志
		TIM_ClearITPendingBit(TIM1, TIM_IT_CC2);     //清除update事件中断标志
		TIM_ClearITPendingBit(TIM1, TIM_IT_CC3);     //清除update事件中断标志
		//TIM_SelectMasterSlaveMode(TIM1, TIM_MasterSlaveMode_Enable);//主从模式MSM  
		TIM_CtrlPWMOutputs(TIM1, ENABLE);
		//TIM_SelectOnePulseMode(TIM1,TIM_OPMode_Single);

		TIM_Cmd(TIM1, ENABLE);

		/* TIM1 ????? */
	
}

void TIM2_PWM_Config(void)
{
    TIM_TimeBaseInitTypeDef timer_init_structure;  
		TIM_OCInitTypeDef timer_OCinit_structure; 
    NVIC_InitTypeDef nvic_init_structure;  
  	GPIO_InitTypeDef gpio_init_structure;	

		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA , ENABLE);	
	
		nvic_init_structure.NVIC_IRQChannel = TIM2_IRQn;                //使能TIM2中断通道  
    nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;                //使能TIM2中断  
    nvic_init_structure.NVIC_IRQChannelPriority = 6;                //优先级为0  
		
    NVIC_Init(&nvic_init_structure); 
		
    gpio_init_structure.GPIO_Pin = PWM1_Pin;  
    gpio_init_structure.GPIO_Mode = GPIO_Mode_AF;                   //????(??)??  
    gpio_init_structure.GPIO_Speed = GPIO_Speed_2MHz;              //Fast speed  
    gpio_init_structure.GPIO_PuPd= GPIO_PuPd_NOPULL;                    //??  
    GPIO_Init(PWM1_GPIO_Port, &gpio_init_structure);

		GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_2);
		/*TIM2*/
    TIM_DeInit(TIM2);                                               //复位TIM2  
    TIM_TimeBaseStructInit(&timer_init_structure);                  //初始化TIM结构体  
  
    timer_init_structure.TIM_CounterMode = TIM_CounterMode_Up;      //向上计数模式  
    timer_init_structure.TIM_Period = 960;                          //每43 uS触发一次中断,??ADC  
    timer_init_structure.TIM_Prescaler = 5;                      //计数时钟分频,f=1M,systick=1 uS  
    timer_init_structure.TIM_RepetitionCounter = 0;              //发生0+1的update事件产生中断 
		
    TIM_TimeBaseInit(TIM2, &timer_init_structure);  
  
		timer_OCinit_structure.TIM_OCMode = TIM_OCMode_PWM1;
		timer_OCinit_structure.TIM_OutputState = TIM_OutputState_Enable;
		timer_OCinit_structure.TIM_Pulse = PWM1_HIGH;
		timer_OCinit_structure.TIM_OCPolarity = TIM_OCPolarity_High;

		TIM_OC3Init(TIM2,&timer_OCinit_structure);
		TIM_OC3PreloadConfig(TIM2,TIM_OCPreload_Enable);
		TIM_ARRPreloadConfig(TIM2,ENABLE);
		
    //TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);                      //使能TIM2中断
		//TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update);           //选择TIM1的update为触发源  
		//TIM_SelectInputTrigger(TIM2, TIM_TS_ITR0);
		//TIM_SelectSlaveMode(TIM2, TIM_SlaveMode_Gated);//触发模式只启动，门控制启停都可以控制
		//TIM_SelectMasterSlaveMode(TIM2, TIM_MasterSlaveMode_Enable);//主从模式MSM

		TIM_Cmd(TIM2, ENABLE);
}

void TIM3_Config(void)
{
    TIM_TimeBaseInitTypeDef timer_init_structure;  
    NVIC_InitTypeDef nvic_init_structure;  
	
	  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);            //??TIM2??  //??GPIO??  
	
		nvic_init_structure.NVIC_IRQChannel = TIM3_IRQn;                //使能TIM3中断通道  
    nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;                //使能TIM3中断  
    nvic_init_structure.NVIC_IRQChannelPriority = 4;                //优先级为4
		
    NVIC_Init(&nvic_init_structure); 

		/*TIM3*/
		TIM_DeInit(TIM3);                                               //复位TIM3
    TIM_TimeBaseStructInit(&timer_init_structure);                  //初始化TIM结构体  
  
    timer_init_structure.TIM_ClockDivision = TIM_CKD_DIV1;          //系统时钟,不分频,24M  
    timer_init_structure.TIM_CounterMode = TIM_CounterMode_Up;      //向上计数模式  
    timer_init_structure.TIM_Period = 1000;                          //每1000 uS触发一次中断,??ADC  
    timer_init_structure.TIM_Prescaler = 47;                      //计数时钟分频,f=1M,systick=1 uS  
    timer_init_structure.TIM_RepetitionCounter = 0x00;              //发生0+1的update事件产生中断 
		
    TIM_TimeBaseInit(TIM3, &timer_init_structure);  
		//TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);                      //使能TIM3中断
    TIM_Cmd(TIM3, ENABLE);                                          //使能TIM3
	
}
    
/****************************??????****************************/  

void TIM2_IRQHandler()  
{  
    if(TIM_GetITStatus(TIM2, TIM_FLAG_Update))            //判断发生update事件中断  
    {
				//_Gpio_7_set;
        TIM_ClearFlag(TIM2, TIM_FLAG_Update);     //清除update事件中断标志
    }  
}  
int16_t captureX=0,captureY=0,captureZ=0;
void TIM1_BRK_UP_TRG_COM_IRQHandler()  
{  
    if(TIM_GetITStatus(TIM1, TIM_FLAG_Update)==SET)            //判断发生update事件中断  
    {
			//_Gpio_7_set;
      TIM_ClearFlag(TIM1, TIM_FLAG_Update);     //清除update事件中断标志
    }
		if(TIM_GetITStatus(TIM1, TIM_IT_CC1) != RESET)
		{
			TIM_ClearFlag(TIM1, TIM_IT_CC1 ); //清除update事件中断标志		
		}
		if(TIM_GetITStatus(TIM1, TIM_IT_CC2) != RESET)
		{
			TIM_ClearFlag(TIM1, TIM_FLAG_CC2);     //清除update事件中断标志
		}
		if(TIM_GetITStatus(TIM1, TIM_IT_CC3) != RESET)
		{
			TIM_ClearFlag(TIM1, TIM_FLAG_CC3);     //清除update事件中断标志
		}
} 



extern uint32_t   ShortCircuitLastTime;
void TIM3_IRQHandler()
{
	  if(TIM_GetITStatus(TIM3, TIM_IT_Update))            //判断发生update事件中断  
    { 
//			scan_tick++;
//			_Gpio_7_set;
//			if(scan_tick>=10)  /*10*1.22=12.2ms*/
//			{
//				scan_tick = 0;
//				scan_key();
//			}
			TIM_ClearITPendingBit(TIM3, TIM_IT_Update);     //清除update事件中断标志
		}
}


int16_t  RunTime = 0; 

extern uint8_t SelfGetADCWell;
extern uint8_t FB_Flag;
uint8_t ADCIndex=0;
uint8_t DMAIndex=0;
int16_t selfADCValue[12];
uint8_t ADC_Conversion_Flag=0;
void DMA1_Channel1_IRQHandler()  
{  
    if(DMA_GetITStatus(DMA_IT_TC))                      //判断DMA传输完成中断  
    {   
			selfADCValue[ADCIndex++] = 4095 - adc_dma_tab[0];
			ADC_Conversion_Flag=1;
//			if(ADCIndex>3)   //X,Y,Z三组，四个，==12个    /*需要采集完4组才计算？？？？，要480us*/
//			{
//				ADCIndex = 0;
////				if(EnterSelfFlag&&(DMAIndex==0))//自学习模式
////				{
////						DMAIndex=1;
////						SelfGetADCWell=1;
////				}
////				else													//正常工作模式
////				{
//					sample_finish = 1;
////					//RunTime = TIM_GetCounter(TIM14);
////				}
//			}
			DMA_ClearITPendingBit(DMA_IT_TC);                   //清楚DMA中断标志位  
    }  
}  

void TIM14_IRQHandler()
{
	  if(TIM_GetITStatus(TIM14, TIM_IT_Update))            //判断发生update事件中断  
    { 
			scan_tick++;
			_Gpio_7_set;
			if(scan_tick>=10)  /*10*1.22=12.2ms*/
			{
				scan_tick = 0;
				scan_key();
			}
			TIM_ClearITPendingBit(TIM14, TIM_IT_Update);     //清除update事件中断标志
		}
}
/***********************/
void TIM14_Config(void)
{
    TIM_TimeBaseInitTypeDef timer_init_structure;  
		NVIC_InitTypeDef nvic_init_structure;  	
	
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
		/*TIM14*/
		TIM_DeInit(TIM14);                                               //复位TIM14

		nvic_init_structure.NVIC_IRQChannel = TIM14_IRQn;                //使能TIM14中断通道  
    nvic_init_structure.NVIC_IRQChannelCmd = ENABLE;                //使能TIM14中断  
    nvic_init_structure.NVIC_IRQChannelPriority = 1;                //优先级为4
    NVIC_Init(&nvic_init_structure); 	

    TIM_TimeBaseStructInit(&timer_init_structure);                  //初始化TIM结构体  
  
    timer_init_structure.TIM_ClockDivision = TIM_CKD_DIV1;          //系统时钟,不分频,24M  
    timer_init_structure.TIM_CounterMode = TIM_CounterMode_Up;      //向上计数模式  
    timer_init_structure.TIM_Period = 1000;                          //每300 uS触发一次中断,??ADC  
    timer_init_structure.TIM_Prescaler = 47;                      //计数时钟分频,f=1M,systick=1 uS  
    timer_init_structure.TIM_RepetitionCounter = 0x00;              //发生0+1的update事件产生中断 
		
    TIM_TimeBaseInit(TIM14, &timer_init_structure);  
		//TIM_ITConfig(TIM14, TIM_IT_Update, ENABLE);                      //使能TIM14中断
    TIM_Cmd(TIM14, ENABLE);                                          //使能TIM14
}


void GPIO_INIT(void)
{
	
	    GPIO_InitTypeDef gpio_init_structure;  
    //??GPIO??  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB|RCC_AHBPeriph_GPIOF, ENABLE);  
  
    GPIO_StructInit(&gpio_init_structure);  
                                                        //PA-0~3??ADC  
    gpio_init_structure.GPIO_Pin = OUT_Pin;  
    gpio_init_structure.GPIO_Mode = GPIO_Mode_OUT;                   //????(??)??  
    gpio_init_structure.GPIO_OType = GPIO_OType_PP;                 //????  
    gpio_init_structure.GPIO_Speed = GPIO_Speed_2MHz;              //Fast speed  
    gpio_init_structure.GPIO_PuPd= GPIO_PuPd_NOPULL;                    //??  
    GPIO_Init(OUT_GPIO_Port, &gpio_init_structure);  
	
		gpio_init_structure.GPIO_Pin = FB_Pin;
		gpio_init_structure.GPIO_Mode = GPIO_Mode_IN;                   //????(??)??  
    gpio_init_structure.GPIO_Speed = GPIO_Speed_2MHz;              //Fast speed  
		gpio_init_structure.GPIO_PuPd= GPIO_PuPd_DOWN;                    //??
    GPIO_Init(FB_GPIO_Port, &gpio_init_structure);  
	
		gpio_init_structure.GPIO_Pin = GOODBAD_Pin;
		gpio_init_structure.GPIO_Mode = GPIO_Mode_OUT;                   //????(??)??  
    gpio_init_structure.GPIO_Speed = GPIO_Speed_2MHz;              //Fast speed  
		gpio_init_structure.GPIO_PuPd= GPIO_PuPd_DOWN;                    //??
    GPIO_Init(GOODBAD_GPIO_Port, &gpio_init_structure);  
	
		gpio_init_structure.GPIO_Pin = SET_Pin;
		gpio_init_structure.GPIO_Mode = GPIO_Mode_IN;                   //????(??)??  
    gpio_init_structure.GPIO_Speed = GPIO_Speed_2MHz;              //Fast speed  
		gpio_init_structure.GPIO_PuPd= GPIO_PuPd_DOWN;                    //??
    GPIO_Init(SET_GPIO_Port, &gpio_init_structure);  

}

void GPIO_DEINIT_ALL(void)
{
	
	   GPIO_InitTypeDef gpio_init_structure;  
    //??GPIO??  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB|RCC_AHBPeriph_GPIOF, ENABLE);  
  
    GPIO_StructInit(&gpio_init_structure);  
                                                        //PA-0~3??ADC  
    gpio_init_structure.GPIO_Pin = GPIO_Pin_7;  
    gpio_init_structure.GPIO_Mode = GPIO_Mode_OUT;                   //????(??)??  
    gpio_init_structure.GPIO_OType = GPIO_OType_PP;                 //????  
    gpio_init_structure.GPIO_Speed = GPIO_Speed_2MHz;              //Fast speed  
    gpio_init_structure.GPIO_PuPd= GPIO_PuPd_DOWN;                    //??  
    GPIO_Init(GPIOA, &gpio_init_structure); 
	
}


void RCC_Configuration(void)
{
	RCC_PLLConfig(RCC_PLLSource_HSI,RCC_PLLMul_12);
	RCC_PLLCmd(ENABLE);
	RCC_ADCCLKConfig(RCC_ADCCLK_PCLK_Div4);
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY)==RESET)
	{
		
	}
	
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	
	while(RCC_GetSYSCLKSource()!=0x08)
	{
	}
}

void DelaymsSet(int16_t ms)
{
		while(1)
		{
			ms--;
			if(ms<=0)
				break;
		}
}

uint8_t CheckFLag=0;

/*************************************************************/
#define 	FLASH_START_ADDR1 	 						0x08007000    //0x0800FFF0

uint8_t* UID=(uint8_t*)0x1FFFF7AC;  //获取UID  stm32f0:0x1FFFF7AC,stm32f100:0x1FFFF7E8
uint32_t Fml_Constant 	= 0x19101943;//输入到公式的常熟
uint8_t *C= (uint8_t*)&Fml_Constant;//把常量转化成数组
uint8_t FormulaResult[4];
uint32_t FormulaCheck;
uint32_t UIDFlashResult;
uint16_t Fml_CRC16;
uint8_t D[12];
int ProgramCounter=0;

void Formula_100(uint8_t *D,uint8_t *Result)
{
	D[0] = UID[4];
	D[1] = UID[8];
	D[2] = UID[1];
	D[3] = UID[3];
	D[4] = UID[0];
	D[5] = UID[5];
	D[6] = UID[10];
	D[7] = UID[7];
	D[8] = UID[9];
	D[9] = UID[2];
	D[10] = UID[11];
	D[11] = UID[6];
	
	Result[0] = C[0] ^ D[0];
	Result[1] = C[1] ^ D[6] ^ D[7] ^ D[8] ^ D[9] ^ D[10] ^ D[11] ;
	Result[2] = C[2] ^ D[4] ;
	Result[3] = C[3] ^ D[2] ^ D[1];
}

uint8_t FlashCheck(void)
{
		uint8_t FlashFlag;
		Formula_100(D,FormulaResult);
		FormulaCheck = FormulaResult[0]+(FormulaResult[1]<<8)+(FormulaResult[2]<<16)+(FormulaResult[3]<<24);
		//UIDFlashResult = *(__IO uint32_t*)(FLASH_START_ADDR1);
		UIDFlashResult = *(uint32_t*)(FLASH_START_ADDR1);
		if(UIDFlashResult==FormulaCheck)
			FlashFlag =1;
		else
			FlashFlag =0;
		
		return FlashFlag;
}

uint16_t Formula_CRC16(uint8_t *p,uint8_t len)
{
	uint8_t i;
	while(len--)
	{
		for(i=0x80; i!=0; i>>=1)
		{
			if((Fml_CRC16 & 0x8000) != 0)
			{
				Fml_CRC16 <<= 1;
				Fml_CRC16 ^= 0x1021;
			}
			else
			{
				Fml_CRC16 <<= 1;
			}
			if((*p&i)!=0)
			{
				Fml_CRC16 ^= 0x1021;
			}
		}
		p++;
	}
	return Fml_CRC16;
}
void ProgramCheck(void)
{
	ProgramCounter 		= ReadFlash(ProgramRUNcounter_Mode_FLASH_DATA_ADDRESS);
	if(ProgramCounter>65535 || ProgramCounter<0)
	{
		ProgramCounter = 0;
		ResetParameter();
	}
	ProgramCounter = ProgramCounter+1;
	WriteFlash(ProgramRUNcounter_Mode_FLASH_DATA_ADDRESS,ProgramCounter);
	DelaymsSet(50); 	
	if(ProgramCounter<=1)
	{
		ResetParameter();
	}
}

void IWDG_Config(void)
{
	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
	{
		RCC_ClearFlag();
	}
	
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	IWDG_SetPrescaler(IWDG_Prescaler_64);
	IWDG_SetReload(40000/20);
	IWDG_ReloadCounter();
	
	IWDG_Enable();
}

/***************主函数***************/
int main(void)
{
		uint32_t checkcouter;
		RCC_Configuration();
		GPIO_INIT();
		BSP_init();
		RCC_GetClocksFreq(&SysClock);
		DelaymsSet(5000); 
		IWDG_Config();
		CheckFLag = FlashCheck();
	
		if(CheckFLag)
		//if(1)
		{
			/*程序运行次数检测*/
			ProgramCheck();
			/*主要运行函数*/
			DataProcess();
		}
		else
			while(1)
			{
				checkcouter++;
			}	
}

