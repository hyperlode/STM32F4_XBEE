#ifndef XBEE_H
#define XBEE_H
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "misc.h"
#include <stdio.h>

#define RECEIVE_BUFFER_SIZE 100

#define API_MODE 2 //ASSUME API2 is used (escape functionality built in).

//used for xbee communication. All tests done with XBEE PRO S3B in API2 mode.



class XBEE{

public:
	XBEE();
	void init(uint8_t UART_Number, uint32_t baud);
	
	void receiveInterruptHandler(char c);
	
	void receiveBuffer_writeByte(char receivedByte);
	void receiveLocalPackage(char receivedByte);
	void readReceivedLocalPackage();
	void receiveBuffer_Readout_Flush();
	
	bool byteIsAvailable();


private:
	uint32_t baud;
	char receiveBuffer[RECEIVE_BUFFER_SIZE+1];
	uint32_t receiveBufferPosition = 0;
	bool receiveBufferOverflow = false;
	
	uint32_t packageRecordPosition = 0;
	bool packageRecording = false;
	char packageRecordBuffer[RECEIVE_BUFFER_SIZE+1];
	bool escapeNextChar = false;
	uint32_t packageLength = 0;
	
	
};

#endif
