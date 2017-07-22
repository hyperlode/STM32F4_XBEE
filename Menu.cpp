#include "Menu.h"

//const char menuFooter [] = {'C', 'h', 'o', 'o','s','e', '.', '.', '.','\r','\n', '0x00'};
const char menuFooter [] = {'C', 'h', 'o', 'o','s','e', '.', '.', '.','\r','\n', '\0'}; //, '.','\r','\n',

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

Menu::Menu(){
	//add default items.
	this->waitingForArgument = true;

}

void Menu::addItem(char* text, uint8_t commandId, argument_type argumentType ){
	//MenuItem item;
	//item.set(text, 36);
	//items[this->numberOfItems] = item;

	//item.set(text, 36);
	this->items[this->numberOfItems].set(this->numberOfItems+1, text,commandId, argumentType );
	this->numberOfItems++;

}

void Menu::display(char* textHandle){
	int32_t index=0;
	for (uint8_t i = 0; i< this->numberOfItems;i++){

		textHandle[index++] = i + 48;
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

	textHandle[index++] = 0x00;

	//printf ("menucharslength: %d\r\n",index);
}


void Menu::userInput(char* input){
	if (!waitingForArgument){
		//first digit is the chosen menuItem
		uint8_t choice  = input[0]-48;
		if (choice > this->numberOfItems || input[0] < 48){
			printf("Invalid Choice...please chose a number between 0 and %d \r\n (your input: %s)", this->numberOfItems-1,input);
		}else{

			switch (items[choice].getArgument_type()){
				case none:
					printf("Menu %d activated \r\n", choice);
					break;
				case integer:
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
					printf("Invalid choice...");
					break;
			}
		}

	}else{
		//argument_type chosenArg = items[chosenItemWaitingForArgument].getArgument_type();
		switch(items[chosenItemWaitingForArgument].getArgument_type()){
		case integer:
			printf("int accepted");
			break;
		case string:
			printf("str accepted");
			break;
		default:
			break;
		}
		waitingForArgument = false;
	}

}

void Menu::init(){

}

void Menu::refresh(){

}
