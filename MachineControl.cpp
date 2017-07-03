#include "MachineControl.h"

MachineControl::MachineControl(){


	//panel 1 (we implement panel 1 for the potentio meters)
	panel1.init(PANEL_1);
	panel1.initADC(); //GPIOC 0,1,2,3
	IOBoardHandler[0] = &panel1; //link the panel instance to the handler.


	//panel4
	panel4.init(PANEL_4);
	panel4.initLeds();
	panel4.initButtons();
	IOBoardHandler[3] = &panel4; //link the panel instance to the handler.


	//add external zeroing button
	initExternalZeroingButton();

	for (uint16_t i = 0;i<16;i++){

		//panel4.setLed(i,true);
		panel4.setLed(i,false);
	}

	//set up the desired sequence.
	panel4.ledSequenceUser_set(0,3);
	panel4.ledSequenceUser_set(1,2);
	panel4.ledSequenceUser_set(2,1);

	//panel4.ledSequenceInterruptHandler(false);

	//ADC
	adcRanges[0]=ADC_MOTOR_HOIST_MAX_VALUE;
	adcRanges[1]=ADC_MOTOR_CROWD_MAX_VALUE;
	adcRanges[2]=ADC_MOTOR_SWING_MAX_VALUE;

	adcZeroSpeedRawValues[0]=ADC_MOTOR_HOIST_ZERO_SPEED_VALUE;
	adcZeroSpeedRawValues[1]=ADC_MOTOR_CROWD_ZERO_SPEED_VALUE;
	adcZeroSpeedRawValues[2]=ADC_MOTOR_SWING_ZERO_SPEED_VALUE;

	//DAC --> control the MAXON motor boards
	dacSpeedControl_Hoist.init(1);
	DacHandlerPointers[0] = &dacSpeedControl_Hoist;

	dacSpeedControl_Crowd.init(2);
	DacHandlerPointers[1] = &dacSpeedControl_Crowd;

	dacSpeedControl_Swing.init(3);
	DacHandlerPointers[2] = &dacSpeedControl_Swing;

	dacRanges[0]= DAC_MOTOR_HOIST_MAX_VALUE;
	dacZeroSpeedValues[0] = DAC_MOTOR_HOIST_ZERO_SPEED_VALUE;
	dacSpeedControl_Hoist.assignValue(dacZeroSpeedValues[0]);

	dacRanges[1]= DAC_MOTOR_CROWD_MAX_VALUE;
	dacZeroSpeedValues[1] = DAC_MOTOR_CROWD_ZERO_SPEED_VALUE;
	dacSpeedControl_Crowd.assignValue(dacZeroSpeedValues[1]);

	dacRanges[2]= DAC_MOTOR_SWING_MAX_VALUE;
	dacZeroSpeedValues [2] = DAC_MOTOR_SWING_ZERO_SPEED_VALUE;
	dacSpeedControl_Crowd.assignValue(dacZeroSpeedValues[2]);
	dacValues[2] = 0;

/*
	//encoders
	//motor1
	setUpHardWareInterrupt_motor1_channelA();
	setUpInputPin_motor1_channelB();

	//motor2
	setUpHardWareInterrupt_motor2_channelA();
	setUpInputPin_motor2_channelB();

	//motor3
	setUpHardWareInterrupt_motor3_channelA();
	setUpInputPin_motor3_channelB();
*/

	//axis control
	//motor1 hoist
	motor1.init(1);
	MotorControlHandles[0] = &motor1;

	//motor2 crowd
	motor2.init(2);
	MotorControlHandles[1] = &motor2;

	//motor 3 swing
	motor3.init(3);
	MotorControlHandles[2] = &motor3;

	//INIT mode for motorcontrollermode
	motorControllerMode = MODE_NORMAL;
	panel4.setLed(LED_MOTORCONTROLLER_MODE,true);

	for (uint8_t i=0; i<NUMBER_OF_MOTORS;i++){
		MotorControlHandles[i]->setMode(motorControllerMode);
	}

	motor1.setLimit(POSITION_HOIST_LIMIT_MIN_DEFAULT,true);
	motor1.setLimit(POSITION_HOIST_LIMIT_MAX_DEFAULT,false);
	motor2.setLimit(POSITION_CROWD_LIMIT_MIN_DEFAULT,true);
	motor2.setLimit(POSITION_CROWD_LIMIT_MAX_DEFAULT,false);
	motor3.setLimit(POSITION_SWING_LIMIT_MIN_DEFAULT,true);
	motor3.setLimit(POSITION_SWING_LIMIT_MAX_DEFAULT,false);

	selectNextLimitToBeCalibrated();


	initEncoders();

#ifdef USE_VCP
	printf("Userinterface: \r\n");
	printf("To interact. Please send v, s, m or a  \r\n");
#endif
	initExtraButton();

}

