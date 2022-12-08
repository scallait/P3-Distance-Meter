/*
 * keypad.h
 *
 *  Created on: Dec 4, 2022
 *      Author: Seb
 */

#ifndef SRC_KEYPAD_H_
#define SRC_KEYPAD_H_
#include <stdio.h>
#include <stdint.h>
#include "main.h"

void keypad_init();

int16_t keypad_read();

#endif /* SRC_KEYPAD_H_ */
