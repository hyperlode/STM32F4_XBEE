#ifndef APPLICATIONCONTROLLER_H
#define APPLICATIONCONTROLLER_H

#include <stdint.h>
#include <stdio.h>

#include "Menu.h"
#include "XBEE.h"

enum commandIds {testInt, testStr,testXbeeGetLocalAddress,xbeeProcess, xbeeStats, xbeeClearReceiveBuffers};




class ApplicationController{

public:
	ApplicationController();
	void init(uint32_t* millis);
	void refresh();
	void excecuteCommand(command command);
	bool isBusy();

	void displayAndResetMenu();

	void serialInput(char* input);
	void executeCommand(command command);

	void XbeeUartInterruptHandler(char c);
private:

	bool isLocked = false;
	char menuString [MENU_SCREEN_TEXT_MAX];
	Menu mainMenu;
	XBEE radio;
	XBEE* pRadio;
	//uint8_t destinationAddress [] = {0x00, 0x13, 0xA2, 0x00, 0x41, 0x05, 0xBC, 0x87};
	uint32_t* millis;

};

#endif
