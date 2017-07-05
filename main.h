#ifndef MAIN_H
#define MAIN_H

//#define USE_VCP //in coocox in precompiler directives defined


#include "stm32f4xx_usart.h"
#include "XBEE.h"

#ifdef __cplusplus
	//#include "MachineControl.h"

	#include <stdio.h>
	extern "C"
	{
#endif
#include "stm32f4xx_adc.h"
#include "stm32f4xx_conf.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_dac.h"
#include "misc.h"
#ifdef USE_VCP
#include "usbd_cdc_core.h"
#include "usbd_usr.h"
#include "usbd_desc.h"
#include "usbd_cdc_vcp.h"
#include "usb_dcd_int.h"
#endif
#include "stm32f4_discovery.h"


	//volatile uint32_t ticker, downTicker;
	static uint32_t ticker, downTicker,ticker20ms ;
	static int ConvertedValue = 0; //Converted value readed from ADC

	static uint16_t temp = 0;
	static uint16_t vref = 0;
	static uint16_t adcSampleChannelCounter = 0;
	static uint16_t adcNumberOfSampleCycles= 0;

	//char* buffer;
	#define SERIAL_BUFFER_SIZE 10
	char serialBuffer[SERIAL_BUFFER_SIZE+1];
	uint8_t serialBufferPosition = 0;
	bool serialBufferOverflow = false;


	uint32_t  millisMemory_testing = 0;
	uint32_t  millisMemory_outputToSerial = 0;

	uint32_t millis;

	XBEE radio;




	void initDiscoveryBoard();
	//void initUSART3();
//	void init_usart1(uint32_t baud);
	//void init_USART1_bis();

#ifdef __cplusplus
	}
#endif




#endif //MAIN_H
