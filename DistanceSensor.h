/*
 * DistanceSensor.h
 *
 *  Created on: Dec 5, 2022
 *      Author: sebas
 */

#ifndef SRC_P3_DISTANCE_METER_DISTANCESENSOR_H_
#define SRC_P3_DISTANCE_METER_DISTANCESENSOR_H_

#include "main.h"

void DistanceSensor_init();
void requestDistance();
int calcDistance(int RTT);
int calcAverage(int arr_len, uint8_t ADC_Vals[]);



#endif /* SRC_P3_DISTANCE_METER_DISTANCESENSOR_H_ */
