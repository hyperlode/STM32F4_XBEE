
#include "XBEE.h"


const uint8_t atTest[] = {0x08, 0x01, 0x53, 0x48};
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



XBEE::XBEE(){
	clearReceiveBuffers();
}

void XBEE::reset(){
	clearReceiveBuffers();
	releaseSendLock();
}

void XBEE::init(uint8_t UART_Number, uint32_t baud, uint32_t* millis){
	clearReceiveBuffers();
	setSleepPinPB9();
	this->millis = millis;
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

//------------------------------------------------------------------------
//XBEE sleep behaviour. Important: will only work if SM parameter is configured correctly. i.e. value 1 for defined by pin. (high = sleeps)
//------------------------------------------------------------------------

void XBEE::setSleepPinPB9()
{
	this->sleepPinPort = GPIOB;
	this->sleepPin = GPIO_Pin_9;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitDef;
	GPIO_InitDef.GPIO_Pin = this->sleepPin;
	GPIO_InitDef.GPIO_Mode = GPIO_Mode_OUT ;
	GPIO_InitDef.GPIO_OType = GPIO_OType_PP;
	GPIO_InitDef.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitDef.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(this->sleepPinPort ,&GPIO_InitDef);
	wakeUp();

}
void XBEE::sleep(){

	GPIO_SetBits(this->sleepPinPort,this->sleepPin);
	this->isAsleep = true;
	printf("Radio sleeps...\r\n");
}

void XBEE::wakeUp(){
	GPIO_ResetBits(this->sleepPinPort,this->sleepPin);
	this->isAsleep = false;
	printf("Radio woke up...\r\n");
}

bool XBEE::isSleeping(){
	return this->isAsleep;
}
//------------------------------------------------------------------
//send data to destination xbee
//------------------------------------------------------------------

bool XBEE::sendMessageToDestinationAwaitResponse(char* message, uint16_t messageLength, uint32_t timeout_millis){
	//printf("messg length : %d , \r\n", messageLength);
	if (!sendMessageToDestination(message, messageLength,true,timeout_millis)){
		return false;
	}else{
		return true;
	}
}



void XBEE::generateFrameType0x00(frameData* frameData,char* message, uint16_t messageLength, uint8_t id){
	//https://www.digi.com/resources/documentation/digidocs/pdfs/90001500.pdf  p117
	(*frameData).length = 11 + messageLength;

	frameData->data[0] = 0x00; //frame type = Transmit request
	frameData->data[1] = id;

	//destination address
	for (uint8_t i=0;i<8;i++){
		frameData->data[2 + i] = destinationXbee.address[i];
		//printf("b: %02x",destinationXbee.address[i]);
	}

	frameData->data[10] = 0x00; //options

	//copy the message.  max 110bytes
	for (uint16_t i = 0;i<messageLength; i++){
		frameData->data[11 + i] = message[i];
	}
}

void XBEE::generateFrameType0x10(frameData* frameData,char* message, uint16_t messageLength, uint8_t id){

	frameData->length = 14 + messageLength;

	frameData->data[0] = 0x10; //frame type = Transmit request
	frameData->data[1] = id;

	//destination address
	for (uint8_t i=0;i<8;i++){
		frameData->data[2 + i] = destinationXbee.address[i];
	}

	frameData->data[10] = 0xFF; // 16 bit dest address->
	frameData->data[11] = 0xFE;

	frameData->data[12] = 0x00; //broadcast radius
	frameData->data[13] = 0x00; //options

	//copy the message.
	for (uint16_t i = 0;i<messageLength; i++){
		frameData->data[14 + i] = message[i];
	}
}




bool XBEE::sendMessageToDestination(char* message, uint16_t messageLength, bool awaitResponse, uint32_t timeout_millis){


	if (!destinationXbee.isValid){
		printf("error: destination not configured, will try to retrieve destination from local xbee\r\n");
		if (!getDestinationAddressFromXbee()){
			return false;
		}
	}
	uint32_t start_millis = *this->millis;


	frameData frameData;


#ifdef FIRMWARE_802.15.4_TH
	generateFrameType0x00(&frameData,message,messageLength,getNextIdForSendFrame(awaitResponse));
#else
	//frame type: 0X10 transmit request
	generateFrameType0x10(&frameData,message,messageLength,getNextIdForSendFrame(awaitResponse));
#endif


	//send the frame
	if (!buildAndSendFrame(&frameData)){
		printf("message sending failed. \r\n");
		return false;
	}

	if (awaitResponse){
		return awaitResponseToLastMessage(timeout_millis);
	}else{
		return true; //sending message success. no feedback requested.
	}



}


void XBEE::processTransmitStatus(){
	if (transmitResponse.status == 0x00){
#ifdef XBEE_PRINTF_INFO
		printf("transmit success (id:%d)\r\n",transmitResponse.id);
#endif
	}else{
		printf("transmit failed status(id:%d): %02x\r\n",transmitResponse.id,transmitResponse.status);
	}
	releaseSendLock();
}

void XBEE::processTxTransmitStatus(){
	if (transmitResponse.status == 0x00){
#ifdef XBEE_PRINTF_INFO
		printf("transmit success (id:%d)\r\n",transmitResponse.id);
#endif
	}else{
		printf("transmit failed status(id:%d): %02x\r\n",transmitResponse.id,transmitResponse.status);
	}
	releaseSendLock();
}


//--------------------------------------------------
//  AT Commands: remote connectivity
//--------------------------------------------------
bool XBEE::searchActiveRemoteXbees(uint32_t timeout_millis){

	//set time out time
	printf("millis: %d\r\n", *this->millis);
	uint32_t start_millis = *this->millis;
	clearNeighbours();


	//repopulate list
	if (! sendAtCommandAndAwaitWithResponse(AT_DISCOVER_NODES_ND, timeout_millis) ){
		//this command can have multiple responses, one from each remote xbee. so, let's keep the lock in place until it times out. (for the sake of simplicity), if messages appear later, they should be handeled properly in the at response processing.

		//return false;
	}


/*
	printf("local xbee address set: ");
	for (uint8_t i=0 ; i<8 ; i++){
		printf ("%02x ", senderXbee.address[i]);
	}

*/
	printf("\r\n");
	printf("millis: %d\r\n", *this->millis);
	return true;
}

void XBEE::clearNeighbours(){
	//reset list of all neighbours (erase names)
	for (uint8_t i = 0; i< BUFFER_NEIGHBOUR_XBEES_SIZE; i++){
		for (uint8_t j=0; j<XBEE_NAME_MAX_NUMBER_OF_CHARACTERS; j++){
			neighbours[i].name[j] =  ' ';
		}
		neighbours[i].isValid = false;
	}
	this->numberOfNeighbours = 0;

}
bool XBEE::setNeighbourAsRemote(uint8_t numberInList){
	if (neighbours[numberInList].isValid){
		destinationXbee = neighbours[numberInList];
		//printf("millis at start: %d\r\n", *this->millis);
	}else{
		printf("invalid neighbour\r\n");
		return false;
	}

	if (!sendAtCommandAndAwaitWithResponse(AT_MAC_DESTINATION_HIGH_DH, &destinationXbee.address[0], 4, 500 )){
		return false;
	}
	if (!sendAtCommandAndAwaitWithResponse(AT_MAC_DESTINATION_LOW_DL, &destinationXbee.address[4], 4, 500 )){
		return false;
	}
	if (!saveChangesinLocalXbee())return false;

	printf("Remote set.\r\n");

	return true;
}


bool XBEE::saveChangesinLocalXbee(){
	//by peeking at XCTU, at WR and and AC are written to store settings to non volatile memory.
	if (!sendAtCommandAndAwaitWithResponse(AT_WRITE_WR,  500 )){
		return false;
	}
	if (!sendAtCommandAndAwaitWithResponse(AT_APPLY_CHANGES_AC,  500 )){
		return false;
	}
	return true;
}

void XBEE::displayNeighbours(){
	if (this->numberOfNeighbours ==0){
		printf("No neighbours \r\n");
	}else{
		printf("Neighbours of this XBEE: \r\n");
		/**/
		for (uint8_t i = 0; i< BUFFER_NEIGHBOUR_XBEES_SIZE; i++){
			//printf ("  %d: ", i);
			if (neighbours[i].isValid){
				//for (uint16_t j=0; j<XBEE_NAME_MAX_NUMBER_OF_CHARACTERS; j+2){
					for (uint16_t j=0; j<XBEE_NAME_MAX_NUMBER_OF_CHARACTERS; j++){
						//printf( "%c%c", neighbours[i].name[j], neighbours[i].name[j+1]) ;
					printf("_%c", neighbours[i].name[j]);
					//printf(neighbours[i].name[j]);
				}

				printf(" : ");
				for (uint8_t j=0; j<8; j++){
					printf( "%02x ", neighbours[i].address[j]) ;
				}

				printf("\r\n");
			}
		}
		/**/
	}
}




//------------------------------------------------------------------------------------
// XBEE local AT Commands
//------------------------------------------------------------------------------------

bool XBEE::getLocalXbeeAddress(uint32_t timeout_millis){

	//set time out time
	//printf("-millis: %d\r\n", *this->millis);
	uint32_t start_millis = *this->millis;

	//get msb bytes.

	if (!sendAtCommandAndAwaitWithResponse(AT_MAC_HEIGH_SH , timeout_millis) ){
		return false;
	}

	if (!sendAtCommandAndAwaitWithResponse(AT_MAC_LOW_SL, start_millis + timeout_millis - *this->millis) ){
		return false;
	}

	printf("local xbee address set: ");
	for (uint8_t i=0 ; i<8 ; i++){
		printf ("%02x ", senderXbee.address[i]);
	}
	printf("\r\n");
	//printf("--millis: %d\r\n", *this->millis);
	return true;
}

bool XBEE::getDestinationAddressFromXbee(){
	printf("retrieve destination address from local XBEE: ");
	//for (uint8_t i=0 ; i<8 ; i++){
	//	printf ("%02x ", destinationXbee.address[i]);
	//}
	printf("\r\n");

	//get msb bytes.

	if (!sendAtCommandAndAwaitWithResponse(AT_MAC_DESTINATION_HIGH_DH , AT_COMMAND_TIMEOUT_GET_SETTING) ){
		return false;
	}

	if (!sendAtCommandAndAwaitWithResponse(AT_MAC_DESTINATION_LOW_DL, AT_COMMAND_TIMEOUT_GET_SETTING) ){
		return false;
	}
	destinationXbee.isValid = true;

	for (uint8_t i=0 ; i<8 ; i++){
		printf ("%02x ", destinationXbee.address[i]);
	}
	printf("\r\n");
	return true;
}



//-----------------------------------------------------------------------------
// XBEE modem status
//---------------------------------------------------------------------------




void XBEE::processModemStatus(frameReceive* frame){

	printf(	"modem status: ");
	switch(frame->frame[FRAME_FRAMEDATA_STARTINDEX+1]){
		case 0x02:
			printf(	"device associated\r\n");
			break;

		case 0x03:
			printf(	"device disassociated\r\n");
			break;

		default:
			printf(	"code: %02x\r\n",frame->frame[ FRAME_FRAMEDATA_STARTINDEX + 1]);
			break;

	}
	printf(	"\r\n");

	/*
	for (uint8_t i = 0; i< frame->lengthFrameData; i++){
		//printf("char: %c\r\n", frame->frame[FRAME_FRAMEDATA_STARTINDEX + i]);

		 //, package->lengthFrameData

	}
	*/

}

//-----------------------------------------------------------------------------
//AT Command mode
//---------------------------------------------------------------------------


//void XBEE::sendLocalATCommand(uint16_t atCommand, uint8_t* payload ){
bool XBEE::sendLocalATCommand(uint16_t atCommand, uint8_t* parameter, uint8_t parameterLength, bool awaitResponse){
	//always use the timeOut version!

	// frame type: 0x08 AT Command
	//test://get address.
	//printf("---sending AT command (please wait)---\r\n");
	frameData frameData;
	atFrameData atData;

	atData.atCommand = atCommand;


	//atData.data [AT_DATA_SIZE];

	if (parameterLength>AT_DATA_SIZE){
		printf("ERROR: parameter to large, increase AT_DATA_SIZE \r\n");
		return false;
	}
	for(uint8_t i = 0; i< parameterLength; i++){
		atData.data[i] = parameter[i];
	}

	atData.dataLength = parameterLength;

	atData.id = getNextIdForSendFrame(awaitResponse);

#ifdef XBEE_PRINTF_INFO
	displayAtFrameData(&atData);
#endif

	atFrameDataToFrameData(&atData,&frameData);

	bool success = buildAndSendFrame(&frameData);

	//displayFrame(&frameToSend);
	//printf("---AT command sent---\r\n");
	return success;
}

bool XBEE::sendAtCommandAndAwaitWithResponse(uint16_t atCommand, uint32_t timeout_millis ){

	uint8_t* dummy;
	sendAtCommandAndAwaitWithResponse(atCommand, dummy,0, timeout_millis );

}

bool XBEE::sendAtCommandAndAwaitWithResponse(uint16_t atCommand, uint32_t parameterInt,  uint32_t timeout_millis){

	//in API frames, the integer numbers are sent as number not as ascii! so: 1 is 0x01  and not 0x31

	//convert int to byte array (for parameter).
	printf("paramfffeter %d\r\n",parameterInt);
	uint8_t parameter [AT_DATA_SIZE];

	#define NONSENSE_VALUE 66

	uint8_t bitFieldByteLength = NONSENSE_VALUE;

	for (uint8_t i = 0;i<4;i++){
		//split the 32 bits in four parts of 8 bits (starting with msb)
		parameter[i] = (uint8_t)(parameterInt>>(8*(3-i)));//parameter[i] = (uint8_t)parameterInt>>(8*(4-i)); //32bit parameterint, chop in pieces.

		//check for first non zero byte.
		if (bitFieldByteLength == NONSENSE_VALUE && parameter[i] != 0){

			bitFieldByteLength = (4-i);
		}
	}

	//if int was zero, send a zero
	if (bitFieldByteLength == 66){
		bitFieldByteLength = 1; //the sent byte will be once: 0x00
	}

	XBEE::sendAtCommandAndAwaitWithResponse(atCommand, &parameter[4-bitFieldByteLength], bitFieldByteLength, timeout_millis );
}

bool XBEE::sendAtCommandAndAwaitWithResponse(uint16_t atCommand, uint8_t* parameter, uint8_t parameterLength, uint32_t timeout_millis ){
	//execute full at command with response.
	//set time out time
	if (!sendLocalATCommand( atCommand,  parameter, parameterLength , true)){
		printf("sending AT command failed...\r\n");
		return false;
	}
	//printf("given lengthss \r\n: %d", parameterLength);
	//reponse
	if(! awaitResponseToLastMessage(timeout_millis)){
		return false;
	}


	//only show response if it contains some payload
	if (atResponse.responseDataLength>0){
		displayAtCommandResponseFrameData(&atResponse);
	}

	//sent command failed.
	if (atResponse.status != 0){
		return false;
	}

	return true;
}


void XBEE::atFrameDataToFrameData(atFrameData* atData, frameData* frameData){
	//translate atFrame to framedata of an API package.

	frameData->length = 4 + atData->dataLength;
	frameData->data[0] = XBEE_FRAME_TYPE_AT_COMMAND; //frame type = AT Command
	frameData->data[1] = atData->id;
	frameData->data[2] = atData->atCommand>>8;
	frameData->data[3] = atData->atCommand & 0x00FF;
	for(uint8_t i = 0; i< atData->dataLength;i++){
		frameData->data[4+i] =  atData->data[i];
	}

}

void XBEE::displayAtFrameData(atFrameData* atFrame){
	printf("Display at command frame: \r\n");
	printf("type: AT command\r\n");
	printf("id: %d\r\n",atFrame->id);
	printf("command: %c%c\r\n", atFrame->atCommand>>8, atFrame->atCommand& 0x00FF );
	if (atFrame->dataLength > 0){
		printf("data: ");
		for (uint8_t i = 0; i< atFrame->dataLength;i++){
			printf("%02x ",atFrame->data[i]);
		}
		printf(" (as ASCII:");
		for (uint8_t i = 0; i< atFrame->dataLength;i++){
				printf("%c ",atFrame->data[i]);
			}
		printf(")\r\n");
	}
}

void XBEE::displayAtCommandResponseFrameData(atCommandResponseFrameData* atFrame){
	//printf("display at command response frame: \r\n");
	printf("type: AT command response\r\n");
	printf("id: %d\r\n",atFrame->id);
	printf("command: %c%c\r\n", atFrame->atCommand>>8, atFrame->atCommand& 0x00FF );
	printf("status: %d\r\n",atFrame->status);

	if (atFrame->responseDataLength > 0){
		printf("data: ");
		for (uint8_t i = 0; i< atFrame->responseDataLength;i++){
			printf("%02x ",atFrame->responseData[i]);
		}
		printf(" (as ASCII:");
		for (uint8_t i = 0; i< atFrame->responseDataLength;i++){
				printf("%c ",atFrame->responseData[i]);
			}
		printf(")\r\n");
	}
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


	if (!atResponse.status){
#ifdef XBEE_PRINTF_DEBUG

		printf("At response received (at %dms) %02x:(code:%d)",*this->millis,atResponse.atCommand, atResponse.status);
#endif
	}else{
		printf("At response received (at %dms) %02x:(code:%d)",*this->millis,atResponse.atCommand, atResponse.status);
	}

	switch(atResponse.status){
	case 0:
#ifdef XBEE_PRINTF_DEBUG
		printf("success.\r\n");


#endif
		break;
	case 1:
		printf("ERROR.\r\n");
		break;
	case 2:
		printf("Invalid command.\r\n");
		break;
	case 3:
		printf("Invalid parameter.\r\n");
		break;
	case 4:
		printf("tx failure\r\n");
		break;
	default:
		printf("failed.(code: %d)\r\n", atResponse.status);
		break;


	}


	//response actions
	switch (atResponse.atCommand){
		case AT_NODE_IDENTIFIER_NI:
			printf("datalength: %d \r\n",atResponse.responseDataLength);
			for (uint8_t i =0;i<atResponse.responseDataLength;i++){
				senderXbee.name[i] = atResponse.responseData[i];
			}
			senderXbee.name[atResponse.responseDataLength] = '\0';
			releaseSendLock();
			break;

		case AT_MAC_DESTINATION_HIGH_DH:
			for (uint8_t i =0;i<4;i++){
				destinationXbee.address[i] = atResponse.responseData[i];
			}
			releaseSendLock();
			break;

		case AT_MAC_DESTINATION_LOW_DL:
			for (uint8_t i =0;i<4;i++){
				destinationXbee.address[i+4] = atResponse.responseData[i];
			}
			releaseSendLock();
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

		case AT_DISCOVER_NODES_ND:
		{

			displayAtCommandResponseFrameData(&atResponse);
			//record the address
			for (uint8_t i=0;i<8;i++){
				neighbours[this->numberOfNeighbours].address[i] = atResponse.responseData[2+i];
			}
			uint8_t i = 15;

			//record the name
			bool charValid = true;
			for (uint8_t i=0;i<XBEE_NAME_MAX_NUMBER_OF_CHARACTERS;i++){
				if (atResponse.responseData[10 + i] == 0){
					charValid = false;
				}

				if (charValid){
					neighbours[this->numberOfNeighbours].name[i] = atResponse.responseData[10 + i];
				}else{
					neighbours[this->numberOfNeighbours].name[i] = 0x20; //space.
				}
			}
			neighbours[this->numberOfNeighbours].isValid = true;

			//increase the number of neighbours.
			this->numberOfNeighbours ++;
			//printf("number of neighbours: %d" , this->numberOfNeighbours);
		}
			break;

		//case AT_FIND_NEIGHBOURS_FN:

			//releaseSendLock();
		//	break;
		default:
			//printf("AT command %04x not included in program list. Contact Lode.\r\n" , atResponse.atCommand);
			releaseSendLock();
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
	printf("Frame: ");
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
	if (senderXbeeLockedWaitingForResponse){
		if (senderXbeeLockedWaitingForResponse){//&& sendingFrameIsBusy
			printf("error (not sent): busy waiting for response from previous message.");
		}else{
			printf("busy processing and sending previous frame.");
		}
		return false;
	}

	if (isSleeping()){
		printf("XBEE sleeping. Sending nor receiving.\r\n");

	}

	if (frameData->data[1]){ //if ID is not zero, wait for response
		//printf("will lock sending until response arrived.");
		senderXbeeLockedWaitingForResponse = true;
		idOfFrameWaitingForResponse = frameData->data[1];
	}
	//printf("dataalength: %d", frameData->length);
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

#ifdef XBEE_PRINTF_MINIMAL_INFO
	printf("send frame (before escaping):");
	for (uint8_t i = 0; i< frameToSend.length;i++){
		printf("%02x ", frameToSend.frame[i]);
	}
	printf("\r\n");
#endif
			/*
	printf("\r\nframe built.\r\n");
		for (uint8_t i = 0; i< frameToSend.lengthEscaped;i++){
			printf("%02x ", frameToSend.frameEscaped[i]);
		}
	/**/
	sendFrame(&frameToSend);
	return true;


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
#ifdef XBEE_PRINTF_DEBUG
	printf("send lock released \r\n");
#endif

}

bool XBEE::sendingIsLocked(){
	return senderXbeeLockedWaitingForResponse;
}


bool XBEE::awaitResponseToLastMessage(uint32_t  timeout_millis){

	//WARNING: this is not with interrupts, the processor will be occupied here until response arrives or function times out!

	//if a response comes with the id of the last send message. function will return true (sendLock will not be touched, it is assumed that the response handler deals with it.
	//if no response in the set timeout time, sendLock will be released but function will return false.

	//check response
	//bool timedOut = *this->millis > (start_millis + timeout_millis);
	uint32_t start_millis = *this->millis;
	bool timedOut = *this->millis > (start_millis + timeout_millis);

	while (sendingIsLocked() && !timedOut) {
		//sendLock is released if at response is received and processed. //sendingIsLocked() &&
		refresh();
		timedOut = *this->millis > (start_millis + timeout_millis);
	}

	//check if timed out
	//if (*this->millis > start_millis + timeout_millis ){
	if (timedOut){
		printf("Processing or awaiting AT response timed out...\r\n");
		printf("-->timeout setting[ms]:%d , time it took[ms]: %d\r\n", timeout_millis, *this->millis -  start_millis  );
		releaseSendLock(); //reset lock (knowing that we will send out a failed message...) if the response arrives anyways, it WILL be processed.
		return false;
	}
	return true;
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
	//printf("frametype: %02x.\r\n",frameType );
	//check if it is a requested response.
	if (frameType == XBEE_FRAME_TYPE_TRANSMIT_STATUS ){
		if (transmitResponse.id == idOfFrameWaitingForResponse && senderXbeeLockedWaitingForResponse){
			processTransmitStatus();
		}
		#ifdef XBEE_PRINTF_DEBUG
				printf("TX transmit status received \r\n. \r\n");
		#endif
	}else if( frameType == XBEE_FRAME_TYPE_TX_TRANSMIT_STATUS){
		if (transmitResponse.id == idOfFrameWaitingForResponse && senderXbeeLockedWaitingForResponse){
			processTxTransmitStatus();

#ifdef XBEE_PRINTF_DEBUG
			printf("TX transmit status received \r\n. \r\n");
#endif
		}
	}else if( frameType == XBEE_FRAME_TYPE_AT_COMMAND_RESPONSE){
		if (atResponse.id == idOfFrameWaitingForResponse && senderXbeeLockedWaitingForResponse){
			processAtResponse();
#ifdef XBEE_PRINTF_DEBUG
			printf("At response received. \r\n");
#endif
		}
	}else if( frameType == XBEE_FRAME_TYPE_MODEM_STATUS){
			processModemStatus(fifoTopFrame);
	}else{
		printf("frame is not a response %d, %d, %d \r\n", atResponse.id ,idOfFrameWaitingForResponse,senderXbeeLockedWaitingForResponse);
	}

	deleteTopFrameInReceivedFifoBuffer();
#ifdef XBEE_PRINTF_DEBUG
	printf("finished processing top frame.");
#endif

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
	//printf("heeey type: %02x" ,frameType );
	//AT response parsing
	if (frameType == XBEE_FRAME_TYPE_AT_COMMAND_RESPONSE){
#ifdef XBEE_PRINTF_DEBUG
		printf("AT response frame.\r\n");
#endif
		atResponse.id = frame->frame[FRAME_FRAMEDATA_STARTINDEX + 1];
		atResponse.atCommand = frame->frame[FRAME_FRAMEDATA_STARTINDEX + 2] <<8 | frame->frame[FRAME_FRAMEDATA_STARTINDEX + 3];
		atResponse.status = frame->frame[FRAME_FRAMEDATA_STARTINDEX + 4];
		atResponse.responseDataLength = frame->lengthFrameData - AT_FRAME_DATA_STARTINDEX;
		for (uint16_t i = 0; i< atResponse.responseDataLength; i++){
			atResponse.responseData[i] = frame->frame[FRAME_FRAMEDATA_STARTINDEX + AT_FRAME_DATA_STARTINDEX + i];
		}
#ifdef XBEE_PRINTF_INFO
		displayAtCommandResponseFrameData(&atResponse);
#endif

	}else if (frameType == XBEE_FRAME_TYPE_TRANSMIT_STATUS){
		//transmit status
		transmitResponse.id =  frame->frame[FRAME_FRAMEDATA_STARTINDEX + 1];
		transmitResponse.status =  frame->frame[FRAME_FRAMEDATA_STARTINDEX + 5];

	}else if (frameType == XBEE_FRAME_TYPE_TX_TRANSMIT_STATUS){
		//transmit status
		transmitResponse.id =  frame->frame[FRAME_FRAMEDATA_STARTINDEX + 1];
		transmitResponse.status =  frame->frame[FRAME_FRAMEDATA_STARTINDEX + 2];
	}

	return frameType;
}

void XBEE::displayTopFrameInReceivedFifoBuffer(frameReceive* frame){
	//checksum
	if (apiFrameIsValid(frame)){
#ifdef XBEE_PRINTF_DEBUG
		//printf("RECEIVE Info: Checksum correct\r\n");
#endif

	}else{
		printf("RECEIVE ERROR: Checksum not correct\r\n");
	}

	//receiveFrameBuffers
#ifdef XBEE_PRINTF_MINIMAL_INFO
	printf("received Frame (unescaped): ");
	for (uint8_t i = 0; i< frame->frameRecordIndex; i++)
	{
		printf("%02X ", frame->frame[i]);
	}
	printf("\r\n");
#endif

#ifdef XBEE_PRINTF_DEBUG
	//printf("\r\n only frameData:");
//	for (uint8_t i = 0; i< frame->lengthFrameData; i++)
	//{
	//	printf("%02X ", frame->frame[i + FRAME_FRAMEDATA_STARTINDEX]);
	//}
	//printf("\r\n");
#endif
}

void XBEE::clearReceiveBuffers(){
	receiveBufferCounter = 0;
	for (int i=0;i<NUMBER_OF_RECEIVEBUFFERS;i++){
		this->receiveFrameBuffersToBeProcessed[i]= NOTHING_TO_BE_PROCESSED;
	}

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
#ifdef XBEE_PRINTF_DEBUG
		printf ("new package, store in buffer: %d\r\n", this->receiveBufferCounter);
#endif

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
#ifdef XBEE_PRINTF_DEBUG
			printf("full package received \r\n");
#endif
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
				}

				//check if next buffer is free
				if (this->receiveFrameBuffersIsLocked[this->receiveBufferCounter] ){
					printf("WARNING: Now all package buffers are full, no room for the next package ... \r\n "); //todo
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

