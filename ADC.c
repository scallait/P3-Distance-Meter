/*
 * ADC.c
 *
 *  Created on: Nov 30, 2022
 *      Author: Seb
 */
#include "ADC.h"
#include "USART.h"
#include <stdio.h>
#include <string.h>

void ADC_init(){
	// Turn on clock to ADC
	RCC->AHB2ENR |= RCC_AHB2ENR_ADCEN;
	ADC123_COMMON->CCR &= ~(ADC_CCR_CKMODE);
	ADC123_COMMON->CCR |= (1 << ADC_CCR_CKMODE_Pos);
	/* Pg. 614 This configuration must be enabled only if the AHB
	clock prescaler is set to 1 (HPRE[3:0] = 0xxx in RCC_CFGR register) */

	// Supply power to ADC (Pg. 515)
	ADC1->CR &= ~(ADC_CR_DEEPPWD); //Clear bit to start ADC operations
	ADC1->CR |= ADC_CR_ADVREGEN;
	HAL_Delay(1); // ADCVR Start up time

	// Calibrate ADC (Pg. 516-7)
	ADC1->CR &= ~(ADC_CR_ADEN);		// Ensure ADC is not enabled
	ADC1->CR &= ~ADC_CR_ADCALDIF; 	// single-ended input conversions
	ADC1->CR |= ADC_CR_ADCAL;		// Start calibration sequence
	while(ADC1->CR & ADC_CR_ADCAL); // Wait until calibration sequence is complete
	/* Note - Can read calibration factor from ADC_CALFACT register */

	// Turn on ADC (Pg. 519)
	ADC1->ISR |= ADC_ISR_ADRDY; 		// Clear the ARDY bit by setting to 1 in ISR
	ADC1->CR |= ADC_CR_ADEN;
	while(!(ADC1->ISR & ADC_ISR_ADRDY)); // Set once ADC is ready

	// Configure the ADC
	ADC1->SQR1 &= ~(0b11111 << ADC_SQR1_SQ1_Pos);
	ADC1->SQR1 |= 0x1 << ADC_SQR1_SQ1_Pos; // Set channel 1?
	ADC1->SMPR1 &= ~(0b111 << ADC_SMPR1_SMP1_Pos); // Clear SMPR (Sets it to 2.5 ADC Clock Cycles)
	ADC1->SMPR1 |= 7 << ADC_SMPR1_SMP1_Pos; // Set to 640.5 ADC clock cycles

	// Configure GPIO for ADC (Can be done any time)
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	GPIOC->MODER |= GPIO_MODER_MODE0; // Set to Analog Mode (11)
	GPIOC->ASCR |= GPIO_ASCR_ASC0;	// Connect analog switch to the ADC input

	// Configure interrupts
	NVIC->ISER[0] |= 1 << (0x1F & ADC1_2_IRQn);
	ADC1->IER |= ADC_IER_EOCIE;
	ADC1->ISR &= ~(ADC_IER_EOCIE);

	__enable_irq();
}

#define MAX_ANALOG 4095.0
#define REF_VOLTAGE 330
#define LOW_CALIBRATION_CUTOFF 434
#define MID_CALIBRATION_CUTOFF 1861
#define LOW_CALIBRATION_COEFF 1.02
#define MID_CALIBRATION_COEFF 1.008
#define HIGH_CALIBRATION_COEFF 1.0028

int ADC_Conversion(uint16_t dig_Val){
	// Calibration
	if(dig_Val < LOW_CALIBRATION_CUTOFF){ // 0 -> 0.35 V ?
		dig_Val *= LOW_CALIBRATION_COEFF;
	}
	if(dig_Val < MID_CALIBRATION_CUTOFF){ // 0.35 -> 1.5 V
		dig_Val *= MID_CALIBRATION_COEFF;
	}
	else{ // 1.5 -> 3.3 V
		dig_Val *= HIGH_CALIBRATION_COEFF;
	}

	// Calculation
	int analog_Val = (dig_Val / MAX_ANALOG) * REF_VOLTAGE + 1; //converting analog to digital

	return analog_Val;
}


