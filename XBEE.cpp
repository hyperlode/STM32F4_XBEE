
#include "XBEE.h"

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


void XBEE::readReceivedFrame(frameReceive* package){
	//receiveFrameBuffers

	for (uint8_t i = 0; i< package->packageRecordPosition; i++)
	{
		printf("%02X ", package->packageData[i]);
	}

	printf("\r\n");
	for (uint8_t i = 0; i< package->packageLength; i++)
	{
		printf("%02X ", package->packageData[i + FRAME_PAYLOAD_STARTINDEX]);
	}


	//this->receiveBufferOverflow = false;
	//this->packageRecordPosition = 0;
	//this->packageRecording = false;
}

bool XBEE::apiFrameIsValid(frameReceive* package){
	//test checksum
	//sum of payload + (last 8 bits of sum of frame bytes must be FF)
	//checksum on unescaped frame.
	//http://knowledge.digi.com/articles/Knowledge_Base_Article/Calculating-the-Checksum-of-an-API-Packet

	printf("package length: %d", package->packageLength);
	printf("first byte: %02xbla\r\n", package->packageData[0]);
	if ( calculateCheckSum((uint8_t*)package->packageData, FRAME_PAYLOAD_STARTINDEX, package->packageLength) == package->packageData[FRAME_PAYLOAD_STARTINDEX+ package->packageLength]){
		printf("RECEIVE ERROR: Checksum correct\r\n");
		return true;
	}else{
		printf("RECEIVE ERROR: Checksum not correct\r\n");
				return false;
	}
/*
	//printf(" is checksumSent + sum =  %01x + %01x ==FF??\r\n", package->packageData[FRAME_PAYLOAD_STARTINDEX+ package->packageLength],sum);
	if (sum + package->packageData[FRAME_PAYLOAD_STARTINDEX+ package->packageLength] & 0x000000FF == 0x000000FF){
		//printf("checksum correct\r\n");
		return true;
	}else{
		printf("RECEIVE ERROR: Checksum not correct\r\n");
		return false;
	}
*/
}

uint8_t XBEE::calculateCheckSum(uint8_t* bytes, uint8_t startIndex, uint32_t length){
	//
	uint32_t sum = 0;
	for(uint16_t i=0;i<length;i++){
		sum = sum + bytes[i + startIndex];
		//checksum + last 8 bits must be FF.
	}

	sum = sum & 0x000000FF; //only last byte is considered.
	return 0xFF - sum;
}

/*


void XBEE::sendLocalPackage(){
}
*/

void XBEE::processReceivedFrame(){
	//check if first element in the list of packages to be processed is a buffer number that needs to be processed.
	int16_t bufferToProcess =  NOTHING_TO_BE_PROCESSED;
	if (this->receiveFrameBuffersToBeProcessed[0] != NOTHING_TO_BE_PROCESSED){
		bufferToProcess = this->receiveFrameBuffersToBeProcessed[0];
	}else{
		printf ("nothing to process... \r\n");
		return;
	}

	//process package

	readReceivedFrame(&this->receiveFrameBuffers[bufferToProcess]);
	apiFrameIsValid(&this->receiveFrameBuffers[bufferToProcess]);

	//delete package
	//move all elements one step to the front, make last slot empty.
	for (uint8_t i=0; i< NUMBER_OF_RECEIVEBUFFERS-1; i++){
		this->receiveFrameBuffersToBeProcessed[i] = this->receiveFrameBuffersToBeProcessed[i+1];
	}
	this->receiveFrameBuffersToBeProcessed[NUMBER_OF_RECEIVEBUFFERS-1] = NOTHING_TO_BE_PROCESSED;

	this->receiveFrameBuffersIsLocked[bufferToProcess] =false;
	printf ("buffer processed and released: %d\r\n", bufferToProcess);


}


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
//  XBEE send
// ------------------------------------------------------------

