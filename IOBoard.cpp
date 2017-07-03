#include "IOBoard.h"

//Lode custom made IO boards with four sliders, four buttons and four leds in standard configurations.
// if no sliders: 16 buttons, 16 leds.
//not completely abstract. every panel is still hard wired to this class. also, for the ADC, watch closely how to deal with the conversion interrupt! Check if ADC is initialized.
//all items of a certain type (leds, sliders, buttons) must use the same port per panel.


const uint32_t IOBoard::ledRingSequence_default[] = {0,1,2,3,7,11,15,14,13,12,8,4};


IOBoard::IOBoard(){
	//first thing to do is init --> with a panelId!
}

void IOBoard::init(PanelId_TypeDef panelId){
	this ->panelId = panelId;
	//check ADC init for further magic numbers and specific initializations.
		if (panelId == PANEL_1){
			//4sliders, 4 leds, 4 buttons
			numberOfLeds = 4;
			ledCathodePins[0] = GPIO_Pin_6;
			ledAnodePins[0] = GPIO_Pin_8;
			ledAnodePins[1] = GPIO_Pin_9;
			ledAnodePins[2] = GPIO_Pin_11;
			ledAnodePins[3] = GPIO_Pin_13;
			ledPort = GPIOC;
			ledPeripheral = RCC_AHB1Periph_GPIOC;

			numberOfButtons =4;
			buttonPins[0] = GPIO_Pin_13; //two buttons per pin
			buttonPins[1] = GPIO_Pin_14; //two buttons per pin
			buttonPort = GPIOB;
			buttonPeripheral = RCC_AHB1Periph_GPIOB;

			adcPins[0] = GPIO_Pin_0;
			adcPins[1] = GPIO_Pin_1;
			adcPins[2] = GPIO_Pin_2;
			adcPins[3] = GPIO_Pin_3;
			adcPort  = GPIOC;
			adcPeripheral = RCC_AHB1ENR_GPIOCEN;

		}else if (panelId == PANEL_2){
			//4sliders, 4 leds, 4 buttons
			numberOfLeds = 4;
			ledCathodePins[0] = GPIO_Pin_4;
			ledAnodePins[0] = GPIO_Pin_5,
			ledAnodePins[1] = GPIO_Pin_6;
			ledAnodePins[2] = GPIO_Pin_7;
			ledAnodePins[3] = GPIO_Pin_8 ;
			ledPort = GPIOB;
			ledPeripheral = RCC_AHB1Periph_GPIOB;

			numberOfButtons =4;
			buttonPins[0] = GPIO_Pin_3; //two buttons per pin
			buttonPins[1] = GPIO_Pin_12; //two buttons per pin
			buttonPort = GPIOB;
			buttonPeripheral = RCC_AHB1Periph_GPIOB;

			adcPins[0] = GPIO_Pin_0;
			adcPins[1] = GPIO_Pin_1;
			adcPins[2] = GPIO_Pin_2;
			adcPins[3] = GPIO_Pin_3;
			adcPort  = GPIOA;
			adcPeripheral = RCC_AHB1ENR_GPIOAEN;

		}else if (panelId == PANEL_3){
			// 16 buttons, 16 leds
			numberOfLeds = 16;
			ledCathodePins[0] = GPIO_Pin_0;
			ledCathodePins[1] = GPIO_Pin_1;
			ledCathodePins[2] = GPIO_Pin_2;
			ledCathodePins[3] = GPIO_Pin_3;
			ledAnodePins[0] = GPIO_Pin_4,
			ledAnodePins[1] = GPIO_Pin_5;
			ledAnodePins[2] = GPIO_Pin_6;
			ledAnodePins[3] = GPIO_Pin_7 ;
			ledPort = GPIOE;
			ledPeripheral = RCC_AHB1Periph_GPIOE;

			numberOfButtons =16;
			buttonPins[0] = GPIO_Pin_8;
			buttonPins[1] = GPIO_Pin_9;
			buttonPins[2] = GPIO_Pin_10;
			buttonPins[3] = GPIO_Pin_11;
			buttonPins[4] = GPIO_Pin_12;
			buttonPins[5] = GPIO_Pin_13;
			buttonPins[6] = GPIO_Pin_14;
			buttonPins[7] = GPIO_Pin_15;
			buttonPort = GPIOE;
			buttonPeripheral = RCC_AHB1Periph_GPIOE;

		}else if (panelId = PANEL_4){
			// 4 buttons, 16 leds
			numberOfLeds = 16;
			ledCathodePins[0] = GPIO_Pin_0;
			ledCathodePins[1] = GPIO_Pin_1;
			ledCathodePins[2] = GPIO_Pin_2;
			ledCathodePins[3] = GPIO_Pin_3;
			ledAnodePins[0] = GPIO_Pin_4,
			ledAnodePins[1] = GPIO_Pin_5;
			ledAnodePins[2] = GPIO_Pin_6;
			ledAnodePins[3] = GPIO_Pin_7 ;
			ledPort = GPIOD;
			ledPeripheral = RCC_AHB1Periph_GPIOD;

			numberOfButtons =4;
			buttonPins[0] = GPIO_Pin_10; //two buttons per pin
			buttonPins[1] = GPIO_Pin_11; //two buttons per pin
			buttonPort = GPIOB;
			buttonPeripheral = RCC_AHB1Periph_GPIOB;
		}


		ledSequenceUser_reset();

		buttonTimer = 0;
		buttonsReadHighElseLow =false;
		scanCathode = 0;
		scanCounterTest = 0;

		adcInitialized = false;
		buttonsInitialized = false;
		ledsInitialized = false;
}


