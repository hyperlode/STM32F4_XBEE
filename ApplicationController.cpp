#include "ApplicationController.h"

ApplicationController::ApplicationController(){

}
void ApplicationController::init(){
	//populate menu






	mainMenu.init();
	mainMenu.addItem("no args", test , none);
	mainMenu.addItem("number command", printNonsense, integer);
	mainMenu.addItem("dre item string", 3, string);
	mainMenu.addItem("vier item", 4, integer);
	mainMenu.addItem("vijf item", 5, integer);
	mainMenu.addItem("tweede item", 2, integer);
	mainMenu.addItem("tweede item", 2, integer);
	mainMenu.addItem("tweede item", 2, integer);
	mainMenu.addItem("tweede item", 2, integer);
	mainMenu.addItem("tweede item", 2, integer);
	mainMenu.addItem("tweede item", 2, integer);


}

void ApplicationController::refresh(){
	if (mainMenu.commandWaitingToBeExecuted()){
		command hoitjes;
		hoitjes = mainMenu.getPreparedCommand();
		printf("command executed: %d \r\n", hoitjes.id);
		mainMenu.releaseMenu();
	}


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

