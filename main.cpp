

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

	XBEE radio;
	pRadio = &radio;


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

		//refresh XPEE
		//radio.refresh();

		//handle commands over
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

					printf("command sent: %s\r\n", serialBuffer);

					//interprete command
					if (stringsAreEqual(serialBuffer, "lode")){
						printf("lode command!");
					}else if (stringsAreEqual(serialBuffer, "txtest")){
						radio.sendPackage();
					}else if (stringsAreEqual(serialBuffer, "xbee")){
						//radio.receiveBuffer_Readout_Flush();
						radio.stats();
					}else if (stringsAreEqual(serialBuffer, "process")){
						radio.processReceivedPackage();
					}else{
						printf("Invalid command received.\r\n"
								"available commands:\r\n"
								"\txbee: Shows xbee receive statistics\r\n"
								"\ttxtest: send data to xbee test\r\n"
								"\tlode: Displays nonsense...\r\n"
							"\tprocess: Process last received package \r\n");
					}

					serialBufferPosition =0;
					serialBuffer[serialBufferPosition]='\0';

/*
					flagTest = USART_GetFlagStatus(USART1,USART_FLAG_RXNE);
					printf("usartflag: %d",flagTest);
*/


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


bool stringsAreEqual(char* A, char*B){
	//check if two strings, char arrays terminated with a '/0' are equal

	/*

	bool test;
	char text [2] = {'b','\0'};
	char lode [2] = {'a','\0'};

	test = stringsAreEqual(text, lode);
	printf ("testresult: %d",test);
	*/

	uint32_t i = 0;
	while (A[i] == B[i]){
		if (A[i] == '\0'){
			return true;
		}
		i++;
	}
	return false;
}

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

//XBEE UART receive interrupt handler, handle as c code.
void USART1_IRQHandler()
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE)){
		pRadio->receiveInterruptHandler	( USART1->DR );
		//after handling, reenable interrupts
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}

#ifdef __cplusplus
 }
#endif




