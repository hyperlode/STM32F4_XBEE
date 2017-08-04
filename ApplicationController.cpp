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

	waveShareIO.setLed(0,true);
	waveShareIO.setLed(1,true);
	waveShareIO.setLed(2,true);
	waveShareIO.setLedBlinkPeriodMillis(3,500);
	waveShareIO.setLed(3,true);

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
}

void ApplicationController::refresh(){

	waveShareIO.refresh(*millis);

	if (mainMenu.commandWaitingToBeExecuted()){
		command cmd;
		cmd = mainMenu.getPreparedCommand();
		executeCommand(cmd);
		mainMenu.releaseMenu();
	}


	//auto processing:
	radio.processReceivedFrame();
	if (cyclicMessageEnabled && *this->millis > lastSentCyclicMessage_ms + cyclicMessagePeriod_ms){

		radio.sendMessageToDestination("Heyhoo",6,false,cyclicMessagePeriod_ms);

		lastSentCyclicMessage_ms = *this->millis;

	}

	//check keypress
	if(checkTestButtonPressed()){
		printf("button 1 pressed \r\n");
		command cmd;
		cmd.id = xbeeSendMessageToRemote;
		cmd.argument_str = "button1";
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
				radio.searchActiveRemoteXbees(5000);
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
		radio.sendMessageToDestinationAwaitResponse(command.argument_str,len,200);
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
