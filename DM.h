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

void update_DC(int min, int max, int avg);

void update_AC(int Vrms, int ptop, int freq, int DCOffset);

int calc_RMS(int ptop);

void clear_DC();

void clear_AC();

int calc_Distance(int RTT);

#endif /* SRC_P3_DISTANCE_METER_DM_H_ */
