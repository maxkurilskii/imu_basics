#ifndef COMMON_H
#define COMMON_H

#include <stm32f767xx.h>
#include <stdbool.h>
#include <string.h>

typedef struct{
    float	acc_meas[3];
    float	gyro_meas[3];
    float   mag_meas[3];
    float   timestamp;
}imu_data_t;

#include "system_clock_init.h" 
#include "led_unit.h"
#include "uart_unit.h"

void SysTick_Handler(void);
void delay_ms(uint16_t milis);

extern volatile uint32_t msCounter;
#endif

