/*
 * DM.c
 *
 *  Created on: Nov 30, 2022
 *      Author: Seb
 */
#include "DM.h"
#include "USART.h"


void GUI_init(){
	USART_ESC_Code("[2J");		// Clear the entire screen
	USART_ESC_Code("[1;0H");	// Move cursor to position (0,0)
	USART_print("-----------------------------------------------------------------");
	USART_ESC_Code("[2;0H");	// Move cursor to position (0,1)
	USART_ESC_Code("[1m");
	USART_print("|                        Distance Meter                         |");
	USART_ESC_Code("[0m");
	USART_ESC_Code("[3;0H");	// Move cursor to position (0,2)
	USART_print("-----------------------------------------------------------------");
	USART_ESC_Code("[4;0H");
	USART_print("|                                                               |");
	USART_ESC_Code("[5;0H");	// Move cursor to position (0,4)
	USART_print("|     000 centimeters                000 inches                 |");
	USART_ESC_Code("[6;0H");
	USART_print("|                                                               |");
	USART_ESC_Code("[7;0H");	// Move cursor to position (0,5)
	USART_print("|    0.00 meters                    0.00 feet                   |");
	USART_ESC_Code("[8;0H");	// Move cursor to position (0,7)
	USART_print("|                                                               |");
	USART_ESC_Code("[9;0H");	// Move cursor to position (0,5)
	USART_print("-----------------------------------------------------------------");
}

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
	USART_print_decimal(inches * 100 / 12);

	USART_ESC_Code("[16;0H");	// Move cursor out of table
}
