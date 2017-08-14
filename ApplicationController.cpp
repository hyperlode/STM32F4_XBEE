#include "ApplicationController.h"


ApplicationController::ApplicationController(){

}

void ApplicationController::init(uint32_t* millis){

	//
	initTestButton();
	initTestButton2();
	initTestButton3();


	waveShareIO.init(WAVESHARE_PORT);
	waveShareIO.initLeds();

	//waveShareIO.setLed(0,true);LED_BAILTRANSDUCER
	//waveShareIO.setLed(1,true);
	//waveShareIO.setLed(2,true);
	//waveShareIO.setLedBlinkPeriodMillis(3,500);
//	waveShareIO.setLed(3,true);

	//init radio
	this->millis  = millis;
	pRadio = &radio;
	radio.init(1,9600,millis); //https://www.digi.com/support/forum/51792/how-to-change-the-xbee-serial-baud-rate  (if you ever have to be able to reset the baud rate...)
	radio.setSleepPinPB9(); //configure sleep pin.

	//for (uint8_t i = 0; i<8;i++){
	//	radio.destinationXbee.address[i] = destinationAddress[i];
	//}

	//populate menu
	mainMenu.init();
	mainMenu.addItem("int test", testInt , integerPositive);
	mainMenu.addItem("string test", testStr, string);
	mainMenu.addItem("xbee get local address test", testXbeeGetLocalAddress, none);
	mainMenu.addItem("xbee STATS", xbeeStats, none);
	mainMenu.addItem("xbee toggle SLEEP", xbeeToggleSleep, none);

	mainMenu.addItem("xbee CLEAR BUFFERS", xbeeClearReceiveBuffers, none);

	mainMenu.addItem("xbee show NEIGHBOURS", xbeeGetRemotes, none);
	mainMenu.addItem("xbee AT + string arg (i.e. NI or NIname)", xbeeCustomAtCommand, string);
	mainMenu.addItem("xbee AT + int arg (i.e. SM or SM1)", xbeeCustomAtCommandConvertArgToInt, string);
	mainMenu.addItem("xbee RESET", xbeeReset, none);
	mainMenu.addItem("xbee set DESTINATION", xbeeSetDestination, integerPositive);
	mainMenu.addItem("xbee get DESTINATION", xbeeGetRemoteAddress, none);

	mainMenu.addItem("xbee send MESSAGE to destination", xbeeSendMessageToRemote, string);



	mainMenu.addItem("toggle send cyclic message to destination each x ms (0 to disable).", sendCyclicMessage, integerPositive);

	//specific for PTM
	mainMenu.addItem("PTM Define role BASE, BAIL or CROWD(from Xbee name)", ptmGetRoleFromXbeeName, none);
	mainMenu.addItem("PTM get destination xbee ", setPtmDestination, none);
	mainMenu.addItem("xbee send broadcast message ", xbeeSendMessageBroadcast, string);
}

void ApplicationController::ptmInit(){
	printf("init ptm: \r\n");
	command cmd;
	cmd.id = ptmGetRoleFromXbeeName;
	executeCommand(cmd);

}

void ApplicationController::interruptRefresh(){
	waveShareIO.refresh(*millis);

}

void ApplicationController::processReceivedPackage(){
	//analyse received data
	rxFrameData* rxData;
	rxData = radio.getRxPackage();

	printf("signal strength: %d\r\n", (*rxData).RSSI);
	printf("status (0:unicast, 1: broadcast): %d\r\n", (*rxData).isBroadcastMessage);
	//printf("last byte address: %02x\r\n", rxData->sourceAddress[7]);
	receivedPackageNotYetAnalysed = false;

	//if not yet found, look for signals
	//ptmRemotesOk


	if ((*rxData).isBroadcastMessage){
		//if broadcast, that means: init!
		bool baseStationCalling;
		baseStationCalling = generalFunctions::stringsAreEqual((char*)rxData->data, "BASE",4 );

		if (baseStationCalling){
			printf("Broadcast from base station received\r\n");
			if (ptmRole != baseStation){
				ptmSetDestination(baseStation, rxData->sourceAddress);
			}else{
				printf("This device IS a base station, neglect call from another base station...\r\n");
			}
		}else{
			printf("Non base station broadcasting...(msg neglected)\r\n");
		}
	}else{
		//specific message.
		printf("msg received (and neglected)\r\n");
	}
}