void IOBoard::stats(char* outputString){

	//if (isConstructed){
		outputString[0]= 'O';
		printf( "number of leds: %d \r\n", this->numberOfLeds);
		//scanCathode++;
		printf( "enabled led row: %d \r\n", this->scanCathode);

		//for (uint16_t i =0; i<this->numberOfLeds; i++){
		//	printf( "led %d status: %d \r\n", i, this->leds[i]);
		//}

		printf("numberOf samples at %d : %d ", millis, scanCounterTest);

	//}else{
	//	outputString[0]= 'P';
	//}
}

void IOBoard::refresh(uint32_t millis){
	this->millis = millis;

	if (buttonsInitialized &&  millis - buttonTimer > BUTTON_PRESS_DELAY / 2){
		buttonTimer = millis;
		this->millis = millis;
		buttonsReadHighElseLow = !buttonsReadHighElseLow;
		if (buttonsReadHighElseLow){
			readButtonsHigh();
		}else{
			readButtonsLow();
			readButtons();
		}
	}

	if (ledsInitialized && millis - ledScanTimer > LED_SCAN_PERIOD_MILLIS){
		ledScanTimer =millis;
		scanLeds();
	}


	if (adcInitialized && millis - adcSampleTimer > ADC_SAMPLE_PERIOD_MILLIS){
		adcDoSingleConversion();
		adcSampleTimer = millis;
	}
}

