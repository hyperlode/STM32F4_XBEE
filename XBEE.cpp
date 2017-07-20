
#include "XBEE.h"


const uint8_t atTest[] = {0x08, 0x01, 0x53, 0x48};

XBEE::XBEE(){
	receiveBufferCounter = 0;
	for (int i=0;i<NUMBER_OF_RECEIVEBUFFERS;i++){
		this->receiveFrameBuffersToBeProcessed[i]= NOTHING_TO_BE_PROCESSED;
		this->receiveFrameBuffersIsLocked[i] = false;
	}
}

void XBEE::init(uint8_t UART_Number, uint32_t baud){
	if (UART_Number ==1){
		//interrupt handler.
		
		/*
		 *
		 // paste this in main.cpp 
		 //there are way more fancy ways to do this, but...let's keep it simple for now.
	 	 //XBEE UART receive interrupt handler, handle as c code.
		void USART1_IRQHandler()
		{
			if (USART_GetITStatus(USART1, USART_IT_RXNE)){
				char c = USART1->DR;
				pRadio->receiveInterruptHandler	( c );
				//printf(" %c",c);
				USART_ClearITPendingBit(USART1, USART_IT_RXNE);
			}
		}
		* 
		*/
		
		
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

	}else{
		printf("ASSERT ERROR invalid value. Please select or configure valid USART");
	}
}



//specific routines
/*
void XBEE::sendAtCommandAndLockXbeeUntilResponse(uint16_t atCommand){
	//list of all remote nodes
	sendLocalATCommand(atCommand);
	idOfAtCommandWaitingForResponse = sendFrameIdCounter;
	senderXbeeLockedWaitingForResponse = true;
}
*/





//--------------------------------------------------

bool XBEE::setLocalXbeeAddress(){
	//set time out time

	//get msb bytes.
	if (!sendLocalATCommand( AT_MAC_HEIGH_SH , true)){
		return false;
	}

	//get low byte --> send at command.
	while (sendingIsLocked()){
		refresh();
		//wait
	}

	if (!sendLocalATCommand( AT_MAC_LOW_SL , true)){
		return false;
	}

	while (sendingIsLocked()){
		refresh();
		//wait
	}

	printf("local xbee address set: ");
	for (uint8_t i=0 ; i<8 ; i++){
		printf ("%02x ", senderXbee.address[i]);
	}
	printf("\r\n");
	return true;
}

void XBEE::setDestinationAddress(){
	for (uint8_t i=0; i<8; i++){
		destinationXbee.address[i] =  0x00;
	}
	destinationXbee.isAddressSet = true;
}

//------------------

//send data to destination xbee

void XBEE::sendMessageToDestination(uint8_t* message, uint16_t messageLength, bool awaitResponse){
	//frame type: 0X10 transmit request
	frameData frameData;
	frameData.length = 14 + messageLength;

	frameData.data[0] = 0x10; //frame type = Transmit request
	frameData.data[1] = getNextIdForSendFrame(awaitResponse);

	//destination address
	for (uint8_t i=0;i<8;i++){
		frameData.data[2 + i] = destinationXbee.address[i];
	}
	frameData.data[10] = 0xFF; // 16 bit dest address.
	frameData.data[11] = 0xFE;

	frameData.data[12] = 0x00; //broadcast radius
	frameData.data[13] = 0x00; //options

	//copy in the message.
	for (uint16_t i = 0;i<messageLength; i++){
		frameData.data[14 + i] = message[i];
	}
	buildAndSendFrame(&frameData);
}

void XBEE::processTransmitStatus(){
	if (transmitResponse.status == 0x00){
		printf("transmit success (id:%d)\r\n",transmitResponse.id);
	}else{
		printf("transmit status(id:%d): %02x\r\n",transmitResponse.id,transmitResponse.status);
	}
	releaseSendLock();

}