void ApplicationController::refresh(){

	//init ptm wait for xbee to start up.
	if (*this->millis > BOOT_TIME_XBEE_MILLIS && !xbeeBooted){
		xbeeBooted = true;
		ptmInit();

	}

	//execute command
	if (mainMenu.commandWaitingToBeExecuted()){
		command cmd;
		cmd = mainMenu.getPreparedCommand();
		executeCommand(cmd);
		mainMenu.releaseMenu();
	}

	//auto processing received packages xbee
	if (!receivedPackageNotYetAnalysed){
		//check and process for new received data
		this-> receivedPackageNotYetAnalysed = radio.processReceivedFrame();
	}else{
		processReceivedPackage();
	}

	//cyclic message sender xbee
	if (cyclicMessageEnabled && *this->millis > lastSentCyclicMessage_ms + cyclicMessagePeriod_ms){
		radio.sendMessageToDestination("Heyhoo",6,false,cyclicMessagePeriod_ms);
		lastSentCyclicMessage_ms = *this->millis;
	}

	//check keypress
	if(checkTestButtonPressed()){
		printf("button 1 pressed \r\n");
		command cmd;

		cmd.id = ptmGetRoleFromXbeeName;
		executeCommand(cmd);

		cmd.id = setPtmDestination;
		executeCommand(cmd);

	}

	//check keypress2
	if(checkTestButton2Pressed()){
		printf("button 2 pressed \r\n");
		command cmd;
		if (cyclicMessageEnabled){
			cmd.argument_int =0;
		}else{
			cmd.argument_int = 500;
		}

		cmd.id = sendCyclicMessage;

		cmd.argument_str = "button2";
		executeCommand(cmd);

	}

	//check keypress3
	if(checkTestButton3Pressed()){
		printf("button 3 pressed \r\n");

		command cmd;
		cmd.id = xbeeGetRemotes;
		//cmd.argument_str = "button2";
		executeCommand(cmd);

	//	command cmd;
		cmd.id = xbeeSetDestination;
		cmd.argument_int = 0; //set first found neighbour as destination.
		executeCommand(cmd);

	}
}

void ApplicationController::initTestButton(){
	// Initialize gpio pins with alternating function
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
}

bool ApplicationController::checkTestButtonPressed(){
	bool input = !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13);
	bool triggerPressed = false;

	if (!this->testButtonEdgeDetection && input){
		this->testButtonStartPress = *this->millis;
	}

	bool testButtonPressed_Debounced = input && (*this->millis > this->testButtonStartPress + BUTTON_PRESS_DELAY) ;

	if (testButtonPressed_Debounced && !(this->testButtonDebouncedEdgeDetection)){
		//button is pressed.
		printf("press\r\n");
		triggerPressed = true;
		this->testButtonDebouncedEdgeDetection = true;
	}
	this->testButtonDebouncedEdgeDetection = testButtonPressed_Debounced;
	this->testButtonEdgeDetection = input; //edge detector handling.
	return triggerPressed;

}



void ApplicationController::initTestButton2(){
	// Initialize gpio pins with alternating function
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
}

bool ApplicationController::checkTestButton2Pressed(){
	bool input = !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14);
	bool triggerPressed = false;

	if (!this->testButton2EdgeDetection && input){
		this->testButton2StartPress = *this->millis;
	}

	bool testButton2Pressed_Debounced = input && (*this->millis > this->testButton2StartPress + BUTTON_PRESS_DELAY) ;

	if (testButton2Pressed_Debounced && !(this->testButton2DebouncedEdgeDetection)){
		//button is pressed.
		printf("press2\r\n");
		triggerPressed = true;
		this->testButton2DebouncedEdgeDetection = true;
	}
	this->testButton2DebouncedEdgeDetection = testButton2Pressed_Debounced;
	this->testButton2EdgeDetection = input; //edge detector handling.
	return triggerPressed;

}


void ApplicationController::initTestButton3(){
	// Initialize gpio pins with alternating function
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
}