void IOBoard::demoModeLoop(){
	if (millis - demoLooptimer > DEMOLOOP_UPDATE_DELAY){

		demoLooptimer = millis;
		if(adcInitialized){
			//panels with 4 sliders, 4 leds, 4 buttons
			for (uint8_t i=0;i<4;i++){
				setLed(i,true);
				setLedBlinkPeriodMillis(i, getSliderValue(i));
			}

			/*
			//call every 20 ms --> 50Hz
			for (uint8_t i=0;i<4;i++){
				this->demoLoopCounter[i]++; //update counter
				//if (getButtonState(i)){
				if (getButtonValueToggleSwitch(i)){
					//blinkmode
					if (demoLoopCounter[i] >  getSliderValue(i)/200){
						setLed(i,true);
					}else{
						setLed(i,false);
					}
				}else{
					//led off
					setLed(i,false);
				}
				if (this-> demoLoopCounter[i] > getSliderValue(i)/100){
					this->demoLoopCounter[i] = 0;
				}
			}
			*/
		}else if (numberOfButtons == 16 && numberOfLeds == 16){
			//panels with 16 buttons, 16 leds.
			for (uint16_t i=0; i<this->numberOfButtons;i++){
				//setLed(i,getButtonState(i));
				if (getButtonEdgePressed(i)){
					toggleLed(i);
				}
			}

		}else if  (numberOfButtons == 4 && numberOfLeds == 16){
			//panels with 16 leds, 4 buttons
			for (uint16_t i=0; i<this->numberOfButtons;i++){
				//setLed(i,getButtonState(i));
				if (getButtonEdgePressed(i)){
					for (uint16_t ledInColumn=0; ledInColumn<4;ledInColumn ++){
						toggleLed(i + 4*ledInColumn);
					}
				}
			}

		}
	}
}

/**************************************************************************************************************************************
 *
 * ADC
 *
 ***************************************************************************************************************************************/

void IOBoard::initADC(){

	//set gpio pins for the ADC
	//if (panelId == PANEL_1){

		//set pin PC0 as analog in
		RCC_AHB1PeriphClockCmd(adcPeripheral,ENABLE);//Clock for the ADC port!! Do not forget about this one ;)
		GPIO_InitTypeDef GPIO_initStructre; //Structure for analog input pin
		//Analog pin configuration
		GPIO_initStructre.GPIO_Pin = adcPins[0] | adcPins[1] | adcPins[2] | adcPins[3]  ;//The channel 10 is connected to PC0
		GPIO_initStructre.GPIO_Mode = GPIO_Mode_AN; //The PC0 pin is configured in analog mode
		GPIO_initStructre.GPIO_PuPd = GPIO_PuPd_NOPULL; //We don't need any pull up or pull down
		GPIO_Init(adcPort ,&GPIO_initStructre);//Affecting the port with the initialization structure configuration
	//}else  if (panelId == PANEL_2){
	//}

	//configure ADC1 (only if not done yet) should be done static?
	if (panelId == PANEL_1){

		/* Unchanged: Define ADC init structures */
		ADC_InitTypeDef       ADC_InitStructure;
		ADC_CommonInitTypeDef ADC_CommonInitStructure;
		NVIC_InitTypeDef NVIC_InitStructure;

		/* Unchanged: populate default values before use */
		ADC_StructInit(&ADC_InitStructure);
		ADC_CommonStructInit(&ADC_CommonInitStructure);

		/* Unchanged: enable ADC peripheral */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

		/* Unchanged: init ADC */
		ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
		ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
		ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
		ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
		ADC_CommonInit(&ADC_CommonInitStructure);

		/* Changed: Enabled scan mode conversion*/
		ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
		ADC_InitStructure.ADC_ScanConvMode = ENABLE;
		ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
		ADC_InitStructure.ADC_DataAlign= ADC_DataAlign_Right;
		ADC_InitStructure.ADC_ExternalTrigConv= 0;
		ADC_InitStructure.ADC_ExternalTrigConvEdge= 0;

		ADC_InitStructure.ADC_NbrOfConversion= 6;  //<--------------------------------------------CHANGE ACCORDINGLY
		//ADC_InitStructure.ADC_NbrOfConversion= 10;  //<-------------------------------CHANGE ACCORDINGLY should be static!

		ADC_Init(ADC1, &ADC_InitStructure);

		/* Enable Vref & Temperature channel */
		ADC_TempSensorVrefintCmd(ENABLE);


		ADC_EOCOnEachRegularChannelCmd(ADC1, ENABLE);

		/* Enable ADC interrupts */
		ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

		/* Configure NVIC */
		NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
		NVIC_Init(&NVIC_InitStructure);

		/* Enable ADC1 **************************************************************/
		ADC_Cmd(ADC1, ENABLE);

		//temperature and VBAT channels are internal
		/* Configure channels */
		/* Temp sensor */
		ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_480Cycles);
		/* VREF_int (2nd) */
		ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 2, ADC_SampleTime_480Cycles);

	}

	//link gpio pins to ADC
	if (panelId == PANEL_1){

		ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 3, ADC_SampleTime_480Cycles); ///PC0  //channel10
		ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 4, ADC_SampleTime_480Cycles);
		ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 5, ADC_SampleTime_480Cycles);
		ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 6, ADC_SampleTime_480Cycles);
	} if (panelId == PANEL_2){
		ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 7, ADC_SampleTime_480Cycles); ///PA0  //channel0
		ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 8, ADC_SampleTime_480Cycles);
		ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 9, ADC_SampleTime_480Cycles);
		ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 10, ADC_SampleTime_480Cycles);
	}
	adcInitialized = true;
}