void XBEE::buildFrame(frameData* frameData){

	//lock the xbee until message sent, or timeout
	sendingFrameIsBusy = true;

	//frameToSend = {};  //reset //https://stackoverflow.com/questions/15183429/c-completely-erase-or-reset-all-values-of-a-struct
	//uint8_t test []= {0x7E, 0x00, 0x14, 0x10, 0x01, 0x00, 0x7D, 0x33, 0xA2, 0x00, 0x41, 0x05, 0xBC, 0x87, 0xFF, 0xFE, 0x00, 0x00, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x7D, 0x5E};//complete frame API2
	//uint8_t test []= {0x10, 0x01, 0x00, 0x7D, 0x33, 0xA2, 0x00, 0x41, 0x05, 0xBC, 0x87, 0xFF, 0xFE, 0x00, 0x00, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36}; //escaped
	//uint8_t test []= {0x10, 0x01, 0x00, 0x13, 0xA2, 0x00, 0x41, 0x05, 0xBC, 0x87, 0xFF, 0xFE, 0x00, 0x00, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36};

	frameToSend.frame[0] = 0x7E;
	frameToSend.frame[1] = frameData->length >>8;
	frameToSend.frame[2] = frameData->length & 0xFF; //http://www.avrfreaks.net/forum/c-programming-how-split-int16-bits-2-char8bit

	uint16_t buildFrameIndex = 3;
	for (uint8_t i=0; i<frameData->length; i++){
		//escape frame
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
		printf("byte: %02x \r\n",byteToSend);

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

/*
	printf("frame unescaped.\r\n");
			for (uint8_t i = 0; i< frameToSend.length;i++){
				printf("%02x ", frameToSend.frame[i]);
			}
	printf("\r\nframe built.\r\n");
		for (uint8_t i = 0; i< frameToSend.lengthEscaped;i++){
			printf("%02x ", frameToSend.frameEscaped[i]);
		}
*/
	sendFrame(&frameToSend);
}

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
void XBEE::receiveFrame(char receivedByte){
	//This is done during the interrupt. keep it short! avoid printf!!
	//printf("+++ %02X ", receivedByte);
	//shorthand for the correct buffer to store the received byte
	frameReceive* buffer;
	buffer = &this->receiveFrameBuffers[this->receiveBufferCounter];
	//the shorthand: buffer->packageLength   is equal to (*buffer).packageLength; //https://stackoverflow.com/questions/22921998/error-request-for-member-size-in-a-which-is-of-pointer-type-but-i-didnt

	if( buffer->packageRecordPosition  >= RECEIVE_BUFFER_SIZE ){
		//buffer overflow
		printf("buffer overflowprogram buffer is %d, XBEE package length is up to 65535 + 3 bytes. --> Overflow, will reset buffer.", RECEIVE_BUFFER_SIZE);
		buffer->packageRecordPosition = 0;
		buffer->packageRecording = false;
		buffer->packageLength = 0;
		buffer->packageData[0]='\0';
		unescapeNextReceivedByte = false;
	}

	if (buffer->packageRecordPosition > buffer->packageLength + 4){
		//package length information conflict with number of received bytes
		printf("ASSERT error: error in communication, package is longer than indicated. Will reset buffer.\r\n");
		buffer->packageRecordPosition = 0;
		buffer->packageRecording = false;
		buffer->packageLength = 0;
		buffer->packageData[0]='\0';
		unescapeNextReceivedByte = false;

	}else if (receivedByte == 0x7E && !unescapeNextReceivedByte){
		//packages are sent in API2 mode which means 0x7E is always start (
		//new package
		buffer->packageLength = 0;
		buffer->packageRecording = true;
		buffer->packageRecordPosition = 0;
		buffer->packageData[0]='\0';
		unescapeNextReceivedByte = false;

		buffer->packageData[buffer->packageRecordPosition] = receivedByte;
		buffer->packageRecordPosition ++;
		buffer->packageData[buffer->packageRecordPosition]='\0';

		printf ("new package, store in buffer: %d\r\n", this->receiveBufferCounter);

	}else if (receivedByte == 0x7D){
		unescapeNextReceivedByte = true;

	}else if (buffer->packageRecording ){

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
		buffer->packageData[buffer->packageRecordPosition] = receivedByte;
		buffer->packageRecordPosition ++;
		buffer->packageData[buffer->packageRecordPosition]='\0';

		//when length information bytes are received, they are used to check when the package is received completely.
		if (buffer->packageRecordPosition == 3){
			buffer->packageLength = buffer->packageData[1]<<8 | buffer->packageData[2]; //this->packageLength = this->receiveBuffer[1]*256 + this->receiveBuffer[2]; //
		}

		//end of package
		if (buffer->packageRecordPosition == buffer->packageLength + 4){
			printf("full package received \r\n");
			buffer->packageRecording = false;

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
					printf("%d\r\n" ,this->receiveBufferCounter );
				}

				printf("%d\r\n" ,this->receiveFrameBuffersIsLocked[this->receiveBufferCounter] );
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

