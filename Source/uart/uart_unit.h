#ifndef UART_UNIT_H
#define UART_UNIT_H

#include "common.h"
#include "imu_ism20948.h"
#include "filter_proces.h"
#include "crc16.h"

#define UART_TX_PERIOD_MS	20

typedef struct{
	uint8_t start_byte;
	uint8_t cmd;
	uint8_t data_len;
    uint8_t timestamp[2];
	uint8_t accelerometer_data[12];
	uint8_t gyroscope_data[12];
    uint8_t crc[2];
}imu_msg_t;

typedef enum{
    USART3_TRANSMITING,
    USART3_FREE
}usart3_state_t;

extern volatile usart3_state_t cur_usart3_state;

void USART3_Init(void);
void DMA1_Stream3_IRQHandler(void);
void dma_clear_flags(void);

void Timer10_Init(void);
void TIM1_UP_TIM10_IRQHandler(void);
void usart3_timer_start(void);
void usart3_timer_stop(void);

void transmit_byte_usart3(uint8_t data);
void transmit_imu_meas_usart3(imu_scaled_t* imu_meas);
void transmit_imu_orient_usart3(imu_orient_t* euler_meas);

void transmit_byte_usart3_debug(uint8_t data);

#endif