void IOBoard::adcDoSingleConversion(){
	if (panelId == PANEL_1){
		ADC_SoftwareStartConv(ADC1);
	}
}

void IOBoard::ADCInterruptHandler(uint16_t slider, uint16_t value){
	//call this from the interrupt vector. Will update the data in this class with the appropriate value.
   // STM_EVAL_LEDOn(LED4);
	this->sliderValues[slider] = value;
	scanCounterTest ++;
}

uint16_t IOBoard::getSliderValue(uint16_t slider){
	return this->sliderValues[slider];
}

/**************************************************************************************************************************************
 *
 * BUTTONS
 *
**************************************************************************************************************************************/

void IOBoard::initButtons(){
	//very specific
		RCC_AHB1PeriphClockCmd(buttonPeripheral, ENABLE);
		//GPIO_InitTypeDef GPIO_Buttons_initStructure; defined in .h file, has to be available because we work with two buttons on one pin...
		//Analog pin configuration
		if (this->numberOfButtons == 4){
			GPIO_Buttons_initStructure.GPIO_Pin = buttonPins[0] | buttonPins[1];
		}else if (this->numberOfButtons == 16){
			GPIO_Buttons_initStructure.GPIO_Pin = buttonPins[0] | buttonPins[1] | buttonPins[2] | buttonPins[3] | buttonPins[4] | buttonPins[5] | buttonPins[6] | buttonPins[7];
			//GPIO_Buttons_initStructure.GPIO_Pin = buttonPins[0];

		}else {
			printf("init ERROR: number of buttons must be 4 or 16. ");
		}

		GPIO_Buttons_initStructure.GPIO_Mode = GPIO_Mode_IN ;
		GPIO_Buttons_initStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Buttons_initStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_Buttons_initStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_Init(buttonPort ,&GPIO_Buttons_initStructure);//Affecting the port with the initialization structure configuration

		for (uint8_t i=0; i<this->numberOfButtons;i++){
				buttonValuesToggleSwitch[i] = 0;
				buttonValues[i] = 0;
		}


		this->buttonsInitialized = true;
}

