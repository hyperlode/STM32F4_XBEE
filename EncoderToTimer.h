/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef ENCODERTOTIMER_H
#define ENCODERTOTIMER_H


#define NUMBER_OF_STEPS_TO_ASSUME_OVERFLOW 25000  //make sure to refresh quick enough to avoid counting wrong!


//http://www.st.com/content/ccc/resource/technical/document/application_note/group0/91/01/84/3f/7c/67/41/3f/DM00236305/files/DM00236305.pdf/jcr:content/translations/en.DM00236305.pdf


#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_gpio.h"
#include <stdio.h>

typedef enum
{
  ENCODER_1 = 0,
  ENCODER_2 = 1,
  ENCODER_3 = 2
} Encoder_TypeDef;




class EncoderToTimer{

public:
	EncoderToTimer();
	void init(Encoder_TypeDef encoder);
	void refresh();
	int32_t getValue();
	void reset();



private:
	uint16_t previousRefreshTimerValue;
	uint16_t timerValue;
	int32_t position;
	Encoder_TypeDef encoderId;



};

#endif //ENCODERTOTIMER_H