//---------------
//AT Command mode
//void XBEE::sendLocalATCommand(uint16_t atCommand, uint8_t* payload ){
bool XBEE::sendLocalATCommand(uint16_t atCommand, bool awaitResponse){
	// frame type: 0x08 AT Command
	//test://get address.
	printf("---sending AT command (please wait)---\r\n");
	frameData frameData;
	atFrameData atData;

	atData.atCommand = atCommand;
	//atData.data [AT_DATA_SIZE];
	atData.dataLength = 0;

	atData.id = getNextIdForSendFrame(awaitResponse);

	displayAtFrameData(&atData	);

	atFrameDataToFrameData(&atData,&frameData);

	bool success = buildAndSendFrame(&frameData);

	//displayFrame(&frameToSend);
	printf("---AT command sent---\r\n");
	return success;
}

void XBEE::atFrameDataToFrameData(atFrameData* atData, frameData* frameData){
	//translate atFrame to framedata of an API package.

	frameData->length = 4;
	frameData->data[0] = XBEE_FRAME_TYPE_AT_COMMAND; //frame type = AT Command
	frameData->data[1] = atData->id;
	frameData->data[2] = atData->atCommand>>8;
	frameData->data[3] = atData->atCommand & 0x00FF;

}

void XBEE::displayAtFrameData(atFrameData* atFrame){
	printf("display at command frame: \r\n");
	printf("type: AT command\r\n");
	printf("id: %d\r\n",atFrame->id);
	printf("command: %c%c\r\n", atFrame->atCommand>>8, atFrame->atCommand& 0x00FF );

}

void XBEE::displayAtCommandResponseFrameData(atCommandResponseFrameData* atFrame){
	printf("display at command response frame: \r\n");

	printf("type: AT command response\r\n");
	printf("id: %d\r\n",atFrame->id);
	printf("command: %c%c\r\n", atFrame->atCommand>>8, atFrame->atCommand& 0x00FF );
	printf("status: %d\r\n",atFrame->status);
	printf("data: ");
	for (uint8_t i = 0; i< atFrame->responseDataLength;i++){
		printf("%02x ",atFrame->responseData[i]);
	}
	printf("\r\n");

}
/*
bool XBEE::atCommandSuccessfullyExecuted(){
	//read and reset.
	if (this->atCommandSuccessfullyExecuted){
		this->atCommandSuccessfullyExecuted = false;
		return true;
	}
	return false;
}
*/
void XBEE::processAtResponse(){
	printf("at response status processed \r\n");
	//printf("-->response to neighbourfinding (id:%d)",idOfFrameWaitingForResponse);

	switch (atResponse.atCommand){
		case AT_DISCOVER_NODES_ND:
			printf("found the list.");

			//atCommandSuccessfullyExecuted =true;
			break;

		case AT_MAC_HEIGH_SH:
			for (uint8_t i =0;i<4;i++){
				senderXbee.address[i] = atResponse.responseData[i];
			}
			releaseSendLock();
			break;

		case AT_MAC_LOW_SL:
			for (uint8_t i =0;i<4;i++){
				senderXbee.address[i+4] = atResponse.responseData[i];
			}
			releaseSendLock();
			break;

		default:
			break;

	}
	/*
	uint8_t responseData [AT_DATA_SIZE];
	uint16_t responseDataLength = 0;
	uint8_t id = 0;
	uint8_t status = 0;
	*/

}
// ------------------------------------------------------------
//  XBEE administration
// ------------------------------------------------------------

void XBEE::refresh(){
	//check if package received.
	//process
	processReceivedFrame();
}

void XBEE::stats(){
	printf("receive package stats: \r\n");
	for (int i=0;i< NUMBER_OF_RECEIVEBUFFERS; i++){
		printf("buffer: %d, locked?:%d\r\n",i,this->receiveFrameBuffersIsLocked[i]);
	}

	printf("packages to be processed(-1 means 'free'):  \r\n");
	for (int i=0;i< NUMBER_OF_RECEIVEBUFFERS; i++){
		printf("slot: %d, buffer:%d\r\n",i,this->receiveFrameBuffersToBeProcessed[i]);
	}
}



// ------------------------------------------------------------
//XBEE frame functions
// ------------------------------------------------------------