void IOBoard::readButtons(){
	//will read all four buttons.
	//this is just the last step after scanning readButtonsLow and readButtonsHigh

	//i.e.
	/*button readout
			if (ticker20ms>=20){
				 ticker20msEdgeMemory= !ticker20msEdgeMemory;
				if (ticker20msEdgeMemory){
					panel1.readButtonsHigh();
				}else{
					panel1.readButtonsLow();
					panel1.readButtons();
				}
				ticker20ms =0;
			}
	*/

	bool previousButtonValues[16];

	//preserve previous button values
	for (uint8_t i=0; i<this->numberOfButtons;i++){
		previousButtonValues[i] = buttonValues[i];
	}
	if (this->numberOfButtons == 4){
		//determine button presses
		buttonValues [0] = !pinsStatePullUpLow [0] && !pinsStatePullUpHigh [0];//low and low, means button connected to ground is switched
		buttonValues [1] =  pinsStatePullUpLow [0] &&  pinsStatePullUpHigh [0];// high and high means button connected to VCC is switched
		buttonValues [2] = !pinsStatePullUpLow [1] && !pinsStatePullUpHigh [1];//low and low, means button connected to ground is switched
		buttonValues [3] =  pinsStatePullUpLow [1] &&  pinsStatePullUpHigh [1];// high and high means button connected to VCC is switched
	}else if (this->numberOfButtons == 16){
		for (uint8_t i=0; i<this->numberOfButtons;i++){
			if (i%2 ==0){
				buttonValues [i] = !pinsStatePullUpLow [i/2] && !pinsStatePullUpHigh [i/2];
			}else{
				buttonValues [i] = pinsStatePullUpLow [i/2] && pinsStatePullUpHigh [i/2];
			}
		}
	}

	//check for edges
	for (uint8_t i=0; i<this->numberOfButtons;i++){
		buttonEdgesPressed[i] = 	!previousButtonValues[i] &&  buttonValues [i];
		buttonEdgesDePressed[i] = 	 previousButtonValues[i] && !buttonValues [i];

		if (buttonEdgesPressed[i]){
			buttonValuesToggleSwitch[i] = !buttonValuesToggleSwitch[i];
		}
	}

	//check if a state changed.
	this->atLeastOneButtonStateChanged = false;
	for (uint8_t i=0; i<this->numberOfButtons;i++){
		if (buttonEdgesPressed[i] || buttonEdgesDePressed[i]){
			this->atLeastOneButtonStateChanged = true;
		}
	}
}

bool IOBoard::getAtLeastOneButtonStateChanged(){
	bool memory;
	memory = this->atLeastOneButtonStateChanged;
	this->atLeastOneButtonStateChanged = false;
	return memory;
}

void IOBoard::readButtonsLow(){
	//ASSUMES the pull up resistor is not set (pin floating) no pull down either, this is set in the hardware.
		for (uint8_t i=0; i<this->numberOfButtons/2;i++){
			this->pinsStatePullUpLow [i] = GPIO_ReadInputDataBit(buttonPort, buttonPins[i]);
		}

		GPIO_Buttons_initStructure.GPIO_PuPd = GPIO_PuPd_UP; //set for the next cycle.
		GPIO_Init(buttonPort,&GPIO_Buttons_initStructure);//Affecting the port with the initialization structure configuration
}

void IOBoard::readButtonsHigh(){
	//ASSUMES the pull up resistor is enabled.
		for (uint8_t i=0; i<this->numberOfButtons/2;i++){
			this->pinsStatePullUpHigh [i] = GPIO_ReadInputDataBit(buttonPort, buttonPins[i]);
		}

		//put already down for the next cycle.
		//GPIO_Buttons_initStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_Buttons_initStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;//set for the next cycle --> no pull down resistor, already implemented in hardware...
		GPIO_Init(buttonPort,&GPIO_Buttons_initStructure);//Affecting the port with the initialization structure configuration
}

bool IOBoard::getButtonValueToggleSwitch(uint16_t button){
	return buttonValuesToggleSwitch[button];
}

bool IOBoard::getButtonState(uint16_t button){
	return buttonValues [button];
}

bool IOBoard::getButtonEdgeDePressed(uint16_t button){
	bool state;
	state = buttonEdgesDePressed [button];
	buttonEdgesDePressed [button] = false;
	return state;
}

bool IOBoard::getButtonEdgePressed(uint16_t button){
	//make sure each edge can only be called once.
	bool state;
	state = buttonEdgesPressed [button];
	buttonEdgesPressed [button] = false;
	return state;
}

/**************************************************************************************************************************************
 *
 * LEDS
 *
**************************************************************************************************************************************/

