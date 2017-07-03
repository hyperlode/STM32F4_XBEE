#include "AppliedDAC.h"

AppliedDAC::AppliedDAC(){


}

void AppliedDAC::init(uint8_t channel){
	//1 and 2 are standard for stm32F4
	//3 is improvised with and resistor ladder.

	this->channel = channel;

	switch (channel){
		case 1:
			initDAC1();
			break;
		case 2:
			initDAC2();
			break;

		case 3:
			initDAC3();
			break;

		default:
			break;


	}

}

void AppliedDAC::assignValue(uint32_t value){
	switch (channel){
			case 1:
				assignValueDAC1(value);
				break;
			case 2:
				assignValueDAC2(value);
				break;

			case 3:
				assignValueDAC3(value);
				break;

			default:
				break;


		}

}

void AppliedDAC::initDAC1(){ //book reference : p75 Digital Interface Design and Application
	//GPIO PA4
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 ;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	DAC_InitTypeDef DAC_InitStructure;

	//PA4 dac channel 1
	// DAC_GPIO_Config();

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC,ENABLE);
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	//DAC_InitStructure.DAC_Trigger = DAC_Trigger_Software; //trigger assigned value like this: DAC_SoftwareTriggerCmd(DAC_Channel_1,ENABLE);
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_None; //no trigger, will directly output the value when given.
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_1,&DAC_InitStructure);

	DAC_Cmd(DAC_Channel_1,ENABLE);
	DAC_SetChannel1Data(DAC_Align_12b_R,0);//ffff=2.89 , fff=2.89,0fff=2.89,fff0 =2.89, ff00=2.76,00ff=0.185,ff=0.185,
	//DAC_SoftwareTriggerCmd(DAC_Channel_1,ENABLE);


}
void AppliedDAC::initDAC2(){
	//GPIO  PA5
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	DAC_InitTypeDef DAC_InitStructure;

   //PA5 dac channel 2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC,ENABLE);
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	//DAC_InitStructure.DAC_Trigger = DAC_Trigger_Software; //trigger assigned value like this: DAC_SoftwareTriggerCmd(DAC_Channel_2,ENABLE);
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_None; //no trigger, will directly output the value when given.
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_2,&DAC_InitStructure);

	DAC_Cmd(DAC_Channel_2,ENABLE);
	DAC_SetChannel2Data(DAC_Align_12b_R,0xfff);//ffff=2.89 , fff=2.89,0fff=2.89,fff0 =2.89, ff00=2.76,00ff=0.185,ff=0.185,
	DAC_SoftwareTriggerCmd(DAC_Channel_2,ENABLE);
}

void AppliedDAC::initDAC3(){
	//8 bit dac with manual r2r ladder, we use half a gpio port. bits 0->7
	//use half a gpio port
	//PE 0->7

	//GPIO E
	dac3 = GPIOE;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7  ;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOE,&GPIO_InitStructure);


}

void AppliedDAC::assignValueDAC3(uint32_t value){
	//set register E only the 8 LSB
	/**/
	value &= 0x000000FF;//make sure only 8 lsb are values, the rest should be zero.
	GPIOE->ODR &= 0xFFFFFF00;
	//printf("value: %x \r\n",value);
	GPIOE->ODR |= value;
	//printf("register: %x \r\n",GPIOE->ODR);
	/**/

}

void AppliedDAC::assignValueDAC1(uint32_t value){
	//value as 12 bit, LSBit positions.
	DAC_SetChannel1Data(DAC_Align_12b_R,value);//ffff=2.89 , fff=2.89,0fff=2.89,fff0 =2.89, ff00=2.76,00ff=0.185,ff=0.185,
	//DAC_SoftwareTriggerCmd(DAC_Channel_1,ENABLE);

}

void AppliedDAC::assignValueDAC2(uint32_t value){
	//value as 12 bit, LSBit positions.
	DAC_SetChannel2Data(DAC_Align_12b_R,value);//ffff=2.89 , fff=2.89,0fff=2.89,fff0 =2.89, ff00=2.76,00ff=0.185,ff=0.185,
	//DAC_SoftwareTriggerCmd(DAC_Channel_1,ENABLE);

}