bool XBEE::apiFrameIsValid(frameReceive* package){
	//test checksum of received frame
	//sum of payload + (last 8 bits of sum of frame bytes must be FF)
	//checksum on unescaped frame.
	//http://knowledge.digi.com/articles/Knowledge_Base_Article/Calculating-the-Checksum-of-an-API-Packet

	//printf("package length: %d", package->lengthFrameData);
	//printf("first byte: %02xbla\r\n", package->frame[0]);
	if ( calculateCheckSum((uint8_t*)package->frame, FRAME_FRAMEDATA_STARTINDEX, package->lengthFrameData) == package->frame[FRAME_FRAMEDATA_STARTINDEX+ package->lengthFrameData]){
		//printf("RECEIVE Info: Checksum correct\r\n");
		return true;
	}else{
		printf("RECEIVE ERROR: Checksum not correct\r\n");
		return false;
	}
}

uint8_t XBEE::calculateCheckSum(uint8_t* bytes, uint8_t startIndex, uint32_t length){
	//http://knowledge.digi.com/articles/Knowledge_Base_Article/Calculating-the-Checksum-of-an-API-Packet
	uint32_t sum = 0;
	for(uint16_t i=0;i<length;i++){
		sum = sum + bytes[i + startIndex];
		//checksum + last 8 bits must be FF.
	}

	sum = sum & 0x000000FF; //only last byte is considered.
	return 0xFF - sum; //
}

void XBEE::displayFrame(frame* frame){
	printf("display frame: ");
	for (uint8_t i = 0; i< frame->length;i++){
		printf("%02x ",frame->frame[i]);
	}
	printf("\r\n");
}


// ------------------------------------------------------------
//  XBEE send
// ------------------------------------------------------------

bool XBEE::buildAndSendFrame(frameData* frameData){

	//lock the xbee until message sent, or timeout

	if (!senderXbeeLockedWaitingForResponse  ){ //&& !sendingFrameIsBusy

		if (frameData->data[1]){ //if ID is not zero, wait for response
			//printf("will lock sending until response arrived.");
			senderXbeeLockedWaitingForResponse = true;
			idOfFrameWaitingForResponse = frameData->data[1];
		}

		frameToSend.frame[0] = 0x7E; //start delimiter
		frameToSend.frame[1] = frameData->length >>8; //frame data length msb
		frameToSend.frame[2] = frameData->length & 0xFF; //frame data length lsb //http://www.avrfreaks.net/forum/c-programming-how-split-int16-bits-2-char8bit

		uint16_t buildFrameIndex = 3; //add frame data
		for (uint8_t i=0; i<frameData->length; i++){
			uint8_t byteToSend = frameData->data[i];
			frameToSend.frame[buildFrameIndex] = frameData->data[i];
			buildFrameIndex++;
		}

		//add checksum
		frameToSend.frame[buildFrameIndex] = calculateCheckSum((uint8_t*)frameData->data, 0 ,frameData->length);
		frameToSend.length = frameData->length + 4;

		//escape frame
		frameToSend.lengthEscaped = frameToSend.length; //add bytes each time a character is escaped
		frameToSend.frameEscaped[0] = 0x7E;
		buildFrameIndex = 1;
		for (uint8_t i=1; i<frameToSend.length; i++){

			uint8_t byteToSend = frameToSend.frame[i];
			//printf("byte: %02x \r\n",byteToSend);

			if (byteToSend == 0x11|| byteToSend == 0x13 || byteToSend == 0x7E || byteToSend == 0x7D){
				frameToSend.lengthEscaped++;
				frameToSend.frameEscaped[buildFrameIndex] = 0x7D;
				buildFrameIndex++;
				frameToSend.frameEscaped[buildFrameIndex] = byteToSend ^ 0x20;
				/*
					if (frameData->data[i] == 0x11 ){

						frameToSend.frame[buildFrameIndex] = 0x31;
					} else if (frameData->data[i] == 0x13 ){
						frameToSend.frame[buildFrameIndex] = 0x33;

					} else if (frameData->data[i] == 0x7E ){
						frameToSend.frame[buildFrameIndex] = 0x5E;

					} else if (frameData->data[i] == 0x7D ){
						frameToSend.frame[buildFrameIndex] = 0x5D;

					}
					*/
			}else{
				frameToSend.frameEscaped[buildFrameIndex] = frameToSend.frame[i];
			}
			buildFrameIndex++;
		}


		printf("send frame (before escaping):");
		for (uint8_t i = 0; i< frameToSend.length;i++){
			printf("%02x ", frameToSend.frame[i]);
		}
		printf("\r\n");
				/*
		printf("\r\nframe built.\r\n");
			for (uint8_t i = 0; i< frameToSend.lengthEscaped;i++){
				printf("%02x ", frameToSend.frameEscaped[i]);
			}
		/**/
		sendFrame(&frameToSend);
		return true;
	}else{
		if (senderXbeeLockedWaitingForResponse){
			printf("error (not sent): busy waiting for response from previous message.");
		}else{
			printf("busy processing and sending previous frame.");
		}
		return false;
	}

}

