

//notes: to convert to cpp:
//-change name
//-configuration -> compile "use CPP"
//-configurations-> link ->"don't use the standard system startup files"


#define HSE_VALUE ((uint32_t)8000000) /* STM32 discovery uses a 8Mhz external crystal */



#include "main.h"



#ifdef USE_VCP
/*
 * The USB data must be 4 byte aligned if DMA is enabled. This macro handles
 * the alignment, if necessary (it's actually magic, but don't tell anyone).
 */
__ALIGN_BEGIN USB_OTG_CORE_HANDLE  USB_OTG_dev __ALIGN_END;

#endif


/*
 * Define prototypes for interrupt handlers here. The conditional "extern"
 * ensures the weak declarations from startup_stm32f4xx.c are overridden.
 */
#ifdef __cplusplus
 extern "C" {
#endif
 void init();
 void ColorfulRingOfDeath(void);
// void encoderInitTest();
// void encoderReadTest();
 //void encoderOutputTest();


//#include "EncoderTimer.h"



void SysTick_Handler(void);
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
#ifdef USE_VCP
void OTG_FS_IRQHandler(void);
void OTG_FS_WKUP_IRQHandler(void);
#endif

#ifdef __cplusplus
}
#endif

int main(void)
{
	/* Set up the system clocks */
	SystemInit();

	/* Initialize USB, IO, SysTick, and all those other things you do in the morning */
	init();
	initDiscoveryBoard();

	/*
	//init machine control
	MachineControl machineControl;
	machineControlPointer = &machineControl;
	machineControl.getCharFunctionPointer = &VCP_get_char;
	/**/

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	//GPIO_InitTypeDef GPIO_initStructre; defined in .h file, has to be available because we work with two buttons on one pin...
	//Analog pin configuration
	GPIO_InitTypeDef GPIO_initStructre;
	GPIO_initStructre.GPIO_Pin = GPIO_Pin_12 ;
	GPIO_initStructre.GPIO_Mode = GPIO_Mode_OUT ;
	GPIO_initStructre.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_initStructre.GPIO_OType = GPIO_OType_PP;
	GPIO_initStructre.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOD,&GPIO_initStructre);//Affecting the port with the initialization structure configuration


	GPIO_ResetBits(GPIOD, GPIO_Pin_12);


	radio.init(1,9600);



	//init_usart1(9600);
	//init_USART1_bis();











	FlagStatus flagTest;

	//flagTest = SET;


	/*
				RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
				GPIO_InitTypeDef GPIO_InitDef;
				GPIO_InitDef.GPIO_Pin = GPIO_Pin_0;
				GPIO_InitDef.GPIO_Mode = GPIO_Mode_IN ;

				GPIO_InitDef.GPIO_OType = GPIO_OType_PP;
				GPIO_InitDef.GPIO_PuPd = GPIO_PuPd_DOWN;
				GPIO_InitDef.GPIO_Speed = GPIO_Speed_100MHz;
				GPIO_Init(GPIOA ,&GPIO_InitDef);


				if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)){
					printf("button pressed\r\n " );
				}else{
					//printf("button not pressed\r\n " );

				}
			/**/

	bool isInit = false;

	millisMemory_outputToSerial = millis;

	//refresh machine control loop
	while(1){


		//serial connection VCP (USB)
		uint8_t theByte;
		if (VCP_get_char(&theByte)){

			if ( theByte == '\r' || theByte == '\n' ){
				//these are the stop characters
				if (serialBufferOverflow){
					serialBufferOverflow = false;
					serialBufferPosition =0;
					serialBuffer[serialBufferPosition]='\0';
				}else if (serialBufferPosition>0){
					printf("buffer: %s\r\n", serialBuffer);
					serialBufferPosition =0;
					serialBuffer[serialBufferPosition]='\0';


					flagTest = USART_GetFlagStatus(USART1,USART_FLAG_RXNE);
					printf("usartflag: %d",flagTest);
				}

			}else if (serialBufferPosition >= SERIAL_BUFFER_SIZE){
				//check overflow
				if (!serialBufferOverflow){
					printf("serial buffer overflow, max chars: %d. Buffer will be purged until newline character received. \r\n", SERIAL_BUFFER_SIZE);

				}
				serialBufferOverflow = true;
				serialBufferPosition = 0;
			}else if (!serialBufferOverflow){
				//record char in buffer
				serialBuffer[serialBufferPosition] = theByte;
				serialBufferPosition ++;
				serialBuffer[serialBufferPosition]='\0';
			}

		}









	}
}