bool ApplicationController::checkTestButton3Pressed(){
	bool input = !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15);
	bool triggerPressed = false;

	if (!this->testButton3EdgeDetection && input){
		this->testButton3StartPress = *this->millis;
	}

	bool testButton3Pressed_Debounced = input && (*this->millis > this->testButton3StartPress + BUTTON_PRESS_DELAY) ;

	if (testButton3Pressed_Debounced && !(this->testButton3DebouncedEdgeDetection)){
		//button is pressed.
		printf("press3\r\n");
		triggerPressed = true;
		this->testButton3DebouncedEdgeDetection = true;
	}
	this->testButton3DebouncedEdgeDetection = testButton3Pressed_Debounced;
	this->testButton3EdgeDetection = input; //edge detector handling.
	return triggerPressed;

}

void ApplicationController::configureCyclicMessage(command command){
	//radio.displayNeighbours();
	 if(command.argument_int == 0){
		 this->cyclicMessageEnabled = false;
	 }else{
		 this->cyclicMessageEnabled = true;
	 }
	cyclicMessagePeriod_ms = command.argument_int;
}



void ApplicationController::executeCommand(command command){
	switch (command.id){

	case testInt:
		printf("Int test with arg: %d\r\n", command.argument_int);
		break;
	case testStr:
		printf("string test with arg: %s\r\n", command.argument_str);
		break;
	case testXbeeGetLocalAddress:
	{
		printf("xbee local address\r\n");
		printf("millis: %d\r\n", millis);
		bool success = radio.getLocalXbeeAddress(100);
		printf("address set(1 if success))?: %d",success);
		break;
	}
	case xbeeGetRemoteAddress:
		radio.getDestinationAddressFromXbee();
		break;
	case xbeeStats:
		radio.stats();
		printf("xbee local address\r\n");
		break;
	case xbeeToggleSleep:
	{
		if (radio.isSleeping()){
			radio.wakeUp();
		}else{
			radio.sleep();
		}
		break;
	}

	case xbeeGetRemotes:
	{
		radio.searchActiveRemoteXbees(8000);
		radio.displayNeighbours();
		break;
	}
	case xbeeCustomAtCommandConvertArgToInt:
	{
/*
		printf("tmp:\r\n");
		for (uint8_t i = 0; i<10;i++){
			printf(".%02x\r\n",command.argument_str[i]);
		}
*/
		uint16_t len = lengthOfString(command.argument_str, COMMAND_ARGUMENT_STRING_MAX_SIZE + 2, true);

		if (len<3){
			printf("user error: AT command is minimum two letters.\r\n");
			break;
		}
		uint32_t argInt = mainMenu.convertStringToPositiveInt(&command.argument_str[2], len);

		if (argInt == -1){
			printf("number invalid.: %08x\r\n",argInt);
		}else{
			printf("number: %08x\r\n",argInt);
			printf("number: %d\r\n",argInt);

			uint16_t atCmd = command.argument_str[0] <<8 | command.argument_str[1];
			printf("str: %04x\r\n",atCmd);
			if (len==3){
				printf("noArg\r\n",atCmd);
				radio.sendAtCommandAndAwaitWithResponse(atCmd, 1000);
			}else{
				printf("withArg\r\n",atCmd);
				if (radio.sendAtCommandAndAwaitWithResponse(atCmd,argInt,1000)){
					radio.saveChangesinLocalXbee();
				}
			}
		}
		break;

	}

	case xbeeCustomAtCommand:
	{
		//first two chars are the command, the rest is argument.
/*
		printf("tmp:\r\n");
		for (uint8_t i = 0; i<10;i++){
			printf(".%02x",command.argument_str[i]);
		}
/**/

		uint16_t len = lengthOfString(command.argument_str, COMMAND_ARGUMENT_STRING_MAX_SIZE + 2,true);

		if (len<3){
			printf("user error: AT command is minimum two letters.\r\n");
			break;
		}
		uint16_t atCmd = command.argument_str[0] <<8 | command.argument_str[1];
		if (len==3){
			//printf("WITHOUT arg");
			radio.sendAtCommandAndAwaitWithResponse(atCmd, 10000);
		}else{
			//printf("WITH arg");
			if (radio.sendAtCommandAndAwaitWithResponse(atCmd, (uint8_t*)(&command.argument_str[2]), len-3, 1000)){ //len-3 because '/0' will not be part of the sent parameter.
				radio.saveChangesinLocalXbee();
			}
		}
	}

	case xbeeClearReceiveBuffers:
	{
		radio.clearReceiveBuffers();
		break;
	}
	case xbeeSendMessageToRemote:
	{
		uint16_t len = lengthOfString(command.argument_str, COMMAND_ARGUMENT_STRING_MAX_SIZE, false);
		radio.sendMessageToDestination(command.argument_str,len,true,200);
		break;
	}
	case xbeeSendMessageToBaseStation:
	{

		if (remoteBaseStationData.isValid){
			uint16_t len = lengthOfString(command.argument_str, COMMAND_ARGUMENT_STRING_MAX_SIZE, false);

			radio.sendMessage( command.argument_str, remoteBaseStationData.address, len, false, 100);
		}else{
			printf("error: no base station configured.\r\n");

		}

	}
		break;

	case xbeeSendMessageBroadcast:
		{
			uint16_t len = lengthOfString(command.argument_str, COMMAND_ARGUMENT_STRING_MAX_SIZE, false);
			radio.sendMessageBroadcast(command.argument_str,len,true,200);
			break;
		}


	case xbeeReset:
		radio.reset();
		break;
	case xbeeSetDestination:
		radio.setNeighbourAsRemote(command.argument_int);
		break;

	case sendCyclicMessage:
	{
		configureCyclicMessage(command);

		break;
	}
	case ptmGetRoleFromXbeeName:
		if (!radio.sendAtCommandAndAwaitWithResponse(AT_NODE_IDENTIFIER_NI,100)){
			printf("ASSERT ERROR: command fail.\r\n");
		}

		//response of NI sets the local radio name
		//the name defines the role of the board.
		for (uint8_t i = 0;i<4;i++){
			waveShareIO.setLed(i,false);
			waveShareIO.setLedBlinkPeriodMillis(i,0);
		}


		if (generalFunctions::stringsAreEqual((char*)radio.senderXbee.name, "BAIL")){
			printf("PTM role set: Bail transducer\r\n");
			this->ptmRole = bailTransducer;
			mainMenu.addItem("bail/crowd: send to baseStation", xbeeSendMessageToBaseStation, string);
			waveShareIO.setLed(LED_BAILTRANSDUCER,true);
		}else if (generalFunctions::stringsAreEqual((char*)radio.senderXbee.name, "BASE")){
			printf("PTM role set: Bail Base Station\r\n");
			this->ptmRole = baseStation;
			waveShareIO.setLed(LED_BASESTATION,true);
		}else if (generalFunctions::stringsAreEqual((char*)radio.senderXbee.name, "CROWD")){
			printf("PTM role set: Crowd Transducer\r\n");
			this->ptmRole = crowdTransducer;
			waveShareIO.setLed(LED_CROWDTRANSDUCER,true);
		}else{
			printf("Name is not a valid PTM device: %s" ,radio.senderXbee.name );

		}

		break;

	case setPtmDestination:
		{
			//check for ptm destination. This is not a very nice way of working, better is to not send anything. Only wake up, and listen. This in order to conserve power.
			//if (!ptmSetDestination()){
				printf("could not set destination \r\n");
			//}

		}

		break;
	default:
		printf("ASSERT ERROR: no valid command.....\r\n");
		break;
	}
	printf("Command executed: %d \r\n", mainMenu.getMenuLineNumberOfCommand(command));
}