void MachineControl::initEncoders(){

	encoder1.init(ENCODER_1);
	EncoderToTimerHandles[0] = &encoder1;
	encoder2.init(ENCODER_2);
	EncoderToTimerHandles[1] = &encoder2;
	encoder3.init(ENCODER_3);
	EncoderToTimerHandles[2] = &encoder3;
}

void MachineControl::speedInputADCInterrupt(uint16_t potentioNumber, uint16_t value){
	IOBoardHandler[0]->ADCInterruptHandler(potentioNumber, value);

}

int32_t MachineControl::rescaleValueToDifferentRange(int32_t value, int32_t minIn , int32_t maxIn, int32_t minOut, int32_t maxOut){
	//http://stackoverflow.com/questions/5294955/how-to-scale-down-a-range-of-numbers-with-a-known-min-and-max-value
	//linear rescaling of ranges
	return (((maxOut - minOut)* (value - minIn)) / (maxIn - minIn) ) + minOut;


}

void MachineControl::refresh(uint32_t millis){
/**/
		this-> millis = millis;

		panel4.refresh(millis);
		panel1.refresh(millis);


		//select mode with button4 on panel

		//if (panel4.getButtonState(BUTTON_MOTORCONTROLLER_SELECT_MODE)){
			//while button is pressed, led goes off(as feedback)
		//	panel4.setLed(LED_MOTORCONTROLLER_MODE,false);
		//}

		if (panel4.getButtonEdgeDePressed(BUTTON_MOTORCONTROLLER_SELECT_MODE)){
			if (getMotorsZeroedSinceStartup()){ //no way we will do calibration if the motors are not zeroed.
				motorControllerMode++;
				if (motorControllerMode>=NUMBER_OF_MODES){
					motorControllerMode  = 0;
				}

				switch (motorControllerMode){
					case MODE_NORMAL:
						panel4.setLed(LED_MOTORCONTROLLER_MODE,true);
						panel4.setLedBlinkPeriodMillis(LED_MOTORCONTROLLER_MODE,0);
						break;
					//case MODE_TEST:
					//	panel4.setLed(LED_MOTORCONTROLLER_MODE,true);
					//	panel4.setLedBlinkPeriodMillis(LED_MOTORCONTROLLER_MODE,1000);
					//	break;
					case MODE_CALIBRATE:
						panel4.setLed(LED_MOTORCONTROLLER_MODE,true);
						panel4.setLedBlinkPeriodMillis(LED_MOTORCONTROLLER_MODE,250);
						break;
					default:
						panel4.setLed(LED_MOTORCONTROLLER_MODE,false);
						break;
				}

				//update the motors
				for (uint8_t i=0; i<NUMBER_OF_MOTORS;i++){
					MotorControlHandles[i]->setMode(motorControllerMode);
				}
			}
		}

		//button pressed actions (depending on mode)
		switch (motorControllerMode){
			case MODE_NORMAL:

				if (panel4.getButtonEdgePressed(BUTTON_ZEROING_ALL_AXIS)){
					this->zeroingButtonPressStartTime = this->millis;
				}

				if ( panel4.getButtonState(BUTTON_ZEROING_ALL_AXIS) &&  this->millis - this->zeroingButtonPressStartTime > ZEROING_BUTTON_TIME_DELAY_MILLIS ){
					setAllMotorPositionsToZero();



				}

				break;
			//case MODE_TEST:
			//	break;
			case MODE_CALIBRATE:
				//if calibration selected select limit and set limit buttons active
				if (panel4.getButtonEdgePressed(BUTTON_MOTORCONTROLLER_SELECT_LIMIT_FOR_SETTING)){
					//select limit to configure
					selectNextLimitToBeCalibrated();
				}
				if (panel4.getButtonEdgePressed(BUTTON_MOTORCONTROLLER_SET_SELECTED_LIMIT_TO_CURRENT_POSITION)){
					MotorControlHandles[activeMotorForTestingOrCalibration]->setCurrentPositionAsLimit();
				}
				if (panel4.getButtonEdgePressed(BUTTON_MOTORCONTROLLER_RESET_ALL_LIMITS)){
					MotorControlHandles[activeMotorForTestingOrCalibration]->resetLimit();
				}
				break;
			default:
				panel4.setLed(LED_MOTORCONTROLLER_MODE,false);
				break;
		}

		//adc speed input potentio meters (joystick)
		if (millis - millisMemory_adcProcess >= REFRESH_DELAY_MILLIS_ADC){
			this->millisMemory_adcProcess = millis;

			for (uint8_t i=0; i<NUMBER_OF_MOTORS;i++){
				int32_t adcRaw = panel1.getSliderValue(i); //joystick input mimics panel1... (0 to 4095) -> from 0->5V

				int32_t adcCorrected =adcRaw - this->adcZeroSpeedRawValues[i];

				int32_t range = 0;
				int32_t correctedValueConsideringDeadBeat = 0;
				int8_t joyStickSpeedPercentage = 0;

				if (adcCorrected > ADC_MOTOR_ZERO_SPEED_OFFSET_AROUND_CENTER){
					//positive speed
					range = adcRanges[i] - this->adcZeroSpeedRawValues[i] ; //raw range
					correctedValueConsideringDeadBeat = rescaleValueToDifferentRange(adcCorrected ,
							ADC_MOTOR_ZERO_SPEED_OFFSET_AROUND_CENTER,
							range,
							0,
							range
							);
					if (correctedValueConsideringDeadBeat > range){
						printf("positive range  %d", correctedValueConsideringDeadBeat);
						correctedValueConsideringDeadBeat = range;

					}

					joyStickSpeedPercentage = (correctedValueConsideringDeadBeat* 100) / range;
					//int32_t rescaleValueToDifferentRange(int32_t value, int32_t minIn , int32_t maxIn, int32_t minOut, int32_t maxOut){
							//http://stackoverflow.com/questions/5294955/how-to-scale-down-a-range-of-numbers-with-a-known-min-and-max-value
				}else if (adcCorrected < -ADC_MOTOR_ZERO_SPEED_OFFSET_AROUND_CENTER){
					//negative speed
					range = this->adcZeroSpeedRawValues[i];

					//adc corrected is a negative value. from
					correctedValueConsideringDeadBeat = rescaleValueToDifferentRange(adcCorrected ,
					-range,
					 -ADC_MOTOR_ZERO_SPEED_OFFSET_AROUND_CENTER,
					-range,
					0
					);
					if (correctedValueConsideringDeadBeat < -range){
						printf("negative range corrected: %d \r\n value in: %d \r\n range: %d \r\n  ", correctedValueConsideringDeadBeat,adcCorrected,range);
						correctedValueConsideringDeadBeat = -range;

					}
					joyStickSpeedPercentage = (correctedValueConsideringDeadBeat* 100) / range;

				}else{
					//zero speed
					joyStickSpeedPercentage = 0;

				}


				//set range without deadbeat
				/*
				int32_t range = this->adcZeroSpeedRawValues[i]; //if negative range from 0 to zero speed point
				if (adcCorrected >0 ){
					range = adcRanges[i] - range;  //if positive range from  zero speed point to maximum.
				}

				int8_t joyStickSpeedPercentage = (adcCorrected* 100) / range; //percentage  = value/ range *100
				*/
				
				MotorControlHandles[i]->setSpeedPercentageDesired(joyStickSpeedPercentage);
			}
		}

		//dac speed output
		if (millis - millisMemory_dacProcess >= REFRESH_DELAY_MILLIS_DAC){
			this->millisMemory_dacProcess = millis; //edge control
/*
			//printf("dac3: %d", dacValues[2]);
			dacValues[2] += 1;
			if (dacValues[2]> 255){
				dacValues[2] = 0;
			}

			DacHandlerPointers[2]->assignValue(dacValues[2]);
*/
			for (uint8_t i=0; i<NUMBER_OF_MOTORS;i++){
				int32_t interval; //interval is the plus or minus range.
				if (MotorControlHandles[i]->getSpeedPercentageChecked() >0 ){

					interval = dacRanges[i] - dacZeroSpeedValues[i];
				}else{
					interval = dacZeroSpeedValues[i];
				}

				//rangeValue = range * percentage /100;
				dacValues[i]  = dacZeroSpeedValues[i] + ((interval * MotorControlHandles[i]->getSpeedPercentageChecked())/100) ;
				DacHandlerPointers[i]->assignValue(dacValues[i]);
			}
		}


		//process encoder values from timers
		if (millis - millisMemory_encoderProcess >= REFRESH_DELAY_MILLIS_ENCODERS){
			this->millisMemory_encoderProcess = millis; //edge control
			encoder1.refresh();
			motor1.updatePosition(encoder1.getValue());
			encoder2.refresh();
			motor2.updatePosition(encoder2.getValue());
			encoder3.refresh();
			motor3.updatePosition(encoder3.getValue());

		}



		//refresh motor status lights
		if (millis - millisMemory_statusLights >= REFRESH_DELAY_MILLIS_STATUSLIGHTS){
			this->millisMemory_statusLights = millis; //edge control

			//dacTest.initDAC1();
			//dacTest.triggerDAC1(millis%4000);
			//update leds for all motors.
			//for (uint8_t i=0; i<NUMBER_OF_MOTORS;i++){
			//		panel4.setLed(LED_MOTOR_HOIST_LIMIT_MIN,MotorControlHandles[i]->getStatusLed(LED_LIMIT_MIN,millis));
			//		panel4.setLed(LED_MOTOR_HOIST_INRANGE,MotorControlHandles[i]->getStatusLed(LED_WITHIN_RANGE, millis));
			//		panel4.setLed(LED_MOTOR_HOIST_LIMIT_MAX,MotorControlHandles[i]->getStatusLed(LED_LIMIT_MAX, millis));
			//		panel4.setLed(LED_MOTOR_HOIST_LIMIT_MAX,MotorControlHandles[i]->getStatusLed(LED_ENABLE, millis));
			//}

			panel4.setLed(LED_MOTOR_HOIST_LIMIT_MIN,motor1.getStatusLed(LED_LIMIT_MIN,millis));
			panel4.setLed(LED_MOTOR_HOIST_INRANGE,motor1.getStatusLed(LED_WITHIN_RANGE, millis));
			panel4.setLed(LED_MOTOR_HOIST_LIMIT_MAX,motor1.getStatusLed(LED_LIMIT_MAX, millis));
		//	panel4.setLed(LED_MOTOR_HOIST_ENABLE,motor1.getStatusLed(LED_ENABLE, millis));

			panel4.setLed(LED_MOTOR_CROWD_LIMIT_MIN,motor2.getStatusLed(LED_LIMIT_MIN,millis));
			panel4.setLed(LED_MOTOR_CROWD_INRANGE,motor2.getStatusLed(LED_WITHIN_RANGE, millis));
			panel4.setLed(LED_MOTOR_CROWD_LIMIT_MAX,motor2.getStatusLed(LED_LIMIT_MAX, millis));
		//	panel4.setLed(LED_MOTOR_CROWD_ENABLE,motor2.getStatusLed(LED_ENABLE, millis));

			//testing
		//	panel4.setLed(LED_MOTOR_CROWD_POS,motor2.getStatusLed(LED_MOVING_POSITIVE, millis));
		//	panel4.setLed(LED_MOTOR_CROWD_NEG,motor2.getStatusLed(LED_MOVING_NEGATIVE, millis));





			panel4.setLed(LED_MOTOR_SWING_LIMIT_MIN,motor3.getStatusLed(LED_LIMIT_MIN,millis));
			panel4.setLed(LED_MOTOR_SWING_INRANGE,motor3.getStatusLed(LED_WITHIN_RANGE, millis));
			panel4.setLed(LED_MOTOR_SWING_LIMIT_MAX,motor3.getStatusLed(LED_LIMIT_MAX, millis));
			//panel4.setLed(LED_MOTOR_SWING_ENABLE,motor3.getStatusLed(LED_ENABLE, millis));

			//total motion indicator.
			IOBoardHandler[3]->ledSequenceRefreshValue(MotorControlHandles[0]->getPosition() + MotorControlHandles[1]->getPosition() +MotorControlHandles[2]->getPosition() );



		}

		//extra button handling
		if (millis - millisMemory_extraButtonDebounce >= EXTRA_BUTTON_DEBOUNCE_MILLIS){
			this->millisMemory_extraButtonDebounce = millis; //debounce control


				if( GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_12)){

					printf("pd12\r\n");
				}


		}


		//external button zeroing procedure
		if (millis - millisMemory_externalZeroingButtonDebounce >= EXTERNAL_ZEROING_BUTTON_DEBOUNCE_MILLIS){
			this->millisMemory_externalZeroingButtonDebounce = millis; //debounce control




			//debounced button press.
			// press 5 seconds, release, press another 5 seconds.   --> zeroing mode.  press another 5 seconds: set axis to zero.


			bool buttonPressed  = getExternalZeroingButtonPressed();
			if (buttonPressed && !externalZeroingButtonPressedMemory){
				//positive edge
				millisMemory_externalZeroingButtonPressStartTime = this->millis;
				externalZeroingNumberConsequtiveLongPresses_numberIsAdded =false;
			}

			if (!buttonPressed && externalZeroingButtonPressedMemory){
				//negative edge
				externalZeroingNumberConsequtiveLongPresses_numberIsAdded =false;
				if (this->millis - millisMemory_externalZeroingButtonPressStartTime < EXTERNAL_ZEROING_BUTTON_TRIGGER_MILLIS){
					//if it was a short press, reset everything.
					externalZeroingNumberConsequtiveLongPresses =0;
				}

			}

			if (!externalZeroingNumberConsequtiveLongPresses_numberIsAdded && buttonPressed && this->millis - millisMemory_externalZeroingButtonPressStartTime > EXTERNAL_ZEROING_BUTTON_TRIGGER_MILLIS ){
				//button pressed for a long time positive edge
				externalZeroingNumberConsequtiveLongPresses ++;
				externalZeroingNumberConsequtiveLongPresses_numberIsAdded = true;

				printf("external zeroing button pressed %d times \r\n", externalZeroingNumberConsequtiveLongPresses);

				if (externalZeroingNumberConsequtiveLongPresses == 3){
					setAllMotorPositionsToZero();
					externalZeroingNumberConsequtiveLongPresses =0;
				}
			}


			externalZeroingButtonPressedMemory =buttonPressed; //button press edge control

			/*

			if (getExternalZeroingButtonPressed()){
				STM_EVAL_LEDOn(LED4);
			}else{
				STM_EVAL_LEDOff(LED4);
			}
			*/

		}
		//each second triggered
		if (millis - millisMemory_secondsBlinker >= 1000){
				this->millisMemory_secondsBlinker = millis; //edge control

			//for (uint16_t i = 0;i<4;i++){
			//	if (panel1.getButtonState(i)){
			//		printf("button %d pressed!\r\n", i);
			//		printf("buttonToggle Switch value: %d \r\n",panel1.getButtonValueToggleSwitch(i));
			//	}
			//}

			//for (uint16_t i = 0;i<4;i++){
			//	if (panel2.getButtonState(i)){
			//		printf("panel 2 button %d pressed!\r\n", i);
			//	}
			//}

			STM_EVAL_LEDToggle(LED3) ;


		}





