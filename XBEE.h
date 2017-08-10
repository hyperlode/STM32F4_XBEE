#ifndef XBEE_H
#define XBEE_H
#include "stm32f4xx_usart.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_exti.h"
#include "misc.h"
#include <stdio.h>

#include "generalFunctions.h"

#define FIRMWARE_802.15.4_TH


//#define XBEE_PRINTF_DEBUG
//#define XBEE_PRINTF_INFO
#define XBEE_PRINTF_MINIMAL_INFO

#define RECEIVE_BUFFER_SIZE 100
#define NUMBER_OF_SENDBUFFERS 3
#define SEND_BUFFER_SIZE RECEIVE_BUFFER_SIZE

#define NUMBER_OF_RECEIVEBUFFERS 3

#define BUFFER_NEIGHBOUR_XBEES_SIZE 10
#define XBEE_NAME_MAX_NUMBER_OF_CHARACTERS 16
#define FRAME_FRAMEDATA_STARTINDEX 3
#define AT_DATA_SIZE 100
#define AT_FRAME_DATA_STARTINDEX 5

#define AT_COMMAND_TIMEOUT_GET_SETTING 100

//https://www.digi.com/resources/documentation/digidocs/90002173/Default.htm#containers/cont_at_cmd_addressing.htm%3FTocPath%3DAT%2520commands%7CAddressing%2520commands%7C_____0
#define AT_MAC_HEIGH_SH 0x5348
#define AT_MAC_LOW_SL 0x534C
#define AT_MAC_DESTINATION_HIGH_DH 0x4448
#define AT_MAC_DESTINATION_LOW_DL 0x444C
#define AT_DISCOVER_NODES_ND 0x4E44    //searches nodes in the entire network. great for discovering radios //https://www.digi.com/resources/documentation/digidocs/90002173/Default.htm#reference/r_cmd_nd.htm%3FTocPath%3DAT%2520commands%7CAddressing%2520discovery%252Fconfiguration%2520commands%7C_____3
#define AT_FIND_NEIGHBOURS_FN 0x464E    //(doesnt seem to work for S2C) same like ND but only one hop away! https://www.digi.com/resources/documentation/digidocs/90002173/Default.htm#reference/r_cmd_fn.htm%3FTocPath%3DAT%2520commands%7CAddressing%2520discovery%252Fconfiguration%2520commands%7C_____4
#define AT_AVAILABLE_FREQUENCIES_AF 0x4146
#define AT_APPLY_CHANGES_AC 0x4143
#define AT_WRITE_WR 0x5752
#define AT_NODE_IDENTIFIER_NI 0x4E49

//used for xbee communication. All tests done with XBEE PRO S3B in API2 mode.
#define API_MODE 2 //ASSUME API2 is used (escape functionality built in). No other mode available.
#define NOTHING_TO_BE_PROCESSED -1
#define INVALID_AT_COMMAND 0x02


//xbee frame types
#define XBEE_FRAME_TYPE_RECEIVE_PACKET 0x90
#define XBEE_FRAME_TYPE_RX_RECEIVE_PACKET 0x80


#define XBEE_FRAME_TYPE_AT_COMMAND 0x08
#define XBEE_FRAME_TYPE_AT_COMMAND_RESPONSE 0x88

#define XBEE_FRAME_TYPE_TX_TRANSMIT_REQUEST_64_BIT 0x00
#define XBEE_FRAME_TYPE_TX_TRANSMIT_STATUS 0x89

#define XBEE_FRAME_TYPE_TRANSMIT_REQUEST 0x10
#define XBEE_FRAME_TYPE_MODEM_STATUS 0x8A
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
	//receiving an at command response
	uint16_t atCommand = INVALID_AT_COMMAND;
	uint8_t responseData [AT_DATA_SIZE];
	uint16_t responseDataLength = 0;
	uint8_t id = 0;
	uint8_t status = 0;
};


struct atFrameData{
	//for sending an at command
	uint16_t atCommand = INVALID_AT_COMMAND;
	uint8_t data [AT_DATA_SIZE];
	uint16_t dataLength = 0;
	uint8_t id = 0;
};