void ApplicationController::XbeeUartInterruptHandler(char c){
	radio.receiveInterruptHandler(c);
}

void ApplicationController::serialInput(char* input){

	//mainMenu.display(menuString);
	//printf("%s",menuString);
	mainMenu.userInput(input);
}

void ApplicationController::excecuteCommand(command command){
	printf("dat gaat hier goed, command id: %d",command.id);



}
bool ApplicationController::isBusy(){
	return isLocked;
}

/*
bool ApplicationController::ptmSetDestination(){

	if (!radio.searchActiveRemoteXbees(8000)){
		printf("no neighbours found\r\n");
		return false;
	}


	//set neighbour (search for the right one in the found neighbours list)
	if (this->ptmRole == baseStation){
		bool success = true;
		if (!ptmSetDestinationByRole(bailTransducer)){
			printf("no bail transducer found. \r\n");
			success = false;
		}
		if (!ptmSetDestinationByRole(crowdTransducer)){
			printf("no crowd Transducer found. \r\n");
			success = false;
		}

		return success;

	}else{
		//if not a base station, search for it!
		return ptmSetDestinationByRole( baseStation);

	}
}
*/

bool ApplicationController::ptmSetDestination(ptmRoles destinationToConnectWith, uint8_t* address){

	switch (destinationToConnectWith){
		case baseStation:

			remoteBaseStationData.isValid = true;
			generalFunctions::copyByteArray(address,remoteBaseStationData.address,  8);

			//set base station address as destination (in xbee)
			radio.setDestinationAddressInLocalXbee(remoteBaseStationData.address);



			//
			//for (uint8_t i = 0; i<8;i++){
			//	printf("last byte address: %02x\r\n", remoteBaseStationData.address[i]);
			//	//printf("last byte address: %02x\r\n", address[i]);
			//}
			ptmRemotesOk  = true;
			waveShareIO.setLedBlinkPeriodMillis(LED_BASESTATION,1000);
			waveShareIO.setLed(LED_BASESTATION,true);
			break;
		case bailTransducer:
			waveShareIO.setLedBlinkPeriodMillis(LED_BAILTRANSDUCER,1000);
			waveShareIO.setLed(LED_BAILTRANSDUCER,true);
			break;
		case crowdTransducer:
			waveShareIO.setLedBlinkPeriodMillis(LED_CROWDTRANSDUCER,1000);
			waveShareIO.setLed(LED_CROWDTRANSDUCER,true);
			break;
		default:
			break;

	}
	return ptmRemotesOk;

}/*

bool ApplicationController::ptmSetDestinationByRole(ptmRoles destinationToConnectWith){


	char* deviceName;
	if ( destinationToConnectWith == baseStation){
		deviceName = "BASE";
	}else if ( destinationToConnectWith == bailTransducer){
		deviceName = "BAIL";
	}else if ( destinationToConnectWith == crowdTransducer){
		deviceName = "CROWD";
	}else  {
		printf("ASSERT ERROR: destination device name.");

	}


	int8_t index = radio.getNeighbourIndexByName(deviceName,false);


	if (index<0){
		printf(" %s not found.(or more than 2).. \r\n", deviceName);
		return false;
	}else if (radio.setNeighbourAsRemote(index)){
		printf(" %sfound and connected, ready for sending.\r\n", deviceName);

		switch (destinationToConnectWith){
		case baseStation:
			waveShareIO.setLedBlinkPeriodMillis(LED_BASESTATION,1000);
			waveShareIO.setLed(LED_BASESTATION,true);
			break;
		case bailTransducer:
			waveShareIO.setLedBlinkPeriodMillis(LED_BAILTRANSDUCER,1000);
			waveShareIO.setLed(LED_BAILTRANSDUCER,true);
			break;
		case crowdTransducer:
			waveShareIO.setLedBlinkPeriodMillis(LED_CROWDTRANSDUCER,1000);
			waveShareIO.setLed(LED_CROWDTRANSDUCER,true);
			break;
		default:
			break;

		}

		return true;

	}else{
		printf("neighbour not set. \r\n");
		return false;
	}


}

*/


uint16_t ApplicationController::lengthOfString(char* string, uint16_t maxLength, bool includeStringDelimiter){

	// 0x00 equal '\0' , and is included in the char array itself.   lode\0   length=5.
	uint16_t length = 0;
	while (string[length]!= '\0' && length < maxLength-1){
		length++;
	}
	length++;
	if (length >= maxLength){
		printf("ASSERT ERROR: string not terminated or longer than allowed.");
	}
	//printf("lenlen: %d", - includeStringDelimiter);

	if (includeStringDelimiter){
		return length   ;
	}else{
		return length -1  ;
	}

}
