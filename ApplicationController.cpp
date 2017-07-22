#include "ApplicationController.h"

ApplicationController::ApplicationController(){

}
void ApplicationController::init(){
	//populate menu
	mainMenu.init();
	mainMenu.addItem("checketuut", 1 , none);
	mainMenu.addItem("tweede item", 2, integer);


}

void ApplicationController::refresh(){



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