#ifdef USE_VCP
		// If there's data on the virtual serial port:
		 //  - Echo it back
		 //  - Turn the green LED on for 10ms
		 //

		if (millis - millisMemory_checkForSerialInput >= 200){
			this->millisMemory_checkForSerialInput = millis; //edge control

			//if (VCP_get_char(&theByte))
			if (this->getCharFunctionPointer(&theByte)) //VCP_get_char is a c function, not working from here, we assign the pointer to the function in main.
			{
				if ( theByte != '\r' &&  theByte != '\n'){
					printf("Char Sent: %c  \r\n", theByte); //VCP_put_char(theByte);

					if ( theByte == 'v'){
						//printf ("value %d /r/n", ConvertedValue);

					//	printf ("samples taken: %d \r\n", adcNumberOfSampleCycles);
						//printf ("value TEMPERATURE %d \r\n", temp);
						printf ("value VREF %d \r\n", this->vref);

						for (uint8_t i=0; i<4;i++){
							//printf ("value slider: %d = %d \r\n", i, adcValues[i]);
							//printf ("panel1 slider %d: %d \r\n", i, panel1.getSliderValue(i));

						}

						printf("nothing here yet.");
					}else if (theByte == 's'){
						//char lodeStrTest [100]; //={'a','\0'};
						//lodeStrTest[99] = '\0';
						//char lodeStrTest []={'a','\0'};
						//panel1.stats(&lodeStrTest);
						//printf ("lets do this: %s \r\n", lodeStrTest);
						//printf("nothing here yet.");
						printf ("slider value:%d \r\n", IOBoardHandler[0]->getSliderValue(0));


					}else if (theByte == '1') {
						printf("motor id: %d \r\n", motor1.getMotorId());
						printf("speed Setting Percentage: %d, speed Out: %d \r\n", motor1.getSpeedPercentageDesired(), motor1.getSpeedPercentageChecked() );
						printf("raw input adc: %d, raw output dac: %d\r\n",  panel1.getSliderValue(0),this->dacValues[0]);
						printf("position: %d \r\n", motor1.getPosition());
						printf("limits:  min:  %d  --  max: %d \r\n", motor1.getLimit(false), motor1.getLimit(true));
					}else if (theByte == '2'){
						printf("motor id: %d \r\n", motor2.getMotorId());
						printf("speed Setting Percentage: %d, speed Out: %d \r\n", motor2.getSpeedPercentageDesired(), motor2.getSpeedPercentageChecked() );
						printf("raw input adc: %d, raw output dac: %d\r\n",  panel1.getSliderValue(1),this->dacValues[1]);
						printf("position: %d \r\n", motor2.getPosition());
						printf("limits:  min:  %d  --  max: %d \r\n", motor2.getLimit(false), motor2.getLimit(true));

					}else if (theByte == '3'){
						printf("motor id: %d \r\n", motor3.getMotorId());
						printf("speed Setting Percentage: %d, speed Out: %d \r\n", motor3.getSpeedPercentageDesired(), motor3.getSpeedPercentageChecked() );
						printf("raw input adc: %d, raw output dac: %d\r\n",  panel1.getSliderValue(2),this->dacValues[2]);
						printf("position: %d \r\n", motor3.getPosition());
						printf("limits:  min:  %d  --  max: %d \r\n", motor3.getLimit(false), motor3.getLimit(true));

					}else{
						printf("BMTWBM shovel model speed and position control v1.0 \r\nNo valid command detected. Please send\r\n 1 for hoist , \r\n 2 for crowd , \r\n 3 for swing motor status  ,\r\n. \r\n");
					}
				}
			}
		}
