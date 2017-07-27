#include "ApplicationController.h"


ApplicationController::ApplicationController(){

}
void ApplicationController::init(uint32_t* millis){


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

	mainMenu.addItem("toggle send cyclic message to destination each x ms.", sendCyclicMessage, integerPositive);
}

void ApplicationController::refresh(){
	if (mainMenu.commandWaitingToBeExecuted()){
		command cmd;
		cmd = mainMenu.getPreparedCommand();
		executeCommand(cmd);
		mainMenu.releaseMenu();
	}


	//auto processing:
	radio.processReceivedFrame();
	if (cyclicMessageEnabled && *this->millis > lastSentCyclicMessage_ms + cyclicMessagePeriod_ms){

		radio.sendMessageToDestination("Heyhoo",6,false,200);

		lastSentCyclicMessage_ms = *this->millis;

	}


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
				radio.searchActiveRemoteXbees(10000);
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
		uint16_t len = lengthOfString(command.argument_str, COMMAND_ARGUMENT_STRING_MAX_SIZE + 2);

		if (len<3){
			printf("user error: AT command is minimum two letters.\r\n");
			break;
		}
		uint32_t argInt = mainMenu.convertStringToPositiveInt(&command.argument_str[2], len);

		if (argInt == -1){
			printf("number invalid.: %08x",argInt);
		}else{

			uint16_t atCmd = command.argument_str[0] <<8 | command.argument_str[1];
			if (len==3){
				radio.sendAtCommandAndAwaitWithResponse(atCmd, 1000);
			}else{
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

		uint16_t len = lengthOfString(command.argument_str, COMMAND_ARGUMENT_STRING_MAX_SIZE + 2);

		if (len<3){
			printf("user error: AT command is minimum two letters.\r\n");
			break;
		}
		uint16_t atCmd = command.argument_str[0] <<8 | command.argument_str[1];
		if (len==3){
			//printf("WITHOUT arg");
			radio.sendAtCommandAndAwaitWithResponse(atCmd, 1000);
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
		uint16_t len = lengthOfString(command.argument_str, COMMAND_ARGUMENT_STRING_MAX_SIZE);
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
		//radio.displayNeighbours();
		this->cyclicMessageEnabled = !this->cyclicMessageEnabled; //toggle
		cyclicMessagePeriod_ms = command.argument_int;

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

uint16_t ApplicationController::lengthOfString(char* string, uint16_t maxLength){
	// 0x00 equal '\0' , and is included in the char array itself.   lode\0   length=5.
	uint16_t length = 0;
	while (string[length]!= '\0' && length < maxLength-1){
		length++;
	}
	length++;
	if (length >= maxLength){
		printf("ASSERT ERROR: string not terminated or longer than allowed.");
	}
	return length;

}