uint8_t XBEE::getNextIdForSendFrame(bool awaitResponse){
	if (!awaitResponse){
		return 0;
	}
	sendFrameIdCounter++;
	if (sendFrameIdCounter == 0){
		sendFrameIdCounter++; //should never be zero. (zero is for messages that do not require a response)
	}
	return sendFrameIdCounter;
}

void XBEE::releaseSendLock(){
	senderXbeeLockedWaitingForResponse = false;
	idOfFrameWaitingForResponse = 0;
}

bool XBEE::sendingIsLocked(){
	return senderXbeeLockedWaitingForResponse;
}
/*
void XBEE::sendSendBuffer(){
	if (!senderXbeeLockedWaitingForResponse){
		sendFrame(&frameToSend);
		sendingFrameIsBusy = false;
	}
}
*/

void XBEE::sendFrame(frame* frame){

	for (uint32_t i=0; i< frameToSend.lengthEscaped; i++){
		sendByte(frame->frameEscaped[i]);
	}
}

void XBEE::sendTest(){

	uint32_t length = 26;
	//\x5A' is of type char, while 0x5A is of type int.
	//uint8_t test []= {0x7E,0x00,0x1D};
	uint8_t test []= {0x7E, 0x00, 0x14, 0x10, 0x01, 0x00, 0x7D, 0x33, 0xA2, 0x00, 0x41, 0x05, 0xBC, 0x87, 0xFF, 0xFE, 0x00, 0x00, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x7D, 0x5E};

	for (uint32_t i=0; i< length; i++){
		sendByte(test[i]);
	}
}

void XBEE::sendByte(uint8_t byteToSend){
	//test for tx.
	 while(!(USART1->SR & USART_SR_TXE));
	 USART1->DR = byteToSend;
}



// ------------------------------------------------------------
//  XBEE receive
// ------------------------------------------------------------

void XBEE::processReceivedFrame(){
	//check if first element in the list of packages to be processed is a buffer number that needs to be processed.
	if (!frameAvailableInFifoBuffer()){
		//nothing to process.
		//printf("nothing here");
		return;
	}

	frameReceive* fifoTopFrame = getTopFrameInReceivedFifoBuffer();

	//process top frame in fifo buffer
	displayTopFrameInReceivedFifoBuffer(fifoTopFrame);
	uint8_t frameType = parseFrame(fifoTopFrame);
	apiFrameIsValid(fifoTopFrame);

	//check type of frame.
	//getTypeOfFrame()

	//check if it is a requested response.
	if (frameType == XBEE_FRAME_TYPE_TRANSMIT_STATUS ){
		if (transmitResponse.id == idOfFrameWaitingForResponse && senderXbeeLockedWaitingForResponse){
			processTransmitStatus();
		}
	}else if( frameType == XBEE_FRAME_TYPE_AT_COMMAND_RESPONSE){
		if (atResponse.id == idOfFrameWaitingForResponse && senderXbeeLockedWaitingForResponse){
			processAtResponse();
			printf("at response");
		}
	}else{
		printf("frame is not a response %d, %d, %d \r\n", atResponse.id ,idOfFrameWaitingForResponse,senderXbeeLockedWaitingForResponse);
	}

	deleteTopFrameInReceivedFifoBuffer();

	printf("finished processing top frame.");

}


bool XBEE::frameAvailableInFifoBuffer(){
	int16_t bufferToProcess =  NOTHING_TO_BE_PROCESSED;
	if (this->receiveFrameBuffersToBeProcessed[0] != NOTHING_TO_BE_PROCESSED){
		return true;
	}else{
		return false;
	}
}