#endif



}

void  MachineControl::logVref(uint16_t value){
	this->vref = value;
}

void MachineControl::setAllMotorPositionsToZero(){
	for (uint8_t i=0; i<NUMBER_OF_MOTORS;i++){
		EncoderToTimerHandles[i]->reset();
		MotorControlHandles[i]->setCurrentPositionToZero();

	}
	printf("all motor positions are set to zero");
}
void MachineControl::initExternalZeroingButton(){
	//button is: PD11

	 // Set variables used
	GPIO_InitTypeDef GPIO_InitStruct;

	// Enable clock for GPIOB
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	// Set pin as input
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

}
bool MachineControl::getExternalZeroingButtonPressed(){

	return !GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_11) ;
}
void MachineControl::initExtraButton(){
	//button is: PD12

	 // Set variables used
	GPIO_InitTypeDef GPIO_InitStruct;

	// Enable clock for GPIOB
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

	// Set pin as input
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	//GPIO_InitStruct.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	//GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	//GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

}



bool MachineControl::getMotorsZeroedSinceStartup(){
	bool allMotorsOk = true;
	//check all motors for axis was zeroed.
	for (uint8_t i=0; i<NUMBER_OF_MOTORS;i++){
		if (!MotorControlHandles[i]->getZeroingAxisHappenedAtLeastOnce()){
			allMotorsOk = false;
		}
	}
	return allMotorsOk;
}

