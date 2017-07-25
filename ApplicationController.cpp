#include "ApplicationController.h"


ApplicationController::ApplicationController(){

}
void ApplicationController::init(uint32_t* millis){


	//init radio

	pRadio = &radio;
	radio.init(1,9600,millis); //https://www.digi.com/support/forum/51792/how-to-change-the-xbee-serial-baud-rate  (if you ever have to be able to reset the baud rate...)


	//for (uint8_t i = 0; i<8;i++){
	//	radio.destinationXbee.address[i] = destinationAddress[i];
	//}

	//populate menu
	mainMenu.init();
	mainMenu.addItem("int test", testInt , integerPositive);
	mainMenu.addItem("string test", testStr, string);
	mainMenu.addItem("xbee get local address test", testXbeeGetLocalAddress, none);
	mainMenu.addItem("xbee STATS", xbeeStats, none);
	mainMenu.addItem("xbee PROCESS", xbeeProcess, none);
	mainMenu.addItem("xbee CLEAR BUFFERS", xbeeClearReceiveBuffers, none);
	mainMenu.addItem("xbee show NEIGHBOURS", xbeeGetRemotes, none);
	mainMenu.addItem("xbee AT custom", xbeeCustomAtCommand, string);
	mainMenu.addItem("xbee RESET", xbeeReset, none);
	mainMenu.addItem("xbee set DESTINATION", xbeeSetDestination, integerPositive);
	mainMenu.addItem("xbee get DESTINATION", xbeeGetRemoteAddress, none);

	mainMenu.addItem("test", testCmd, none);


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
		radio.getDestinationFromXbee();
		break;
	case xbeeStats:
		radio.stats();
		printf("xbee local address\r\n");
		break;
	case xbeeProcess:
		{
			radio.processReceivedFrame();
			break;
		}

	case xbeeGetRemotes:
			{
				radio.searchActiveRemoteXbees(10000);
				radio.displayNeighbours();
				break;
			}

	case xbeeCustomAtCommand:
	{
			uint16_t atCmd = command.argument_str[0] <<8 | command.argument_str[1];

			radio.sendAtCommandAndAwaitWithResponse(atCmd, 1000);
	}
	case xbeeClearReceiveBuffers:
			{
				radio.clearReceiveBuffers();
				break;
			}
	case xbeeReset:
		radio.reset();
		break;
	case xbeeSetDestination:
		radio.setNeighbourAsRemote(command.argument_int);
		break;
	case testCmd:
		radio.displayNeighbours();
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

