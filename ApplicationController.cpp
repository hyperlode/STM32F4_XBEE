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
	mainMenu.init();
	mainMenu.addItem("int test", testInt , integerPositive);
	mainMenu.addItem("string test", testStr, string);
	mainMenu.addItem("xbee get local address test", testXbeeGetLocalAddress, none);
	mainMenu.addItem("xbee STATS", xbeeStats, integerPositive);
	mainMenu.addItem("vijf item", 5, integerPositive);
	mainMenu.addItem("tweede item", 2, integerPositive);
	mainMenu.addItem("tweede item", 2, integerPositive);
	mainMenu.addItem("tweede item", 2, integerPositive);
	mainMenu.addItem("tweede item", 2, integerPositive);
	mainMenu.addItem("tweede item", 2, integerPositive);
	mainMenu.addItem("tweede item", 2, integerPositive);


}

void ApplicationController::refresh(){
	if (mainMenu.commandWaitingToBeExecuted()){
		command hoitjes;
		hoitjes = mainMenu.getPreparedCommand();
		executeCommand(hoitjes);
		mainMenu.releaseMenu();
	}



	radio.processReceivedFrame();


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
		printf("xbee local address\r\n");
		break;
	case xbeeStats:
			radio.stats();
			printf("xbee local address\r\n");
			break;

	default:
		printf("ASSERT ERROR: no valid command.....\r\n");
		break;
	}

	printf("command executed: %d \r\n", command.id);


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

