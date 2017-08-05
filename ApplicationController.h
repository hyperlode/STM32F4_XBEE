#ifndef APPLICATIONCONTROLLER_H
#define APPLICATIONCONTROLLER_H

#include <stdint.h>
#include <stdio.h>

#include "Menu.h"
#include "XBEE.h"
#include "stm32f4xx_gpio.h"

#include "IOBoard.h"
#include "generalFunctions.h"

#define BUTTON_PRESS_DELAY 50

#define LED_BASESTATION 0
#define LED_BAILTRANSDUCER 1
#define LED_CROWDTRANSDUCER 2

enum ptmRoles {
	baseStation,
	bailTransducer,
	crowdTransducer,
	pinTransducer,
	baseStationPortable

};

enum commandIds {testInt,
	testStr,
	testXbeeGetLocalAddress,
	xbeeToggleSleep,
	xbeeStats,
	xbeeClearReceiveBuffers,
	xbeeGetRemotes,
	sendCyclicMessage,
	xbeeCustomAtCommand,
	xbeeCustomAtCommandConvertArgToInt,
	xbeeReset,
	xbeeSetDestination,
	xbeeGetRemoteAddress,
	xbeeSendMessageToRemote,
	ptmGetRoleFromXbeeName,

	setPtmDestination
};


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

	uint16_t lengthOfString(char* string, uint16_t maxLength, bool includeStringDelimiter);



	bool ptmSetDestinationByRole(ptmRoles destinationToConnectWith);
	bool ptmSetDestination();


	bool checkTestButtonPressed();
	void initTestButton();
	bool checkTestButton2Pressed();
	void initTestButton2();
	bool checkTestButton3Pressed();
	void initTestButton3();

	void configureCyclicMessage(command command);

private:
	bool cyclicMessageEnabled = false;
	uint32_t cyclicMessagePeriod_ms = 0;
	uint32_t lastSentCyclicMessage_ms = 0;
	bool isLocked = false;
	char menuString [MENU_SCREEN_TEXT_MAX];
	Menu mainMenu;
	XBEE radio;
	XBEE* pRadio;


	IOBoard waveShareIO;
	//uint8_t destinationAddress [] = {0x00, 0x13, 0xA2, 0x00, 0x41, 0x05, 0xBC, 0x87};
	uint32_t* millis;

	bool testButtonEdgeDetection = false;
	bool testButtonDebouncedEdgeDetection = false;
	uint32_t testButtonStartPress;

	bool testButton2EdgeDetection = false;
	bool testButton2DebouncedEdgeDetection = false;
	uint32_t testButton2StartPress;

	bool testButton3EdgeDetection = false;
	bool testButton3DebouncedEdgeDetection = false;
	uint32_t testButton3StartPress;

	ptmRoles ptmRole;


};

#endif
