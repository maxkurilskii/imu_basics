#ifndef UART_UNIT_H
#define UART_UNIT_H

#include "common.h"
#include "imu_ism20948.h"
#include "filter_proces.h"
#include "crc16.h"

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
    USART3_FREE,
    USART3_READY,
    USART3_TRANSMITING
}usart3_state_t;

extern volatile usart3_state_t cur_usart3_state;

void USART3_Init(void);

void dma_clear_flags(void);

void transmit_byte_usart3(uint8_t data);
void transmit_imu_sample_usart3(imu_sample_t* imu_meas);
void transmit_imu_orient_usart3(imu_orient_t* euler_meas);

void transmit_byte_usart3_debug(uint8_t data);

#endif

