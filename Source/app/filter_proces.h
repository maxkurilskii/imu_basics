#ifndef FILTER_PROCES_H
#define FILTER_PROCES_H

#include "common.h"
#include "imu_ism20948.h"
#include "Fusion.h"

#define SAMPLE_RATE 200.0f //should be equal to imu meas upd rate
#define GAIN_BETA   1.2f  //gain or beta coef in ahrs

void Madgwick_Filter_Init(void);
void update_orientation(imu_sample_t* meas);
imu_orient_t* get_euler_angles(uint32_t* meas_time);


#endif