void init_usart1(uint32_t baud){
	if (!baud) {
		baud = 9600;
		printf("Warning! No baud set for usart1! (setting to 9600bps\n");
	}

	//our structs used for initialization
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;


	// Enable clock for GPIOA
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	//Enable clock for usart 1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);


	//Tell pins PB6 and PB7 which alternating function you will use
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1); //tx
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1); //rx

	// Initialize gpio pins with alternating function
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;   // = TX | RX
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);


	//initialize usart
	USART_InitStruct.USART_BaudRate = baud; // default is 9600 baud
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;  //8 bit size
	USART_InitStruct.USART_StopBits = USART_StopBits_1;  //1 stop bit
	USART_InitStruct.USART_Parity = USART_Parity_No; //no parity
	USART_InitStruct.USART_HardwareFlowControl =USART_HardwareFlowControl_None;  //no flow control
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;  //enable two way communication
	USART_Init(USART1, &USART_InitStruct);


	//(Nested Vector Interrupt controller)
	//config receive interrupt so that we can get messages as soon as they come in
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); //enable usart1 receive interrupt
	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn; //attach usart1's irq
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; //interrupt priority group (high)
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0; //sub priority within group (high)
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);

	//initializes a circular array for storing incoming serial data
	//initCircArray(&msg,200);


	//Lastly, enable usart1
	USART_Cmd(USART1, ENABLE);


}



void init_USART1_bis(){


		USART_InitTypeDef USART_InitStruct;
		//UART
		GPIO_InitTypeDef     GPIO_InitStruct;
		// Enable clock for GPIOA
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		/**
		* Tell pins PA2 and PA3 which alternating function you will use
		* @important Make sure, these lines are before pins configuration!
		*/
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
		GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_USART1);
		// Initialize pins as alternating function
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOB, &GPIO_InitStruct);




		/**
		 * Enable clock for USART2 peripheral
		 */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

		/**
		 * Set Baudrate to value you pass to function
		 * Disable Hardware Flow control
		 * Set Mode To TX and RX, so USART will work in full-duplex mode
		 * Disable parity bit
		 * Set 1 stop bit
		 * Set Data bits to 8
		 *
		 * Initialize USART1
		 * Activate USART1
		 */


		//USART_InitStruct.USART_BaudRate = baudrate;
		USART_InitStruct.USART_BaudRate = 9600;
		USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
		USART_InitStruct.USART_Parity = USART_Parity_No;
		USART_InitStruct.USART_StopBits = USART_StopBits_1;
		USART_InitStruct.USART_WordLength = USART_WordLength_8b;
		USART_Init(USART1, &USART_InitStruct);

		USART_Cmd(USART1, ENABLE);



		/**
		 * Set Channel to USART1
		 * Set Channel Cmd to enable. That will enable USART1 channel in NVIC
		 * Set Both priorities to 0. This means high priority
		 *
		 * Initialize NVIC
		 */
		NVIC_InitTypeDef NVIC_InitStruct;

		NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
		NVIC_Init(&NVIC_InitStruct);



		/**
			 * Enable RX interrupt
			 */
		USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);


}


 void initUSART3(){


		USART_InitTypeDef USART_InitStruct;
		//UART
		GPIO_InitTypeDef     GPIO_InitStruct;
		// Enable clock for GPIOA
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
		/**
		* Tell pins PA2 and PA3 which alternating function you will use
		* @important Make sure, these lines are before pins configuration!
		*/
		GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_USART3);
		GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_USART3);
		// Initialize pins as alternating function
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOD, &GPIO_InitStruct);




		/**
		 * Enable clock for USART2 peripheral
		 */
		RCC_APB2PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

		/**
		 * Set Baudrate to value you pass to function
		 * Disable Hardware Flow control
		 * Set Mode To TX and RX, so USART will work in full-duplex mode
		 * Disable parity bit
		 * Set 1 stop bit
		 * Set Data bits to 8
		 *
		 * Initialize USART1
		 * Activate USART1
		 */


		//USART_InitStruct.USART_BaudRate = baudrate;
		USART_InitStruct.USART_BaudRate = 9600;
		USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
		USART_InitStruct.USART_Parity = USART_Parity_No;
		USART_InitStruct.USART_StopBits = USART_StopBits_1;
		USART_InitStruct.USART_WordLength = USART_WordLength_8b;
		USART_Init(USART3, &USART_InitStruct);
		USART_Cmd(USART3, ENABLE);



		/**
		 * Set Channel to USART1
		 * Set Channel Cmd to enable. That will enable USART1 channel in NVIC
		 * Set Both priorities to 0. This means high priority
		 *
		 * Initialize NVIC
		 */
		NVIC_InitTypeDef NVIC_InitStruct;

		NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
		NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
		NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
		NVIC_Init(&NVIC_InitStruct);



		/**
			 * Enable RX interrupt
			 */
			USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);


 }


