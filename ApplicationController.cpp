#include "ApplicationController.h"


ApplicationController::ApplicationController(){

}
void ApplicationController::init(uint32_t* millis){


	//init radio

	pRadio = &radio;
	radio.init(1,9600,millis);


	//for (uint8_t i = 0; i<8;i++){
	//	radio.destinationXbee.address[i] = destinationAddress[i];
	//}

	//populate menu
	//mainMenu.init();
	//mainMenu.addItem("xbee STATS", xbeeStats, none);
	/*
	mainMenu.addItem("xbee Empty receive Buffers", xbeeClearReceiveBuffers , none);
	mainMenu.addItem("int test", testInt , integerPositive);
	mainMenu.addItem("string test", testStr, string);
	mainMenu.addItem("xbee get local address test", testXbeeGetLocalAddress, none);
	mainMenu.addItem("xbee process frames that arrived", xbeeProcess, none);

	mainMenu.addItem("tweede item", 2, integerPositive);
	mainMenu.addItem("tweede item", 2, integerPositive);
	mainMenu.addItem("tweede item", 2, integerPositive);
	mainMenu.addItem("tweede item", 2, integerPositive);
	mainMenu.addItem("tweede item", 2, integerPositive);
	mainMenu.addItem("tweede item", 2, integerPositive);
*/

}

void ApplicationController::refresh(){
	/*
	if (mainMenu.commandWaitingToBeExecuted()){
		command cmd;
		cmd = mainMenu.getPreparedCommand();
		executeCommand(cmd);
		mainMenu.releaseMenu();
	}
*/
	//XBEE auto process received frames.
	//radio.processReceivedFrame();


}
void ApplicationController::executeCommand(command command){
	switch (command.id){

	case testInt:
		printf("Int test with arg: %d\r\n", command.argument_int);
		break;
	case testStr:
		printf("string test with arg: %s\r\n", command.argument_str);
		break;
	case xbeeClearReceiveBuffers:
		radio.clearReceiveBuffers();
		break;

	case testXbeeGetLocalAddress:
	{
		printf("xbee local address\r\n");
		printf("millis: %d\r\n", millis);
		bool success = radio.setLocalXbeeAddress(20);
		printf("address set(1 if success))?: %d",success);


		break;
	}
	case xbeeProcess:
	{
		radio.processReceivedFrame();
		break;
	}
	case xbeeStats:
	{
		radio.stats();
		break;
	}

	default:
		printf("ASSERT ERROR: no valid command.....\r\n");
		break;
	}

	printf("executed commandId : %d \r\n", command.id);


}



void ApplicationController::XbeeUartInterruptHandler(char c){
	radio.receiveInterruptHandler(c);
}

void ApplicationController::serialInput(char* input){


	//mainMenu.userInput(input);
}

void ApplicationController::executeTestCommand(){
	printf("dat gaat hier goed, command id...\r\n");
	radio.stats();



}
bool ApplicationController::isBusy(){
	return isLocked;
}

