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

#define BUFFER_NEIGHBOUR_XBEES_SIZE 10
#define XBEE_NAME_MAX_NUMBER_OF_CHARACTERS 16
#define FRAME_FRAMEDATA_STARTINDEX 3
#define AT_DATA_SIZE 100
#define AT_FRAME_DATA_STARTINDEX 5

//https://www.digi.com/resources/documentation/digidocs/90002173/Default.htm#containers/cont_at_cmd_addressing.htm%3FTocPath%3DAT%2520commands%7CAddressing%2520commands%7C_____0
#define AT_MAC_HEIGH_SH 0x5348
#define AT_MAC_LOW_SL 0x534C
#define AT_MAC_DESTINATION_HIGH_DH 0x4448
#define AT_MAC_DESTINATION_LOW_DL 0x444C
#define AT_DISCOVER_NODES_ND 0x4E44    //searches nodes in the entire network. great for discovering radios //https://www.digi.com/resources/documentation/digidocs/90002173/Default.htm#reference/r_cmd_nd.htm%3FTocPath%3DAT%2520commands%7CAddressing%2520discovery%252Fconfiguration%2520commands%7C_____3
#define AT_FIND_NEIGHBOURS_FN 0x464E    //same like ND but only one hop away! https://www.digi.com/resources/documentation/digidocs/90002173/Default.htm#reference/r_cmd_fn.htm%3FTocPath%3DAT%2520commands%7CAddressing%2520discovery%252Fconfiguration%2520commands%7C_____4
#define AT_AVAILABLE_FREQUENCIES_AF 0x4146

//used for xbee communication. All tests done with XBEE PRO S3B in API2 mode.
#define API_MODE 2 //ASSUME API2 is used (escape functionality built in). No other mode available.
#define NOTHING_TO_BE_PROCESSED -1
#define INVALID_AT_COMMAND 0x02


//xbee frame types
#define XBEE_FRAME_TYPE_RECEIVE_PACKET 0x90

#define XBEE_FRAME_TYPE_AT_COMMAND 0x08
#define XBEE_FRAME_TYPE_AT_COMMAND_RESPONSE 0x88
#define XBEE_FRAME_TYPE_TRANSMIT_REQUEST 0x10

#define XBEE_FRAME_TYPE_TRANSMIT_STATUS 0x8B

/*
struct message{
	uint32_t address;
	uint16_t command;
	uint32_t id;
	uint8_t  arguments [64]; //64bytes payload
};
*/


struct xbeeRadio{
	uint8_t address [8]; //64 bit
	bool isValid = false;
	uint8_t name [XBEE_NAME_MAX_NUMBER_OF_CHARACTERS];


};

//http://docs.digi.com/display/RFKitsCommon/Frame+structure
struct frameData{
	uint16_t length = 0;
	//uint8_t frameType;
	uint8_t data [RECEIVE_BUFFER_SIZE+1];
	//uint8_t destinationAddress [8];

};

struct transmitStatusFrame{
	uint8_t id = 0;
	uint8_t status = 0;
};

struct atCommandResponseFrameData{
	//same struct used for command and command response.
	uint16_t atCommand = INVALID_AT_COMMAND;
	uint8_t responseData [AT_DATA_SIZE];
	uint16_t responseDataLength = 0;
	uint8_t id = 0;
	uint8_t status = 0;
};


struct atFrameData{
	//same struct used for command and command response.
	//bool isResponse;
	uint16_t atCommand = INVALID_AT_COMMAND;
	uint8_t data [AT_DATA_SIZE];
	uint16_t dataLength = 0;
	uint8_t id = 0;
	//uint8_t status = 0;
};

struct frameReceive{
	//frameData data;
	uint32_t frameRecordIndex = 0;
	bool frameBusyReceiving = false;
	char frame[RECEIVE_BUFFER_SIZE+1];
	uint16_t lengthFrameData = 0; //
	uint16_t length = 4; //minumum four chars long.
};

struct frame{
	//frameData data;
	uint8_t frame[SEND_BUFFER_SIZE+1]; //packageData
	uint8_t frameEscaped[SEND_BUFFER_SIZE+1];
	uint16_t length = 0;
	uint16_t lengthEscaped = 0;
};



extern const uint8_t atTest[];


class XBEE{

public:
	XBEE();


	//administration
	void init(uint8_t UART_Number, uint32_t baud, uint32_t* millis);
	void refresh();
	void stats();
	void displayFrame(frame* frame);

	uint8_t calculateCheckSum(uint8_t* bytes, uint8_t startIndex, uint32_t length);
	bool apiFrameIsValid(frameReceive* package);





	bool setLocalXbeeAddress(uint32_t timeout_millis);
	bool searchActiveRemoteXbees(uint32_t timeout_millis);
	void displayNeighbours();

	void setDestinationAddress();
	uint8_t getNextIdForSendFrame(bool awaitResponse);


	//transmit request
	void processTransmitStatus();
	void sendMessageToDestination(uint8_t* message, uint16_t messageLength, bool awaitResponse);

	//AT
	bool sendLocalATCommand(uint16_t atCommand, bool awaitResponse);
	bool sendAtCommandAndAwaitWithResponse(uint16_t atCommand, uint32_t timeout_millis);
	void atFrameDataToFrameData(atFrameData* atData, frameData* frameData);
	void processAtResponse();
	void displayAtCommandResponseFrameData(atCommandResponseFrameData* atFrame);
	void displayAtFrameData(atFrameData* atFrame);
	
	//receive frame
	void receiveFrame(char receivedByte);
	void displayTopFrameInReceivedFifoBuffer(frameReceive* package);
	uint8_t parseFrame(frameReceive* package);

	void receiveInterruptHandler(char c);
	void processReceivedFrame();

	bool frameAvailableInFifoBuffer();
	void deleteTopFrameInReceivedFifoBuffer();
	frameReceive* getTopFrameInReceivedFifoBuffer();
	void clearReceiveBuffers();
	
	//send frame
	void sendPackage(char charToSend);
	//void sendSendBuffer();
	void sendTest();
	void sendFrame(frame* frame);
	void sendByte(uint8_t byteToSend);

	void releaseSendLock();
	bool buildAndSendFrame(frameData* frameData);
	bool sendingIsLocked();



private:
	xbeeRadio destinationXbee;
	xbeeRadio senderXbee;

	uint32_t baud;

	uint8_t receiveBufferCounter =0;
	uint16_t receiveMessageCounter = 0;

	bool unescapeNextReceivedByte = false;

	bool receiveFrameBuffersIsLocked [NUMBER_OF_RECEIVEBUFFERS];
	int16_t receiveFrameBuffersToBeProcessed [NUMBER_OF_RECEIVEBUFFERS];
	frameReceive receiveFrameBuffers [NUMBER_OF_RECEIVEBUFFERS];
	frameReceive test;

	
	xbeeRadio neighbours [BUFFER_NEIGHBOUR_XBEES_SIZE];

	uint8_t idOfNeighBoursFindCommand; //record the id of this command while waiting for response.

	bool sendingFrameIsBusy = false;
	bool senderXbeeLockedWaitingForResponse = false;
	uint8_t idOfFrameWaitingForResponse = 0;
	uint8_t sendFrameIdCounter =0;
	uint8_t numberOfNeighbours =0;
	frame frameToSend;

	atCommandResponseFrameData	atResponse;
	transmitStatusFrame transmitResponse;
	uint32_t* millis;

};

#endif