void initDiscoveryBoard(){
	//init the leds on the discoveryboard
	STM_EVAL_LEDInit(LED5);
	STM_EVAL_LEDOn(LED5);

	STM_EVAL_LEDInit(LED3);

	STM_EVAL_LEDInit(LED4);


	STM_EVAL_LEDInit(LED6);
	STM_EVAL_LEDOff(LED6);

}
#ifdef __cplusplus
 extern "C" {
#endif

void init()
{
	/* STM32F4 discovery LEDs */
	//GPIO_InitTypeDef LED_Config;

	/* Always remember to turn on the peripheral clock...  If not, you may be up till 3am debugging... */
	//RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	//LED_Config.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13| GPIO_Pin_14| GPIO_Pin_15;
	//LED_Config.GPIO_Mode = GPIO_Mode_OUT;
	//LED_Config.GPIO_OType = GPIO_OType_PP;
	//LED_Config.GPIO_Speed = GPIO_Speed_25MHz;
	//LED_Config.GPIO_PuPd = GPIO_PuPd_NOPULL;
	//GPIO_Init(GPIOD, &LED_Config);

	//dac test
	//DAC_GPIO_Config();
	//DAC_Config();
/*
	// GPIOD Periph clock enable
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	//GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_Init(GPIOA, &GPIO_InitStructure);

*/
	//Setup SysTick or CROD!
	if (SysTick_Config(SystemCoreClock / 1000))
	{
		ColorfulRingOfDeath();
}

#ifdef USE_VCP
	// Setup USB
	USBD_Init(&USB_OTG_dev,
		USB_OTG_FS_CORE_ID,
		&USR_desc,
		&USBD_CDC_cb,
		&USR_cb);
#endif
	return;

}


/*
 * Call this to indicate a failure.
 */

void ColorfulRingOfDeath(void){
	STM_EVAL_LEDOn(LED6);
	printf("Board generated an internal error. Please restart.");
	//while (1)
	//{
	//	//get stuck here forever.

	//}

}

/*
 * Interrupt Handlers
 */

void SysTick_Handler(void)
{
	ticker++;
	if (downTicker > 0)
	{
		downTicker--;
	}

	ticker20ms++;
	millis++;

}

void NMI_Handler(void)       {}
void HardFault_Handler(void) { ColorfulRingOfDeath(); }
void MemManage_Handler(void) { ColorfulRingOfDeath(); }
void BusFault_Handler(void)  { ColorfulRingOfDeath(); }
void UsageFault_Handler(void){ ColorfulRingOfDeath(); }
void SVC_Handler(void)       {}
void DebugMon_Handler(void)  {}
void PendSV_Handler(void)    {}

#ifdef USE_VCP
void OTG_FS_IRQHandler(void)
{
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
}

void OTG_FS_WKUP_IRQHandler(void)
{
  if(USB_OTG_dev.cfg.low_power)
  {
    *(uint32_t *)(0xE000ED10) &= 0xFFFFFFF9 ;
    SystemInit();
    USB_OTG_UngateClock(&USB_OTG_dev);
  }
  EXTI_ClearITPendingBit(EXTI_Line18);
}

#endif


void USART1_IRQHandler()
{
	GPIO_SetBits(GPIOD, GPIO_Pin_12);

	if (USART_GetITStatus(USART1, USART_IT_RXNE))
			{
				USART_ClearITPendingBit(USART3, USART_IT_RXNE);
			  //get newest incoming char/byte from data register and put in buffer
				char c = USART1->DR;
				printf(" %c",c);
				//buf_putbyte(&msg,c);

			}

}

void USART3_IRQHandler(void)
{
	GPIO_SetBits(GPIOD, GPIO_Pin_12);
	if (USART_GetITStatus(USART3, USART_IT_RXNE)) {
			//Do your stuff here
			printf ("success!");
			//Clear interrupt flag
			USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		}
}


#ifdef __cplusplus
 }
#endif