struct rxFrameData{
	//https://www.digi.com/resources/documentation/digidocs/pdfs/90001500.pdf p121
	//receive rx data
	//uint16_t atCommand = INVALID_AT_COMMAND;
	uint8_t sourceAddress[8];
	uint8_t RSSI ;//received strength indicator
	uint8_t options; //0x01 = address broadcast (unicast) , //0x02 PAN broadcast (broadcast)

	uint8_t data [AT_DATA_SIZE];
	uint16_t dataLength = 0;
	//uint8_t id = 0;
	//uint8_t status = 0;
};

struct frameReceive{
	//complete api frame
	//frameData data;
	uint32_t frameRecordIndex = 0;
	bool frameBusyReceiving = false;
	char frame[RECEIVE_BUFFER_SIZE+1];
	uint16_t lengthFrameData = 0; //
	uint16_t length = 4; //minimum four chars long (because this is a complete frame)
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
	void reset();
	void stats();
	void displayFrame(frame* frame);


	//sleep
	void setSleepPinPB9();
	void sleep();
	void wakeUp();
	bool isSleeping();


	uint8_t calculateCheckSum(uint8_t* bytes, uint8_t startIndex, uint32_t length);
	bool apiFrameIsValid(frameReceive* package);

	void generateFrameType0x00(frameData* frameData,char* message, uint16_t messageLength, uint8_t id, uint8_t* destinationAddress);
	void generateFrameType0x10(frameData* frameData,char* message, uint16_t messageLength, uint8_t id, uint8_t* destinationAddress);

	bool sendMessageToDestination(char* message, uint16_t messageLength, bool awaitResponse, uint32_t timeout_millis);
	bool sendMessageBroadcast(char* message, uint16_t messageLength, bool awaitResponse, uint32_t timeout_millis);
	bool sendMessage(char* message, uint8_t* address, uint16_t messageLength, bool awaitResponse, uint32_t timeout_millis);

	bool getLocalXbeeAddress(uint32_t timeout_millis);
	bool getDestinationAddressFromXbee();
	bool setDestinationAddressInLocalXbee(uint8_t* address);
	bool saveChangesinLocalXbee();
	bool searchActiveRemoteXbees(uint32_t timeout_millis);
	int8_t getNeighbourIndexByName(char* name, bool getFirstOccurenceIfMultiple);
	void displayNeighbours();
	bool setNeighbourAsRemote(uint8_t numberInList);
	void clearNeighbours();
	uint8_t getNextIdForSendFrame(bool awaitResponse);

	//modem status
	void processModemStatus(frameReceive* frame);

	//transmit request
	void processTransmitStatus();
	void processTxTransmitStatus();

	//AT

	bool sendAtCommandAndAwaitWithResponse(uint16_t atCommand, uint8_t* parameter, uint8_t parameterLength, uint32_t timeout_millis);
	bool sendAtCommandAndAwaitWithResponse(uint16_t atCommand, uint32_t parameterInt, uint32_t timeout_millis);
	bool sendAtCommandAndAwaitWithResponse(uint16_t atCommand, uint32_t timeout_millis);

	void atFrameDataToFrameData(atFrameData* atData, frameData* frameData);
	void processAtResponse();
	void displayAtCommandResponseFrameData(atCommandResponseFrameData* atFrame);
	void displayAtFrameData(atFrameData* atFrame);
	
	//receive frame
	rxFrameData* getRxPackage();

	void receiveFrame(char receivedByte);
	void displayTopFrameInReceivedFifoBuffer(frameReceive* package);
	uint8_t parseFrame(frameReceive* package);

	void receiveInterruptHandler(char c);
	bool processReceivedFrame();

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
	bool awaitResponseToLastMessage(uint32_t  timeout_millis);
	bool buildAndSendFrame(frameData* frameData);
	bool sendingIsLocked();


	xbeeRadio destinationXbee;
	xbeeRadio senderXbee;

private:

	bool sendLocalATCommand(uint16_t atCommand, uint8_t* parameter, uint8_t parameterLength, bool awaitResponse); //only make time out version public


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

	rxFrameData rxData;
	atCommandResponseFrameData	atResponse;
	transmitStatusFrame transmitResponse;
	uint32_t* millis;

	bool isAsleep = 0;
	GPIO_TypeDef* sleepPinPort;
	uint16_t sleepPin;

};

#endif
