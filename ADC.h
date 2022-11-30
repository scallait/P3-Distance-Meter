/*
 * ADC.h
 *
 *  Created on: Nov 30, 2022
 *      Author: Seb
 */

#ifndef SRC_P3_DISTANCE_METER_ADC_H_
#define SRC_P3_DISTANCE_METER_ADC_H_

#include "main.h"

void ADC_init();

int ADC_Conversion(uint16_t analog_Val);

void ADC_Avg(uint16_t * ADC_Arr, int array_length, int * output);

#endif /* SRC_P3_DISTANCE_METER_ADC_H_ */