frameReceive* XBEE::getTopFrameInReceivedFifoBuffer(){
	if(frameAvailableInFifoBuffer()){
		int16_t bufferToProcess = this->receiveFrameBuffersToBeProcessed[0];
		return &this->receiveFrameBuffers[bufferToProcess];
	}else{
		printf ("assert ERROR: (check if buffer available before calling this function) nothing to process... \r\n");
		//return;
	}
}

void XBEE::deleteTopFrameInReceivedFifoBuffer(){
	if(!frameAvailableInFifoBuffer()){
		printf ("assert ERROR: (check if buffer available before calling this function) nothing to process... \r\n");
		//return;
	}

	int16_t bufferToProcess = this->receiveFrameBuffersToBeProcessed[0];
	this->receiveFrameBuffersIsLocked[bufferToProcess] =false; //release lock

	//delete package
	//move all elements one step to the front, make last slot empty.
	for (uint8_t i=0; i< NUMBER_OF_RECEIVEBUFFERS-1; i++){
		this->receiveFrameBuffersToBeProcessed[i] = this->receiveFrameBuffersToBeProcessed[i+1];
	}
	this->receiveFrameBuffersToBeProcessed[NUMBER_OF_RECEIVEBUFFERS-1] = NOTHING_TO_BE_PROCESSED;
}


uint8_t XBEE::parseFrame (frameReceive* frame){

	uint8_t frameType = frame->frame[FRAME_FRAMEDATA_STARTINDEX];

	//AT response parsing
	if (frameType == 0x88){
		printf("AT response frame.\r\n");
		atResponse.id = frame->frame[FRAME_FRAMEDATA_STARTINDEX + 1];
		atResponse.atCommand = frame->frame[FRAME_FRAMEDATA_STARTINDEX + 2] <<8 | frame->frame[FRAME_FRAMEDATA_STARTINDEX + 3];
		atResponse.status = frame->frame[FRAME_FRAMEDATA_STARTINDEX + 4];
		atResponse.responseDataLength = frame->lengthFrameData - AT_FRAME_DATA_STARTINDEX;
		for (uint16_t i = 0; i< atResponse.responseDataLength; i++){
			atResponse.responseData[i] = frame->frame[FRAME_FRAMEDATA_STARTINDEX + AT_FRAME_DATA_STARTINDEX + i];
		}
		displayAtCommandResponseFrameData(&atResponse);

	}else if (frameType == 0x8B){
		//transmit status
		transmitResponse.id =  frame->frame[FRAME_FRAMEDATA_STARTINDEX + 1];
		transmitResponse.status =  frame->frame[FRAME_FRAMEDATA_STARTINDEX + 5];
	}
	return frameType;
}

void XBEE::displayTopFrameInReceivedFifoBuffer(frameReceive* frame){
	//checksum
	if (apiFrameIsValid(frame)){
		//printf("RECEIVE Info: Checksum correct\r\n");

	}else{
		printf("RECEIVE ERROR: Checksum not correct\r\n");
	}

	//receiveFrameBuffers
	printf("raw frame (unescaped): ");
	for (uint8_t i = 0; i< frame->frameRecordIndex; i++)
	{
		printf("%02X ", frame->frame[i]);
	}

	printf("\r\n only frameData:");
	for (uint8_t i = 0; i< frame->lengthFrameData; i++)
	{
		printf("%02X ", frame->frame[i + FRAME_FRAMEDATA_STARTINDEX]);
	}
	printf("\r\n");





}

