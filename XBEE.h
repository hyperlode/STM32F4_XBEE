#ifndef XBEE_H
#define XBEE_H
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "misc.h"
#include <stdio.h>

#define RECEIVE_BUFFER_SIZE 100
#define NUMBER_OF_SENDBUFFERS 3
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

struct frameReceive{
	uint32_t packageRecordPosition = 0;
	bool packageRecording = false;
	char packageData[RECEIVE_BUFFER_SIZE+1];
	char payload[RECEIVE_BUFFER_SIZE+1];

	uint32_t packageLength = 0;
};

struct frame{
	char payload[SEND_BUFFER_SIZE+1];
	uint8_t frame[SEND_BUFFER_SIZE+1]; //packageData
	uint8_t frameEscaped[SEND_BUFFER_SIZE+1];
	uint16_t length = 0;
	uint16_t lengthEscaped = 0;
};


//http://docs.digi.com/display/RFKitsCommon/Frame+structure
struct frameData{
	uint16_t length = 0;
	uint8_t frameType;
	uint8_t data [RECEIVE_BUFFER_SIZE+1];
	uint8_t destinationAddress [8];

};



class XBEE{

public:
	XBEE();
	void init(uint8_t UART_Number, uint32_t baud);
	void refresh();
	void stats();


	
	void receiveFrame(char receivedByte);
	void readReceivedFrame(frameReceive* package);
	void receiveInterruptHandler(char c);
	void processReceivedFrame();

	
	uint8_t calculateCheckSum(uint8_t* bytes, uint8_t startIndex, uint32_t length);
	bool apiFrameIsValid(frameReceive* package);

	void sendPackage(char charToSend);
	void sendTest();
	void sendFrame(frame* frame);
	void sendByte(uint8_t byteToSend);

	void buildFrame(frameData* frameData);




private:
	uint32_t baud;

	uint8_t receiveBufferCounter =0;
	uint16_t receiveMessageCounter = 0;

	bool unescapeNextReceivedByte = false;

	bool receiveFrameBuffersIsLocked [NUMBER_OF_RECEIVEBUFFERS];
	int16_t receiveFrameBuffersToBeProcessed [NUMBER_OF_RECEIVEBUFFERS];
	frameReceive receiveFrameBuffers [NUMBER_OF_RECEIVEBUFFERS];
	frameReceive test;

	
	bool sendingFrameIsBusy = false;
	frame frameToSend;


};

#endif
