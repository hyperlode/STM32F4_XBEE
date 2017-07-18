#ifndef XBEE_H
#define XBEE_H
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "misc.h"
#include <stdio.h>

#define RECEIVE_BUFFER_SIZE 100
#define SEND_BUFFER_SIZE RECEIVE_BUFFER_SIZE
#define NUMBER_OF_RECEIVEBUFFERS 3
#define FRAME_PAYLOAD_STARTINDEX 3

//used for xbee communication. All tests done with XBEE PRO S3B in API2 mode.
#define API_MODE 2 //ASSUME API2 is used (escape functionality built in). No other mode available.
#define NOTHING_TO_BE_PROCESSED -1


//xbee frame types
#define XBEE_FRAME_TYPE_RECEIVE_PACKET 'x\90'




struct message{
	uint32_t address;
	uint16_t command;
	uint32_t id;
	uint8_t  arguments [64]; //64bytes payload
};

struct receivePackage{
	uint32_t packageRecordPosition = 0;
	bool packageRecording = false;
	char packageData[RECEIVE_BUFFER_SIZE+1];
	char payload[RECEIVE_BUFFER_SIZE+1];
	bool escapeNextChar = false;
	uint32_t packageLength = 0;
};

struct sendPackage{
	char payload[SEND_BUFFER_SIZE+1];
	char packageData[SEND_BUFFER_SIZE+1];
};



class XBEE{

public:
	XBEE();
	void init(uint8_t UART_Number, uint32_t baud);
	
	void receiveInterruptHandler(char c);
	
	void receiveBuffer_writeByte(char receivedByte);
	void receiveLocalPackage(char receivedByte);
	void readReceivedLocalPackage(receivePackage* package);
	bool apiFrameIsValid(receivePackage* package);
	void receiveBuffer_Readout_Flush();
	void sendPackage(char charToSend);
	void sendBuffer();
	void sendByte(uint8_t byteToSend);
	void processReceivedPackage();
	void unescapeAPIFrame(receivePackage* package);
	void refresh();
	void stats();
	bool byteIsAvailable();


private:
	uint32_t baud;

	uint8_t receiveBufferCounter =0;
	uint16_t receiveMessageCounter = 0;

	bool unescapeNextReceivedByte = false;

	bool packageReceiveBufferIsLocked [NUMBER_OF_RECEIVEBUFFERS];
	int16_t  packageReceiveBuffersToBeProcessed [NUMBER_OF_RECEIVEBUFFERS];
	receivePackage packageReceiveBuffer [NUMBER_OF_RECEIVEBUFFERS];
	
};

#endif
