
#include "XBEE.h"

XBEE::XBEE(){
	receiveBufferCounter = 0;
	for (int i=0;i<NUMBER_OF_RECEIVEBUFFERS;i++){
		this->packageReceiveBuffersToBeProcessed[i]= NOTHING_TO_BE_PROCESSED;
		this->packageReceiveBufferIsLocked[i] = false;
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
/*
void XBEE::receiveBuffer_Readout_Flush(){
	//http://rubenlaguna.com/wp/2009/03/12/example-of-xbee-api-frames/
	//printf("buffer: %s\r\n", receiveBuffer);

	for (uint8_t i = 0; i< receiveBufferPosition; i++)
	{
		printf("%02X ", receiveBuffer[i]);
	}
	receiveBufferOverflow = false;
	receiveBufferPosition = 0;
}
*/




void XBEE::readReceivedLocalPackage(uint8_t receiveBuffer){
	//packageReceiveBuffer

	for (uint8_t i = 0; i< this->packageReceiveBuffer[receiveBuffer].packageRecordPosition; i++)
	{
		printf("%02X ", this->packageReceiveBuffer[receiveBuffer].packageData[i]);
	}
	/*
	for (uint8_t i = 0; i< this->packageReceiveBuffer[receiveBufferCounter].packageRecordPosition; i++)
	{
		printf("%02X ", packageReceiveBuffer[receiveBufferCounter].packageData[i]);
	}
	*/

	//this->receiveBufferOverflow = false;
	//this->packageRecordPosition = 0;
	//this->packageRecording = false;
}

/*
void XBEE::unescapeAPIFrame(){

	if (this->excapeNextChar){
		if (receivedByte == 0x33){

		}else if (receivedByte == ){
		}else if (receivedByte == ){
		}else if (receivedByte == ){
		}else{
			printf ("escape error");

		}

	}
	if (receivedByte == 0x7D){
			//this is an escape character, and simply means that the next byte is escaped.
			this->escapeNextChar = true;
		}else if{

		}
}
*/
/*
bool XBEE::apiFrameIsValid(char* frame){
	//copy to buffer

	//checksum on unescaped frame.
//http://knowledge.digi.com/articles/Knowledge_Base_Article/Calculating-the-Checksum-of-an-API-Packet
		//test checksum:
		uint32_t sum = 0;
		for(uint16_t i=0;i<this->packageLength;i++){
			//sum += this->receiveBuffer[i+3];
			sum = sum + this->receiveBuffer[i+3];
			printf("checksum + %08x: %08X\r\n", this->receiveBuffer[i+3],sum);
			//checksum + last 8 bits must be FF.
		}
}
/**/
/*

void XBEE::unescapeAPIFrame(char* frame){

}

void XBEE::escapeAPIFrame(char* frame){

}

void XBEE::sendLocalPackage(){
}
*/

void XBEE::processReceivedPackage(){

	int16_t bufferToProcess =  NOTHING_TO_BE_PROCESSED;
	//check if first element is a buffer number that needs to be processed.
	if (this->packageReceiveBuffersToBeProcessed[0] != NOTHING_TO_BE_PROCESSED){
		bufferToProcess = this->packageReceiveBuffersToBeProcessed[0];
	}

	if (bufferToProcess != NOTHING_TO_BE_PROCESSED){
		readReceivedLocalPackage((uint8_t)bufferToProcess);


		//move all elements one step to the front, make last slot empty.
		for (uint8_t i=0; i< NUMBER_OF_RECEIVEBUFFERS-1; i++){
			this->packageReceiveBuffersToBeProcessed[i] = this->packageReceiveBuffersToBeProcessed[i +1];
		}
		this->packageReceiveBuffersToBeProcessed[NUMBER_OF_RECEIVEBUFFERS-1] = NOTHING_TO_BE_PROCESSED;

		this->packageReceiveBufferIsLocked[bufferToProcess] =false;
		printf ("buffer processed and released: %d\r\n",bufferToProcess);
	}else{
		printf ("nothing to process... \r\n");

	}

}


void XBEE::refresh(){
	//check if package received.



	//process
	processReceivedPackage();


}

void XBEE::stats(){
	printf("receive package stats: \r\n");
	for (int i=0;i< NUMBER_OF_RECEIVEBUFFERS; i++){
		printf("buffer: %d, locked?:%d\r\n",i,this->packageReceiveBufferIsLocked[i]);
	}

	printf("packages to be processed(-1 means 'free'):  \r\n");
	for (int i=0;i< NUMBER_OF_RECEIVEBUFFERS; i++){
		printf("slot: %d, buffer:%d\r\n",i,this->packageReceiveBuffersToBeProcessed[i]);
	}



}

void XBEE::receiveLocalPackage(char receivedByte){
	//This is done during the interrupt. keep it short! avoid printf!!

	//shorthand for the correct buffer to store the received byte
	receivePackage* buffer;
	buffer = &this->packageReceiveBuffer[this->receiveBufferCounter];
	//the shorthand: buffer->packageLength   is equal to (*buffer).packageLength; //https://stackoverflow.com/questions/22921998/error-request-for-member-size-in-a-which-is-of-pointer-type-but-i-didnt

	if( buffer->packageRecordPosition  >= RECEIVE_BUFFER_SIZE ){
		//buffer overflow
		printf("buffer overflowprogram buffer is %d, XBEE package length is up to 65535 + 3 bytes. --> Overflow, will reset buffer.", RECEIVE_BUFFER_SIZE);
		buffer->packageRecordPosition = 0;
		buffer->packageRecording = false;
		buffer->packageLength = 0;
		buffer->packageData[0]='\0';
	}

	if (buffer->packageRecordPosition > buffer->packageLength + 5){
		//package length information conflict with number of received bytes
		printf("ASSERT error: error in communication, package is longer than indicated. Will reset buffer.\r\n");
		buffer->packageRecordPosition = 0;
		buffer->packageRecording = false;
		buffer->packageLength = 0;
		buffer->packageData[0]='\0';

	}else if (receivedByte == 0x7E){
		//packages are sent in API2 which means 0x7E is always start (
		//new package
		buffer->packageLength = 0;
		buffer->packageRecording = true;
		buffer->packageRecordPosition = 0;
		buffer->packageData[0]='\0';

		buffer->packageData[buffer->packageRecordPosition] = receivedByte;
		buffer->packageRecordPosition ++;
		buffer->packageData[buffer->packageRecordPosition]='\0';

		printf ("new package, store in buffer: %d\r\n", this->receiveBufferCounter);


	}else if (buffer->packageRecording ){
		//record byte
		buffer->packageData[buffer->packageRecordPosition] = receivedByte;
		buffer->packageRecordPosition ++;
		buffer->packageData[buffer->packageRecordPosition]='\0';

		//when length information bytes are received, they are used to check when the package is received completely.
		if (buffer->packageRecordPosition == 3){
			buffer->packageLength = buffer->packageData[1]<<8 | buffer->packageData[2]; //this->packageLength = this->receiveBuffer[1]*256 + this->receiveBuffer[2]; //
		}

		//end of package
		if (buffer->packageRecordPosition == buffer->packageLength + 5){
			printf("full package received \r\n");
			buffer->packageRecording = false;



			//add finished package to list of packages to be processed
			uint8_t i =0;
			while (this->packageReceiveBuffersToBeProcessed[i] != NOTHING_TO_BE_PROCESSED && i<NUMBER_OF_RECEIVEBUFFERS){
				i++;
			}

			if (i>= NUMBER_OF_RECEIVEBUFFERS){
				printf("unprocessed overflow error: all package process buffers are full. package will be neglected.");

			}else{

				this->packageReceiveBuffersToBeProcessed[i] =this->receiveBufferCounter;
				this->packageReceiveBufferIsLocked[this->receiveBufferCounter] = true;


				//next buffer activate
				this->receiveBufferCounter++;

				if (this->receiveBufferCounter >= NUMBER_OF_RECEIVEBUFFERS){
					this->receiveBufferCounter = 0;
					printf("%d\r\n" ,this->receiveBufferCounter );
				}

				printf("%d\r\n" ,this->packageReceiveBufferIsLocked[this->receiveBufferCounter] );
				//check if next buffer is free
				if (this->packageReceiveBufferIsLocked[this->receiveBufferCounter] ){
					printf("unprocessed overflow ERROR: all package buffers full, wait with sending... \r\n "); //todo
				}
			}



		}
	}else {
		printf ("stray char received: %02X\r\n",receivedByte);
	}
}


bool XBEE::byteIsAvailable(){
	printf("ahoi");

}

void XBEE::receiveInterruptHandler(char c){
	//printf(".%c",c);
	//this->receiveBuffer_writeByte(c);
	this->receiveLocalPackage(c);
	//printf("ahoi");
}
