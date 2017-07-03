#include "EncoderToTimer.h"


EncoderToTimer::EncoderToTimer(){



}
void EncoderToTimer::init(Encoder_TypeDef encoderId){

	this ->encoderId = encoderId;
	if (this->encoderId == ENCODER_1){
		GPIO_InitTypeDef GPIO_InitStructure;
		// turn on the clocks for each of the ports needed
		RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA, ENABLE);
		RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOB, ENABLE);


		// now configure the pins themselves
		// they are all going to be inputs with pullups
		GPIO_StructInit (&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		//GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
		GPIO_Init(GPIOA, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_Init(GPIOB, &GPIO_InitStructure);


		// Connect the pins to their Alternate Functions
		GPIO_PinAFConfig (GPIOA, GPIO_PinSource15, GPIO_AF_TIM2);
		GPIO_PinAFConfig (GPIOB, GPIO_PinSource3, GPIO_AF_TIM2);


		// Timer peripheral clock enable
		RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM2, ENABLE);


		// set them up as encoder inputs
		// set both inputs to rising polarity to let it use both edges
		/*
		TIM_EncoderInterfaceConfig (TIM2, TIM_EncoderMode_TI12,
								  TIM_ICPolarity_Rising,
								  TIM_ICPolarity_Rising);
		*/
		TIM_EncoderInterfaceConfig (TIM2, TIM_EncoderMode_TI1,
										  TIM_ICPolarity_Rising,
										  TIM_ICPolarity_Rising);



		TIM_SetAutoreload (TIM2, 0xffff);


		// turn on the timer/counters
		TIM_Cmd (TIM2, ENABLE);

		this->reset();
		printf("encoder init ready: \r\n");
	}else if (this->encoderId == ENCODER_2){
			GPIO_InitTypeDef GPIO_InitStructure;
			// turn on the clocks for each of the ports needed
			RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOB, ENABLE);


			// now configure the pins themselves
			// they are all going to be inputs with pullups
			GPIO_StructInit (&GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
			//GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
			GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
			GPIO_Init(GPIOB, &GPIO_InitStructure);

			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
			GPIO_Init(GPIOB, &GPIO_InitStructure);


			// Connect the pins to their Alternate Functions
			GPIO_PinAFConfig (GPIOB, GPIO_PinSource4, GPIO_AF_TIM3);
			GPIO_PinAFConfig (GPIOB, GPIO_PinSource5, GPIO_AF_TIM3);


			// Timer peripheral clock enable
			RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM3, ENABLE);


			// set them up as encoder inputs
			// set both inputs to rising polarity to let it use both edges
			/*
			TIM_EncoderInterfaceConfig (TIM4, TIM_EncoderMode_TI12,
									  TIM_ICPolarity_Rising,
									  TIM_ICPolarity_Rising);
			*/
			TIM_EncoderInterfaceConfig (TIM3, TIM_EncoderMode_TI1,
											  TIM_ICPolarity_Rising,
											  TIM_ICPolarity_Rising);



			TIM_SetAutoreload (TIM3, 0xffff);


			// turn on the timer/counters
			TIM_Cmd (TIM3, ENABLE);

			this->reset();
			printf("encoder init ready: \r\n");
	}else if (this->encoderId == ENCODER_3){
		GPIO_InitTypeDef GPIO_InitStructure;
		// turn on the clocks for each of the ports needed
		RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOB, ENABLE);


		// now configure the pins themselves
		// they are all going to be inputs with pullups
		GPIO_StructInit (&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
		//GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
		GPIO_Init(GPIOB, &GPIO_InitStructure);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
		GPIO_Init(GPIOB, &GPIO_InitStructure);


		// Connect the pins to their Alternate Functions
		GPIO_PinAFConfig (GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);
		GPIO_PinAFConfig (GPIOB, GPIO_PinSource7, GPIO_AF_TIM4);


		// Timer peripheral clock enable
		RCC_APB1PeriphClockCmd (RCC_APB1Periph_TIM4, ENABLE);


		// set them up as encoder inputs
		// set both inputs to rising polarity to let it use both edges
		/*
		TIM_EncoderInterfaceConfig (TIM4, TIM_EncoderMode_TI12,
								  TIM_ICPolarity_Rising,
								  TIM_ICPolarity_Rising);
		*/
		TIM_EncoderInterfaceConfig (TIM4, TIM_EncoderMode_TI1,
										  TIM_ICPolarity_Rising,
										  TIM_ICPolarity_Rising);



		TIM_SetAutoreload (TIM4, 0xffff);


		// turn on the timer/counters
		TIM_Cmd (TIM4, ENABLE);

		this->reset();
		printf("encoder init ready: \r\n");
	}else{

		printf("wrong encoder ID \r\n");
	}
}

void EncoderToTimer::refresh(){
	this->previousRefreshTimerValue = this->timerValue;

	if (this->encoderId == ENCODER_1){
		this->timerValue = TIM2->CNT;
	} else if (this->encoderId == ENCODER_2){
		this->timerValue = TIM3->CNT;
	}else if (this->encoderId == ENCODER_3){
		this->timerValue = TIM4->CNT;
	}

	//get absolute distance between the two last values.
	//uint16_t difference;
	//diff = this->timerValue > y ? this->timerValue - y : y - this->timerValue;

	if (this->timerValue > this->previousRefreshTimerValue){
		if (this->timerValue - this-> previousRefreshTimerValue >= NUMBER_OF_STEPS_TO_ASSUME_OVERFLOW){
			//assume overflow --> subtraction.
			this->position = this->position - this->previousRefreshTimerValue - (65535 - this->timerValue);
			//printf("**********neg overflow, steps: %d \r\n" , this->timerValue - this-> previousRefreshTimerValue);
			printf("---\r\n");
		}else{
			//add increment to position
			this->position += this->timerValue - this->previousRefreshTimerValue;
		}
	}else{
		if (this-> previousRefreshTimerValue  - this->timerValue >= NUMBER_OF_STEPS_TO_ASSUME_OVERFLOW){
			//assume overflow --> addition
			this->position = this->position + (65535 - this->previousRefreshTimerValue) + this->timerValue;
			//printf("**********pos overflow, steps: %d\r\n" ,this-> previousRefreshTimerValue - this->timerValue);
			printf("+++\r\n");
		}else{
			//subtract difference from position
			this->position -=  this->previousRefreshTimerValue - this->timerValue;
		}

	}

}

void EncoderToTimer::reset(){
	__disable_irq();
	this->position = 0;
	this->previousRefreshTimerValue = 0;
	this->timerValue = 0;
	if (this->encoderId == ENCODER_1){
		TIM_SetCounter (TIM2, 0);
	}else if (this->encoderId == ENCODER_2){
		TIM_SetCounter (TIM3, 0);
	}else if (this->encoderId == ENCODER_3){
		TIM_SetCounter (TIM4, 0);
	}

	refresh();
	__enable_irq();

}

int32_t EncoderToTimer::getValue(){
	return this->position;
}
