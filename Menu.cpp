#include "Menu.h"

const char menuFooter [] = "Choose....";

MenuItem::MenuItem(){

}

void MenuItem::set(uint8_t id, char* text, uint8_t commandId, argument_type argumentType){
	this->text = text;
	this->id = id;
	this->argumentType = argumentType;
	this->commandId = commandId;

}
char* MenuItem::getText(){

	return text;
}

argument_type MenuItem::getArgument_type(){

	return this->argumentType;
}
uint8_t MenuItem::getCommandId(){
	return this->commandId;
}

//**************************************************************
Menu::Menu(){
	//add default items.
	this->waitingForArgument = true;

}

bool Menu::commandWaitingToBeExecuted(){
	return commandSelectedAndWaitingForRelease;
}

command Menu::getPreparedCommand(){

	if (commandSelectedAndWaitingForRelease){
		return chosenCommand;

	}else{
		printf("ASSERT ERROR: no command selected.");
	}
}

void Menu::releaseMenu(){
	//when command is executed, release the menu.
	commandSelectedAndWaitingForRelease = false;

}

void Menu::addItem(char* text, uint8_t commandId, argument_type argumentType ){
	//MenuItem item;
	//item.set(text, 36);
	//items[this->numberOfItems] = item;

	//item.set(text, 36);
	this->items[this->numberOfItems].set(this->numberOfItems+1, text,commandId, argumentType );
	this->numberOfItems++;

}

void Menu::display(){
	//char output [10000]; //if 1000, errors after a while (hardFault)
	menuAsString(output);
	uint16_t i=0;
	printf("  ");
	while (output[i]!= '\0' && output[i+1]!= '\0' && i< 10000){
		printf("%c%c",output[i++],output[i++]); //of course outputting one char would make more sense, but there are problems: ///http://www.coocox.org/forum/viewtopic.php?f=2&t=4143&start=10
	}
}

void Menu::menuAsString(char* textHandle){
	int32_t index=0;
	for (uint8_t i = 0; i< this->numberOfItems;i++){

		if (this->numberOfItems>9){
			if (i<10){
				textHandle[index++] = ' ';
				textHandle[index++] = i + 48;
			}else{
				textHandle[index++] = i/10 + 48;
				textHandle[index++] = i%10 + 48;
			}

		}else{
			textHandle[index++] = i + 48;
		}
		textHandle[index++] = ':';
		textHandle[index++] = ' ';

		uint8_t j = 0;
		while (this->items[i].getText()[j] != 0x00 && j < MENU_ITEM_TEXT_MAX ){
			textHandle[index] =  this->items[i].getText()[j];
			index++;
			j++;
		}

		textHandle[index++] = '\r';
		textHandle[index++] = '\n';

//		textHandle[index++] = 0x0A;
	}
	textHandle[index++] = ' ';
	uint8_t i = 0;
	while (menuFooter[i] != '\0' ){
		textHandle[index++] =  menuFooter[i];
		//printf (".%c",menuFooter[i]);
		i++;

	}
	textHandle[index++] = ' ';//nonsense space to deal with the putchar error....
	textHandle[index++] = 0x00;
	textHandle[index++] = 0x00;
	//textHandle[index++] = '\0';

	//printf ("menucharslength: %d\r\n",index);
}

void Menu::prepareCommandAndLock(uint8_t commandId, int32_t intArg, char* strArg){
	chosenCommand.id = commandId;
	chosenCommand.argument_int = intArg;
	chosenCommand.argument_str = strArg;

	commandSelectedAndWaitingForRelease = true;
	//printf("debug: str arg: \"%s\"\r\n",chosenCommand.argument_str);
}


void Menu::userInput(char* input){
	if(commandSelectedAndWaitingForRelease){
		if (input[0] == 'd' ){
			releaseMenu();
			printf("available for commands.\r\n");
		}else{
			printf("busy... (press d to re enter command)\r\n");
		}
	}else if (!waitingForArgument){
		//first digit is the chosen menuItem


		if (input[0] == 'm' ){
			//if character m
			display();
			return;
		}

		int32_t  choice = convertStringToPositiveInt(input, 3);

		if (choice >= this->numberOfItems || choice<0){
			printf("Invalid Choice...please choose a number between 0 and %d \r\n (your input was: %s)\r\n", this->numberOfItems-1,input);
			printf("Alternatively press m to display the menu.\r\n");
		}else {
			switch (items[choice].getArgument_type()){
				case none:
					printf("No argument asked. \r\n", choice);
					prepareCommandAndLock(this->items[choice].getCommandId(),-1,"");
					break;
				case integerPositive:
					printf("Input a number:\r\n");
					chosenItemWaitingForArgument = choice;
					waitingForArgument = true;
					break;
				case string:
					printf("Input a string:\r\n");
					chosenItemWaitingForArgument = choice;
					waitingForArgument = true;
					break;
				default:
					printf("ASSERT ERROR: Invalid argument type...\r\n");
					break;
			}
		}

	}else{
		switch(items[chosenItemWaitingForArgument].getArgument_type()){
		case integerPositive:
		{
			int32_t value = 0;
			value = convertStringToPositiveInt(input, COMMAND_ARGUMENT_STRING_MAX_SIZE  );

			if (value<0){
				printf("value not accepted, please try again. (positive integer) \r\n", value);
			}else{
				printf("int accepted: %d \r\n", value);
				waitingForArgument = false;
				prepareCommandAndLock(this->items[chosenItemWaitingForArgument].getCommandId(),value,"")  ;
			}
			break;
		}
		case string:
		{
			int16_t j = 0;
			while( !(input[j] == '\0') && j< COMMAND_ARGUMENT_STRING_MAX_SIZE-1  ){
				argumentString[j] = input[j];
				j++;
			}
			argumentString[++j] = '\0'; //terminate string

			if(j>= COMMAND_ARGUMENT_STRING_MAX_SIZE){
				printf("string too long, max %d chars", COMMAND_ARGUMENT_STRING_MAX_SIZE);
			}else{

				//chosenCommand.argument_str  = argumentString;
				//printf("str accepted: \"%s\"\r\n",chosenCommand.argument_str);
				waitingForArgument = false;
				prepareCommandAndLock(this->items[chosenItemWaitingForArgument].getCommandId(),-1,argumentString)  ;
			}
			break;
		}
		default:
			break;
		}

	}

}

bool Menu::isCharADigit(char c){
	return (c>=48 && c< 58);
}

int8_t Menu::convertCharToDigit(char c){
	if (isCharADigit(c)){
		return c - 48;
	}else{
		return -1;
	}

}

int32_t Menu::convertStringToPositiveInt(char* input, uint16_t maxLength){

	int32_t value = 0;
	int16_t j = 0;
	while( !(input[j] == '\r' ||input[j] == '\n' ||input[j] == '\0') && j< maxLength  ){
		if (isCharADigit(input[j])){
			value = 10*value +  convertCharToDigit(input[j]);
		}else{

			return -1;
		}
		j++;
		//	printf("v:%d ... %02x \r\n",value,input[j]);
	}
	if (j==maxLength){
		//in reality, index is larger then maxlength
		return -1;
	}
	return value;

}


void Menu::init(){

}

void Menu::refresh(){

}