void IOBoard::initLeds(){
	//very specific

	if (numberOfLeds ==4){
		//leds
		RCC_AHB1PeriphClockCmd(ledPeripheral, ENABLE);
		//GPIO_InitTypeDef GPIO_initStructre; defined in .h file, has to be available because we work with two buttons on one pin...
		//Analog pin configuration
		GPIO_InitTypeDef GPIO_initStructre;
		GPIO_initStructre.GPIO_Pin = ledAnodePins[0] |  ledAnodePins[1] | ledAnodePins[2] | ledAnodePins[3] |ledCathodePins[0] ;
		GPIO_initStructre.GPIO_Mode = GPIO_Mode_OUT ;
		GPIO_initStructre.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_initStructre.GPIO_OType = GPIO_OType_PP;
		GPIO_initStructre.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(ledPort,&GPIO_initStructre);//Affecting the port with the initialization structure configuration

		//common cathode pin always low.
		GPIO_ResetBits(ledPort, ledCathodePins[0]);



	}else if (numberOfLeds == 16){
		//leds
		RCC_AHB1PeriphClockCmd(ledPeripheral, ENABLE);
		//GPIO_InitTypeDef GPIO_initStructre; defined in .h file, has to be available because we work with two buttons on one pin...
		//Analog pin configuration
		GPIO_InitTypeDef GPIO_initStructre;
		GPIO_initStructre.GPIO_Pin = ledAnodePins[0] |  ledAnodePins[1] | ledAnodePins[2] | ledAnodePins[3] | ledCathodePins[0] | ledCathodePins[1] |ledCathodePins[2] |ledCathodePins[3] ;
		GPIO_initStructre.GPIO_Mode = GPIO_Mode_OUT ;
		GPIO_initStructre.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_initStructre.GPIO_OType = GPIO_OType_PP;
		GPIO_initStructre.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_Init(ledPort,&GPIO_initStructre);//Affecting the port with the initialization structure configuration

		//common cathode pin always low.
		//GPIO_ResetBits(ledPort, ledCathodePin);

		//initialize cathode pins high (standard "OFF")
		for (uint16_t cathode = 0; cathode<this->numberOfLeds/4; cathode++){
			GPIO_SetBits(ledPort, ledCathodePins[cathode]);
		}

		//initialize anode pins low (standard "OFF")
		for (uint16_t anode = 0; anode<4; anode++){
			GPIO_SetBits(ledPort, ledAnodePins[anode]);
		}



	}
	//initialize leds
	for (uint16_t led = 0; led<this->numberOfLeds; led++){
		setLed(led, false); //, standard off
		setLedBlinkPeriodMillis(led, 0); //standard continous
	}
	this->ledsInitialized = true;

}

void IOBoard::ledSequenceUser_reset(){
	for (uint8_t i=0;i<16;i++){
		this->ledRingSequence_user[i]=UNEXISTING_LED;
	}
}

void IOBoard::ledSequenceUser_set(uint8_t arrayIndex, uint32_t ledNumber ){
	this->ledRingSequence_user[arrayIndex] = ledNumber;
}

void IOBoard::ledSequenceUpdate(bool directionIsForward){

	if (ledRingSequence_user[0] == UNEXISTING_LED){

		setLed(this->ledRingSequence_default[sequenceCounter] ,false);
		//this->leds[this->ledRingSequence_default[sequenceCounter]] = false;

    	if (directionIsForward){
    		//STM_EVAL_LEDToggle(LED4);
    		//CW
    		sequenceCounter ++;
			if (sequenceCounter>=12 ){
				sequenceCounter = 0;
			}

    	}else{
    		//CCW
    		sequenceCounter --;
    		if (sequenceCounter<0){
    			sequenceCounter = 11;
    		}
    	}
    	setLed(ledRingSequence_default[sequenceCounter] ,true);
    	//this->leds[this->ledRingSequence_default[sequenceCounter]] = true;
	}else{

		setLed(this->ledRingSequence_user[sequenceCounter] ,false);
		//this->leds[this->ledRingSequence_user[sequenceCounter]] = false;

    	if (directionIsForward){
    		//STM_EVAL_LEDToggle(LED4);
    		//CW
    		sequenceCounter ++;
			if (sequenceCounter>=16 || ledRingSequence_user[sequenceCounter] == UNEXISTING_LED ){
				sequenceCounter = 0;
			}

    	}else{
    		//CCW
    		sequenceCounter --;
    		if (sequenceCounter<0){

    			sequenceCounter = 15;
    			while (ledRingSequence_user[sequenceCounter] == UNEXISTING_LED){
    				sequenceCounter--;
    			}
    		}
    	}
    	setLed(ledRingSequence_user[sequenceCounter] ,true);
    	//this->leds[this->ledRingSequence_user[sequenceCounter]] = true;

	}
}


