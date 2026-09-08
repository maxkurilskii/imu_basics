#ifndef APP_TIMER_H
#define APP_TIMER_H

#include "common.h"

#define TICK_PERIOD_US 500

static inline uint32_t ticks_to_us(uint32_t ticks){
    return ticks * TICK_PERIOD_US;
}

static inline float ticks_to_ms(uint32_t ticks){
    return (float)ticks * TICK_PERIOD_US * 0.001;
}

static inline float ticks_to_sec(uint32_t ticks){
    return (float)ticks * TICK_PERIOD_US * 0.000001;
}



void APP_Timer10_Init(void);
void App_Timer_Start(void);
void App_Timer_Stop(void);
uint32_t App_GetTicks(void);

#endif