/*
 * DistanceSensor.c
 *
 *  Created on: Dec 5, 2022
 *      Author: sebas
 */

#include "DistanceSensor.h"


#define PRESCALE_VAL 0
#define ARR_VAL 319

// Set up distance sensor
void DistanceSensor_init(){
	// Set clock for distance sensor GPIO pins & TIM2 timer
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

	// Configure PA5 to output and PA6 to input from sensor
	GPIOA->MODER &= ~(GPIO_MODER_MODE5 | GPIO_MODER_MODE6);
	GPIOA->MODER |= GPIO_MODER_MODE5_0;

	// Setting up TIM2 timer
	TIM2->PSC = PRESCALE_VAL;		// No prescaler
	TIM2->ARR = ARR_VAL; 			//Need ARR to go for 10 microseconds

	// Enable Global Interrupts
	TIM2->DIER |= TIM_DIER_UIE;

	// Clear Interrupt Flag
	TIM2->SR &= ~(TIM_SR_UIF);

	// Start Global Interrupts
	NVIC->ISER[0] |= 1<<TIM2_IRQn;
	__enable_irq();

}

// Sends out a 10 us pulse from the sensor to measure the distance
void requestDistance(){
	//Start Timer and turning PA5 High to have distance sensor to send pulse
	TIM2->CR1 |= TIM_CR1_CEN;
	GPIOA->ODR |= GPIO_PIN_5;
}


#define SPEED_CONSTANT 0.3451 // m/s
#define CALIBRATION_FACTOR 1.115

// Calculates the average distance from 100 consecutive measurements
int calcAverage(int arr_len, uint16_t ADC_Vals[]){
		uint16_t average = 0;	// Average Round Trip Time
		uint16_t total = 0; 		// Round Trip Time

		// Accumulates total
		for(int i = 0; i < arr_len; i+=2){
			total += ADC_Vals[i+1] - ADC_Vals[i];
		}

		average = total / (arr_len / 2); // Find average

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
