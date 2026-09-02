#ifndef FILTER_PROCES_H
#define FILTER_PROCES_H

#include "common.h"
#include "imu_ism20948.h"
#include "Fusion.h"

#define FILTER_PERIOD_MS    10

void Timer11_Init(void);
void TIM1_TRG_COM_TIM11_IRQHandler(void);
void filter_timer_start(void);
void filter_timer_stop(void);

void Madgwick_Filter_Init(void);

void swap_orientation_buffers(void);
void update_orientation(imu_scaled_t* imu_meas);
imu_orient_t* get_orientation(void);


#endif