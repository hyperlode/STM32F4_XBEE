#ifndef MENU_H
#define MENU_H

#define NUMBER_OF_ITEMS_MAX 16
#define MENU_ITEM_TEXT_MAX 64
#define MENU_OVERHEAD_TEXT_MAX 100

#define COMMAND_ARGUMENT_STRING_MAX_SIZE 64


#define MENU_SCREEN_TEXT_MAX NUMBER_OF_ITEMS_MAX * MENU_ITEM_TEXT_MAX + MENU_OVERHEAD_TEXT_MAX
#include <stdint.h>
#include <stdio.h>


enum argument_type {none, integerPositive, string };


struct command{
	uint8_t id=0;
	int32_t argument_int;
	char* argument_str;
};

class MenuItem{

public:
	MenuItem();
	void set(uint8_t id, char* text, uint8_t commandId, argument_type argumentType);
	char* getText();
	argument_type getArgument_type();
	void refresh();
	uint8_t getCommandId();

private:
	char* text;
	uint8_t id=0;
	uint8_t commandId =0;
	argument_type argumentType = none;
};

class Menu{
public:
	Menu();
	void init();
	void refresh();
	void display();
	void menuAsString(char* textHandle);
	command getPreparedCommand();
	bool commandWaitingToBeExecuted();
	void releaseMenu();
	//void addItem(MenuItem* item);
	void addItem(char* text, uint8_t commandId, argument_type argumentType);
	void userInput(char* input);

	bool isCharADigit(char c);
	int8_t convertCharToDigit(char c);


private:
	uint8_t chosenItemWaitingForArgument = 0;
	MenuItem items [NUMBER_OF_ITEMS_MAX];
	uint8_t numberOfItems=0;
	bool waitingForArgument = true;
	bool commandSelectedAndWaitingForRelease = 0;

	int32_t argumentInt = 0;
	char argumentString [COMMAND_ARGUMENT_STRING_MAX_SIZE];

	char* input;
	char output [MENU_SCREEN_TEXT_MAX];
	command chosenCommand;
};




#endif
