/*
 * DM.h
 *
 *  Created on: Nov 30, 2022
 *      Author: Seb
 */

#ifndef SRC_P3_DISTANCE_METER_DM_H_
#define SRC_P3_DISTANCE_METER_DM_H_

#include "main.h"

void GUI_init();

void GUI_2D_init();

void printDistance(int centimeters, int inches);

void print2Distance(uint16_t array2D[]);

int calc_Distance(int RTT);

#endif /* SRC_P3_DISTANCE_METER_DM_H_ */
