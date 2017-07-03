/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APPLIEDDAC_H
#define APPLIEDDAC_H
#include <stdint.h>
#include <stdio.h>
#include "stm32f4xx_dac.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"

class AppliedDAC{

public:
	AppliedDAC();
	void init(uint8_t channel);
	void assignValue(uint32_t value);

private:
	void initDAC1();
	void initDAC2();
	void initDAC3();
	void assignValueDAC1(uint32_t value);
	void assignValueDAC2(uint32_t value);
	void assignValueDAC3(uint32_t value);

	GPIO_TypeDef* dac3;
	uint8_t channel;
	uint32_t value;
};

#endif
