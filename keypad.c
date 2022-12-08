/*
 * keypad.c
 *
 *  Created on: Dec 4, 2022
 *      Author: Seb
 */

#include "keypad.h"

// Bitmask for corresponding row and column pins
int rows = GPIO_PIN_1;
int cols = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;

// Initializes keypad
void keypad_init(){
	// Enable the clock to the internal peripheral (GPIO Port A)
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOCEN);

	GPIOC->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7); // Clears pins C4-C7 to set to output mode
	GPIOC->MODER &= ~GPIO_MODER_MODE1; // Sets C1 to input mode for row
	GPIOC->MODER |= GPIO_MODER_MODE4_0 | GPIO_MODER_MODE5_0 | GPIO_MODER_MODE6_0 | GPIO_MODER_MODE7_0; // C4-C7 (Columns) to output mode

	//GPIOC->MODER |= GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0 | GPIO_MODER_MODE10_0 | GPIO_MODER_MODE11_0; // C8-C11 (LED Pins) to output mode

	// Enable pull down resistors on row pin
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD1); // Clears Pull Up/Down Register for row
	GPIOC->PUPDR |= GPIO_PUPDR_PUPD1_1; // Sets row to pull down

	// Set all columns to high
	GPIOC->BSRR = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;

}

#define REGISTER_RESET_SHIFT 16
#define ONE_KEY 1
#define TWO_KEY 2
#define THREE_KEY 3
#define TEN_KEY 10

// Reads a key press from the keypad
int16_t keypad_read(){
	GPIOC->BSRR = cols; // Set all columns to high
	// Checks for input from any row
	if(GPIOC->IDR & (rows)){
		GPIOC->BRR = cols; // Turns off all columns

		// Check column 0 (Reset previous column pin and set next column pin high)
		GPIOC->BSRR = GPIO_PIN_4;
		if(GPIOC->IDR & (rows)){
			return ONE_KEY;
		}

		// Check column 1 (Reset previous column pin and set next column pin high)
		GPIOC->BSRR = GPIO_PIN_5 | (GPIO_PIN_4 << REGISTER_RESET_SHIFT);
		if(GPIOC->IDR & (rows)){
			return TWO_KEY;
		}

		// Check column 2 (Reset previous column pin and set next column pin high)
		GPIOC->BSRR = GPIO_PIN_6 | (GPIO_PIN_5 << REGISTER_RESET_SHIFT);
		if(GPIOC->IDR & (rows)){
			return THREE_KEY;
		}

		// Check column 3 (Reset previous column pin and set next column pin high)
		GPIOC->BSRR = GPIO_PIN_7 | (GPIO_PIN_6 << REGISTER_RESET_SHIFT);
		if(GPIOC->IDR & (rows)){
			return TEN_KEY;
		}

	}
	return -1; // No button pressed
}