void MachineControl::selectNextLimitToBeCalibrated(){

	activeLimit = MotorControlHandles[activeMotorForTestingOrCalibration]->getSelectedLimitForCalibration();

	if (activeLimit >= 2){
		MotorControlHandles[activeMotorForTestingOrCalibration]->selectLimitToBeCalibrated(0);
		activeMotorForTestingOrCalibration++;
		activeLimit = 0;//because of ++ will be set to 1 further down this routine
	}

	if (activeMotorForTestingOrCalibration >= NUMBER_OF_MOTORS){
		activeMotorForTestingOrCalibration = 0;
	}

	activeLimit++;
	MotorControlHandles[activeMotorForTestingOrCalibration]->selectLimitToBeCalibrated(activeLimit);
}



//---------------------------------------------------------------------------------------
/*
void MachineControl::setUpInputPin_motor1_channelB(){
	//PB5
	 // Set variables used
		GPIO_InitTypeDef GPIO_InitStruct;
		EXTI_InitTypeDef EXTI_InitStruct;
		NVIC_InitTypeDef NVIC_InitStruct;

		// Enable clock for GPIOB
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		// Enable clock for SYSCFG
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

		// Set pin as input
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_5;
		//GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOB, &GPIO_InitStruct);


		//GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);

}

void MachineControl::setUpHardWareInterrupt_motor1_channelA(){
	//PB3
	//https://stm32f4-discovery.net/2014/08/stm32f4-external-interrupts-tutorial/

	 // Set variables used
	    GPIO_InitTypeDef GPIO_InitStruct;
	    EXTI_InitTypeDef EXTI_InitStruct;
	    NVIC_InitTypeDef NVIC_InitStruct;

	    // Enable clock for GPIOB
	    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	    // Enable clock for SYSCFG
	    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	    // Set pin as input
	    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	    //GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	    GPIO_Init(GPIOB, &GPIO_InitStruct);

	    // Tell system that you will use PB3 for EXTI_Line3
	    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource3);

	    // PB3 is connected to EXTI_Line3
	    EXTI_InitStruct.EXTI_Line = EXTI_Line3;
	    // Enable interrupt
	    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	    // Interrupt mode
	    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	    // Triggers on rising and falling edge
	    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	    //EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	    // Add to EXTI
	    EXTI_Init(&EXTI_InitStruct);

	    // Add IRQ vector to NVIC
	    // PB12 is connected to EXTI_Line12, which has EXTI15_10_IRQn vector
	    NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
	    // Set priority
	    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	    // Set sub priority
	    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;
	    // Enable interrupt
	    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	    // Add to NVIC
	    NVIC_Init(&NVIC_InitStruct);


}

void MachineControl::Motor1InterruptHandler(){
	//triggers on rising and falling edge of encoder channel
		//we are not interested in the added accuracy, but we need to check the edges (jitter at standstill could cause erroneous possition change)
		//edge up --> position change ,(only if channel 2 is different from edge down value)
		//edge down --> store channel 2
	    if (EXTI_GetITStatus(EXTI_Line3) != RESET) { //Make sure that interrupt flag is set
	    	//printf ("checkkflelfl;f");
	    	bool isCCW = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5);//check other channel
	    	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3)){
	    		//positive edge
	    		if (isCCW != motor1ChannelBMemory){
					//IOBoardHandler[3]->ledSequenceInterruptHandler(!isCCW); //input defines direction
					MotorControlHandles[0]->updatePositionOneStep(isCCW); //2 channel encoder update.
	    		}
			}else{
				//negative edge
				motor1ChannelBMemory = isCCW; //store ch2.
	    	}
	        // Clear interrupt flag
	        EXTI_ClearITPendingBit(EXTI_Line3);

	    }
}
*/
//----------------------------------------------------------------------------------------------------------------------------------------------------
/*
void MachineControl::setUpInputPin_motor2_channelB(){
	//PB8
	 // Set variables used
		GPIO_InitTypeDef GPIO_InitStruct;
		EXTI_InitTypeDef EXTI_InitStruct;
		NVIC_InitTypeDef NVIC_InitStruct;

		// Enable clock for GPIOB
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		// Enable clock for SYSCFG
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

		// Set pin as input
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
		//GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOB, &GPIO_InitStruct);

}

void MachineControl::setUpHardWareInterrupt_motor2_channelA(){
	//PB4
	//https://stm32f4-discovery.net/2014/08/stm32f4-external-interrupts-tutorial/

	 // Set variables used
	    GPIO_InitTypeDef GPIO_InitStruct;
	    EXTI_InitTypeDef EXTI_InitStruct;
	    NVIC_InitTypeDef NVIC_InitStruct;

	    // Enable clock for GPIOB
	    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	    // Enable clock for SYSCFG
	    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	    // Set pin as input
	    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
	    //GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	    GPIO_Init(GPIOB, &GPIO_InitStruct);


	    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource4);

	    EXTI_InitStruct.EXTI_Line = EXTI_Line4;
	    // Enable interrupt
	    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	    // Interrupt mode
	    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	    // Triggers on rising and falling edge
	    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	    //EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	    // Add to EXTI
	    EXTI_Init(&EXTI_InitStruct);

	    // Add IRQ vector to NVIC
	    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
	    // Set priority
	    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	    // Set sub priority
	    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;
	    // Enable interrupt
	    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	    // Add to NVIC
	    NVIC_Init(&NVIC_InitStruct);


}



void MachineControl::Motor2InterruptHandler(){
	//triggers on rising and falling edge of encoder channel
		//we are not interested in the added accuracy, but we need to check the edges (jitter at standstill could cause erroneous possition change)
		//edge up --> position change ,(only if channel 2 is different from edge down value)
		//edge down --> store channel 2
	    if (EXTI_GetITStatus(EXTI_Line4) != RESET) { //Make sure that interrupt flag is set
	    	bool isCCW = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8);//check other channel
	    	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4)){
	    		//positive edge
	    		if (isCCW != motor2ChannelBMemory){
					//IOBoardHandler[3]->ledSequenceInterruptHandler(!isCCW); //input defines direction
					MotorControlHandles[1]->updatePositionOneStep(isCCW); //2 channel encoder update.
	    		}
			}else{
				//negative edge
				motor2ChannelBMemory = isCCW; //store ch2.
	    	}
	        // Clear interrupt flag
	        EXTI_ClearITPendingBit(EXTI_Line4);

	    }
}


*/


