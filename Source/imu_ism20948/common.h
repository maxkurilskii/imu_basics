#ifndef COMMON_H
#define COMMON_H

#include <stm32f767xx.h>
#include <stdbool.h>
#include <string.h>

#include "system_clock_init.h" 
#include "led_unit.h"
#include "imu_data_types.h"

void SysTick_Handler(void);
void delay_ms(uint16_t milis);

extern volatile uint32_t msCounter;
#endif