void XBEE::receiveFrame(char receivedByte){
	//This is done during the interrupt. keep it short! avoid printf!!

	//printf("+++ %02X ", receivedByte);
	//shorthand for the correct buffer to store the received byte


	frameReceive* buffer;
	buffer = &this->receiveFrameBuffers[this->receiveBufferCounter];
	//the shorthand: buffer->lengthFrameData   is equal to (*buffer).lengthFrameData; //https://stackoverflow.com/questions/22921998/error-request-for-member-size-in-a-which-is-of-pointer-type-but-i-didnt

	if( buffer->frameRecordIndex  >= RECEIVE_BUFFER_SIZE ){
		//buffer overflow
		printf("buffer overflowprogram buffer is %d, XBEE package length is up to 65535 + 3 bytes. --> Overflow, will reset buffer.", RECEIVE_BUFFER_SIZE);
		buffer->frameRecordIndex = 0;
		buffer->frameBusyReceiving = false;
		buffer->lengthFrameData = 0;
		buffer->length = 4;
		buffer->frame[0]='\0';
		unescapeNextReceivedByte = false;
	}

	if (buffer->frameRecordIndex > buffer->lengthFrameData + 4){
		//package length information conflict with number of received bytes
		printf("ASSERT error: error in communication, package is longer than indicated. Will reset buffer.\r\n");
		buffer->frameRecordIndex = 0;
		buffer->frameBusyReceiving = false;
		buffer->lengthFrameData = 0;
		buffer->length = 4;
		buffer->frame[0]='\0';
		unescapeNextReceivedByte = false;

	}else if (receivedByte == 0x7E && !unescapeNextReceivedByte){
		//packages are sent in API2 mode which means 0x7E is always start (
		//new package
		buffer->lengthFrameData = 0;
		buffer->length = 4;
		buffer->frameBusyReceiving = true;
		buffer->frameRecordIndex = 0;
		buffer->frame[0]='\0';
		unescapeNextReceivedByte = false;

		buffer->frame[buffer->frameRecordIndex] = receivedByte;
		buffer->frameRecordIndex ++;
		buffer->frame[buffer->frameRecordIndex]='\0';

		printf ("new package, store in buffer: %d\r\n", this->receiveBufferCounter);

	}else if (receivedByte == 0x7D){
		unescapeNextReceivedByte = true;

	}else if (buffer->frameBusyReceiving ){

		//API2 unescaping.
		if (unescapeNextReceivedByte){
			if (receivedByte == '\x31'){
				receivedByte = '\x11';
			}else if (receivedByte == '\x33'){
				receivedByte = '\x13';
			}else if (receivedByte == '\x5E'){
				receivedByte = '\x7E';
			}else if (receivedByte == '\x5D'){
				receivedByte = '\x7D';
			}else{
				printf("ASSERT ERROR: bad escape! char to be unescaped: %02X",receivedByte);
			}
			unescapeNextReceivedByte = false;
		}

		//record byte
		buffer->frame[buffer->frameRecordIndex] = receivedByte;
		buffer->frameRecordIndex ++;
		buffer->frame[buffer->frameRecordIndex]='\0';

		//when length information bytes are received, they are used to check when the package is received completely.
		if (buffer->frameRecordIndex == 3){
			buffer->lengthFrameData = buffer->frame[1]<<8 | buffer->frame[2]; //this->lengthFrameData = this->receiveBuffer[1]*256 + this->receiveBuffer[2]; //
			buffer->length = buffer->lengthFrameData + 4;
		}

		//end of package
		if (buffer->frameRecordIndex == buffer->length){
			printf("full package received \r\n");
			buffer->frameBusyReceiving = false;

			//add finished package to list of packages to be processed
			uint8_t i =0;
			while (this->receiveFrameBuffersToBeProcessed[i] != NOTHING_TO_BE_PROCESSED && i<NUMBER_OF_RECEIVEBUFFERS){
				i++;
			}

			if (i>= NUMBER_OF_RECEIVEBUFFERS){
				printf("unprocessed overflow error: all package process buffers are full. package will be neglected.");

			}else{
				this->receiveFrameBuffersToBeProcessed[i] =this->receiveBufferCounter;
				this->receiveFrameBuffersIsLocked[this->receiveBufferCounter] = true;

				//next buffer activate
				this->receiveBufferCounter++;

				if (this->receiveBufferCounter >= NUMBER_OF_RECEIVEBUFFERS){
					this->receiveBufferCounter = 0;
					//printf("%d\r\n" ,this->receiveBufferCounter );
				}

				//printf("%d\r\n" ,this->receiveFrameBuffersIsLocked[this->receiveBufferCounter] );
				//check if next buffer is free
				if (this->receiveFrameBuffersIsLocked[this->receiveBufferCounter] ){
					printf("unprocessed overflow WARNING: all package buffers full, wait with sending... \r\n "); //todo
				}
			}
		}
	}else {
		printf ("stray char received: %02X\r\n",receivedByte);
	}
}

void XBEE::receiveInterruptHandler(char c){
	this->receiveFrame(c);
}

