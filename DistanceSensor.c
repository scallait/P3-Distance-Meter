/*
 * DistanceSensor.c
 *
 *  Created on: Dec 5, 2022
 *      Author: sebas
 */

#include "DistanceSensor.h"

void DistanceSensor_init(){
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

		// Configure PA5 to output and PA6 to input
	GPIOA->MODER &= ~(GPIO_MODER_MODE5 | GPIO_MODER_MODE6);
	GPIOA->MODER |= GPIO_MODER_MODE5_0;

		//Setting clock for TIM2
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
	TIM2->PSC = 0;
	//Need ARR to go for 10 microseconds
	TIM2->ARR = 319;
		//TODO: On oscilloscope was 9.6 microseconds (seemed to work but could be improved)
		//Setting up and clearing interrupt flag

	// Enable Global Interrupts
	TIM2->DIER |= TIM_DIER_UIE;

	// Clear Interrupt Flag
	TIM2->SR &= ~(TIM_SR_UIF);

	// Start Global Interrupts
	NVIC->ISER[0] |= 1<<TIM2_IRQn;
	__enable_irq();

}

void requestDistance(){
	//Will set the pin high to send the required 10 microsecond pulse to the distance sensor

	//Starting Timer and turning PA5 High to have distance sensor to send pulse
	TIM2->CR1 |= TIM_CR1_CEN;
	GPIOA->ODR |= GPIO_PIN_5;
}

#define SPEED_CONSTANT 0.3451 // m/s
#define CALIBRATION_FACTOR 1.115

int calcAverage(int arr_len, uint16_t ADC_Vals[]){
		uint16_t average = 0;	// Average Round Trip Time
		uint16_t total = 0; 		// Round Trip Time

		for(int i = 0; i < arr_len; i+=2){
			total += ADC_Vals[i+1] - ADC_Vals[i];
		}

		average = total / (arr_len / 2); // Find average (Values of half)

		return average;
}

// Calculate distance from round trip travel time
int calcDistance(int RTT){
	uint16_t distance = 0;

	// Calculate Distance
	distance = RTT * SPEED_CONSTANT; // Will need to calibrate

	distance = CALIBRATION_FACTOR * distance;

	return distance;
}