/*
void MachineControl::setUpInputPin_motor3_channelB(){
	//PB0
	 // Set variables used
		GPIO_InitTypeDef GPIO_InitStruct;
		EXTI_InitTypeDef EXTI_InitStruct;
		NVIC_InitTypeDef NVIC_InitStruct;

		// Enable clock for GPIOB
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		// Enable clock for SYSCFG
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

		// Set pin as input
		GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
		GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
		//GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
		GPIO_Init(GPIOB, &GPIO_InitStruct);

}

void MachineControl::setUpHardWareInterrupt_motor3_channelA(){
	//PB1
	//https://stm32f4-discovery.net/2014/08/stm32f4-external-interrupts-tutorial/

	 // Set variables used
	    GPIO_InitTypeDef GPIO_InitStruct;
	    EXTI_InitTypeDef EXTI_InitStruct;
	    NVIC_InitTypeDef NVIC_InitStruct;

	    // Enable clock for GPIOB
	    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	    // Enable clock for SYSCFG
	    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	    // Set pin as input
	    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
	    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	    //GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	    GPIO_Init(GPIOB, &GPIO_InitStruct);

	    // Tell system that you will use PB3 for EXTI_Line1
	    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource1);

	    // PB3 is connected to EXTI_Line1
	    EXTI_InitStruct.EXTI_Line = EXTI_Line1;
	    // Enable interrupt
	    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	    // Interrupt mode
	    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	    // Triggers on rising and falling edge
	    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	    //EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Rising;
	    // Add to EXTI
	    EXTI_Init(&EXTI_InitStruct);

	    // Add IRQ vector to NVIC
	    // PB1 is connected to EXTI_Line1, which has     vectors for 0,1,2,3,4,  5-9, 10-15 . i.e. EXTI15_10_IRQn vector
	    NVIC_InitStruct.NVIC_IRQChannel = EXTI1_IRQn;
	    // Set priority
	    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	    // Set sub priority
	    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x01;
	    // Enable interrupt
	    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	    // Add to NVIC
	    NVIC_Init(&NVIC_InitStruct);
}

void MachineControl::Motor3InterruptHandler(){
	//triggers on rising and falling edge of encoder channel
		//we are not interested in the added accuracy, but we need to check the edges (jitter at standstill could cause erroneous possition change)
		//edge up --> position change ,(only if channel 2 is different from edge down value)
		//edge down --> store channel 2
	    if (EXTI_GetITStatus(EXTI_Line1) != RESET) { //Make sure that interrupt flag is set
	    	bool isCCW = GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0);//check other channel
	    	if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1)){
	    		//positive edge
	    		if (isCCW != motor3ChannelBMemory){
					//IOBoardHandler[3]->ledSequenceInterruptHandler(!isCCW); //input defines direction
					MotorControlHandles[2]->updatePositionOneStep(isCCW); //2 channel encoder update.
	    		}
			}else{
				//negative edge
				motor3ChannelBMemory = isCCW; //store ch2.
	    	}
	        // Clear interrupt flag
	        EXTI_ClearITPendingBit(EXTI_Line1);

	    }
}

*/





