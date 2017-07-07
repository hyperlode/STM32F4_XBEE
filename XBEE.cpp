
#include "XBEE.h"

XBEE::XBEE(){
	receiveBufferCounter = 0;
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

void XBEE::readReceivedLocalPackage(){
	for (uint8_t i = 0; i< this->packageReceiveBuffer[receiveBufferCounter].packageRecordPosition; i++)
	{
		printf("%02X ", packageReceiveBuffer[receiveBufferCounter].packageRecordBuffer[i]);
	}
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

void XBEE::receiveLocalPackage(char receivedByte){
	//check if byte part of api package

	//shorthand for the correct buffer to store the received byte
	receivePackage buffer;
	buffer = this->packageReceiveBuffer[receiveBufferCounter];

	if(buffer.packageRecordPosition  >= RECEIVE_BUFFER_SIZE ){
		//buffer overflow
		printf("program buffer is %d, XBEE package length is up to 65535 + 3 bytes. --> Overflow, will reset buffer.", RECEIVE_BUFFER_SIZE);
		buffer.packageRecordPosition = 0;
		buffer.packageRecording = false;
		buffer.packageLength = 0;
		buffer.packageRecordBuffer[0]='\0';
	}

	if (buffer.packageRecordPosition > buffer.packageLength + 5){
		printf("ASSERT error: error in communication, package is longer than indicated. Will reset buffer.");
		buffer.packageRecordPosition = 0;
		buffer.packageRecording = false;
		buffer.packageLength = 0;
		buffer.packageRecordBuffer[0]='\0';

	}else if (receivedByte == 0x7E){
		//packages are sent in API2 which means 0x7E is always start (
		//new package
		buffer.packageLength = 0;
		buffer.packageRecording = true;
		buffer.packageRecordPosition = 0;
		buffer.packageRecordBuffer[0]='\0';

		buffer.packageRecordBuffer[buffer.packageRecordPosition] = receivedByte;
		buffer.packageRecordPosition ++;
		buffer.packageRecordBuffer[buffer.packageRecordPosition]='\0';

		printf ("new package\r\n");


	}else if (buffer.packageRecording ){
		//record package

		buffer.packageRecordBuffer[buffer.packageRecordPosition] = receivedByte;
		buffer.packageRecordPosition ++;
		buffer.packageRecordBuffer[buffer.packageRecordPosition]='\0';

		//when length bytes are received, they are used to check when the package is received completely.
		if (buffer.packageRecordPosition == 3){
			buffer.packageLength = buffer.packageRecordBuffer[1]<<8 | buffer.packageRecordBuffer[2]; //this->packageLength = this->receiveBuffer[1]*256 + this->receiveBuffer[2]; //
		}
		if (buffer.packageRecordPosition == buffer.packageLength + 5){
			printf("full package received \r\n");
			buffer.packageRecording = false;
			printf ("tmp  %02X\r\n",buffer.packageRecordBuffer[buffer.packageRecordPosition-1]);
		}

	}else {
		printf ("stray char received: %02X\r\n");
	}
}

/*
void XBEE::receiveBuffer_writeByte(char receivedByte){
	if (this->receiveBufferPosition >= RECEIVE_BUFFER_SIZE ){
		//check overflow
		if (!receiveBufferOverflow){
			printf("serial buffer overflow, max chars: %d. \r\n", RECEIVE_BUFFER_SIZE);

		}
		receiveBufferOverflow = true;
		receiveBufferPosition = 0;
		
	
	}else if (!receiveBufferOverflow){
		//record char in buffer
		receiveBuffer[receiveBufferPosition] = receivedByte;
		receiveBufferPosition ++;
		receiveBuffer[receiveBufferPosition]='\0';
		printf("buffer: %d, %c\r\n", receiveBufferPosition, receivedByte);
		if (receivedByte == 0x7E){
			printf("tilde");
		}
	}
	
	
}
*/

bool XBEE::byteIsAvailable(){
	printf("ahoi");

}

void XBEE::receiveInterruptHandler(char c){
	//printf(".%c",c);
	//this->receiveBuffer_writeByte(c);
	this->receiveLocalPackage(c);
	//printf("ahoi");
}

