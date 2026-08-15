#ifndef TIMERS_UNIT_H
#define TIMERS_UNIT_H

#include "common.h"

#define TIM9_PERIOD_MS  1

void TIM1_BRK_TIM9_IRQHandler(void);
void Timer9_Init(void);
void imu_timer_start(void);
void imu_timer_stop(void);

#endif