void IOBoard::ledSequenceRefreshValue(int32_t value){

	if (value > this->valueMemory ){
		if (value - this->valueMemory > NUMBER_OF_SUBSTEPS_PER_LED_SEQUENCE_STEP){

			ledSequenceUpdate( true);
			this->valueMemory += NUMBER_OF_SUBSTEPS_PER_LED_SEQUENCE_STEP;
		}
	}else{
		if (this->valueMemory - value  > NUMBER_OF_SUBSTEPS_PER_LED_SEQUENCE_STEP){
			ledSequenceUpdate( false);
			this->valueMemory -= NUMBER_OF_SUBSTEPS_PER_LED_SEQUENCE_STEP;
		}
	}
}
/*
void IOBoard::ledSequenceInterruptHandler(bool directionIsForward){

	if (directionIsForward){
		substeps++;
		if (substeps>=NUMBER_OF_SUBSTEPS_PER_LED_SEQUENCE_STEP){
			ledSequenceUpdate( directionIsForward);
			substeps = 0;
		}
	}else{
		substeps--;
		if (substeps<=0){
			ledSequenceUpdate( directionIsForward);
			substeps = NUMBER_OF_SUBSTEPS_PER_LED_SEQUENCE_STEP;
		}
	}


}

*/
void IOBoard::scanLeds(){

		//set "previous" scan cycle cathode to HIGH again (so it is "off")
		GPIO_SetBits(ledPort, ledCathodePins[scanCathode]);
		//scan every time scanLeds is called, we go for the next row of Leds.
		scanCathode++;
		if (scanCathode >= this->numberOfLeds/4 ){
			scanCathode =0;
		}





		for (uint8_t anode=0; anode<4;anode++){
			//led enabled AND blinking ok.
			uint32_t blinkPeriodMillis = ledsBlinkPeriod[(this->numberOfLeds/4)*scanCathode + anode]; //total blink period
			bool blinkOnPhase = true;
			if (blinkPeriodMillis != 0){
				blinkOnPhase = (this->millis % blinkPeriodMillis) > blinkPeriodMillis/2;
			}

			if (leds[(this->numberOfLeds/4)*scanCathode + anode] && blinkOnPhase){
				GPIO_SetBits(ledPort,this->ledAnodePins[anode]);
			}else{
				GPIO_ResetBits(ledPort,this->ledAnodePins[anode]);
			}
		}

		//set cathode low, so current can flow, enabling a row of leds.
		GPIO_ResetBits(ledPort, ledCathodePins[scanCathode]);

}

void IOBoard::setLedBlinkPeriodMillis(uint16_t ledNumber, uint32_t value){
	//values to about 50ms are displayed ok (see update scan frequency)
	//value: 0 is continuous, all the rest is millis.
	if (value ==0){
		//continuous
		ledsBlinkPeriod[ledNumber] = 0;
	}else{
		ledsBlinkPeriod[ledNumber] = value; //
	}
}

void IOBoard::setLed(uint16_t ledNumber, bool value){
	//ledNumber is index in array.
	this->leds[ledNumber] = value;

}

void IOBoard::toggleLed(uint16_t ledNumber){
	this->leds[ledNumber] = !this->leds[ledNumber];
}
