/*
 * DM.c
 *
 *  Created on: Nov 30, 2022
 *      Author: Seb
 */
#include "DM.h"
#include "USART.h"

// Sets up the starting UI and the UI for the 1 Input Mode
void GUI_init(){
	USART_ESC_Code("[2J");		// Clear the entire screen
	USART_ESC_Code("[1;0H");	// Move cursor to position (0,0)
	USART_print("-----------------------------------------------------------------");
	USART_ESC_Code("[2;0H");
	USART_ESC_Code("[1m");
	USART_print("|                        Distance Meter                         |");
	USART_ESC_Code("[0m");
	USART_ESC_Code("[3;0H");
	USART_print("-----------------------------------------------------------------");
	USART_ESC_Code("[4;0H");
	USART_print("|                                                               |");
	USART_ESC_Code("[5;0H");
	USART_print("|     000 centimeters                000 inches                 |");
	USART_ESC_Code("[6;0H");
	USART_print("|                                                               |");
	USART_ESC_Code("[7;0H");
	USART_print("|    0.00 meters                    0.00 feet                   |");
	USART_ESC_Code("[8;0H");
	USART_print("|                                                               |");
	USART_ESC_Code("[9;0H");
	USART_print("-----------------------------------------------------------------");
}


#define INCHES_TO_FEET 100 / 12  // 12 inches in one foot & multiply by 100 for integer operations

// Prints the measurements for the 1 input mode
void printDistance(int centimeters, int inches){
	// Print Centimeters Value
	USART_ESC_Code("[5;7H");	// Row 7 Column 41
	USART_print_num(centimeters);

	// Print Meters Value
	USART_ESC_Code("[7;6H");
	USART_print_decimal(centimeters);

	// Print Inches
	USART_ESC_Code("[5;38H");
	USART_print_num(inches);

	// Print Feet
	USART_ESC_Code("[7;37H");
	USART_print_decimal(inches * INCHES_TO_FEET);

	USART_ESC_Code("[16;0H");	// Move cursor out of table
}

// Sets up the UI for the 2 Measurement Input Mode
void GUI_2D_init(){
	USART_ESC_Code("[2J"); 		// Clear the screen
	USART_ESC_Code("[1;0H");	// Move cursor to the correct location on the screen
	USART_print("-----------------------------------------------------------------");
	USART_ESC_Code("[2;0H");
	USART_ESC_Code("[1m");
	USART_print("|                        Distance Meter                         |");
	USART_ESC_Code("[0m");
	USART_ESC_Code("[3;0H");
	USART_print("-----------------------------------------------------------------");
	USART_ESC_Code("[4;0H");
	USART_print("|                                                               |");
	USART_ESC_Code("[5;0H");
	USART_print("|            Reading 1: 000 cm     Reading 2: 000 cm            |");
	USART_ESC_Code("[6;0H");
	USART_print("|                                                               |");
	USART_ESC_Code("[7;0H");
	USART_print("|                       Area: 00000 cm^2                        |");
	USART_ESC_Code("[8;0H");
	USART_print("|                                                               |");
	USART_ESC_Code("[9;0H");
	USART_print("-----------------------------------------------------------------");
}

#define FIRST_VALUE_INDEX 0
#define SECOND_VALUE_INDEX 1

// Updates the measurements for the 2 input mode
void print2Distance(uint16_t array2D[]){

	// Print 1st Value
	USART_ESC_Code("[5;25H");	// Row 7 Column 41
	USART_print_num(array2D[FIRST_VALUE_INDEX]);

	// Print 2nd Value
	USART_ESC_Code("[5;47H");
	USART_print_num(array2D[SECOND_VALUE_INDEX]);

	// Print Area
	USART_ESC_Code("[7;31H");
	USART_print_area(array2D[FIRST_VALUE_INDEX] * array2D[SECOND_VALUE_INDEX]);

	USART_ESC_Code("[16;0H");	// Move cursor out of table
}

