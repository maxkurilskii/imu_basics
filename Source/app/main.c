#include "common.h"
#include "uart_unit.h"
#include "imu_ism20948.h"
#include "filter_proces.h"

volatile uint32_t msCounter = 0;
uint32_t start_delay = 0;

uint16_t tmp_rx_buffer = 0;
uint8_t mag_device_id = 0;

void SysTick_Handler(void){
	msCounter++;
}

void delay_ms(uint16_t millis){
	start_delay = msCounter;
	while ((msCounter - start_delay) < millis){
		__NOP();
	}
}


int main(void){
	SysClockInit();
	SysTickInit();
    APP_Timer10_Init();
	LEDs_Init();
    USART3_Init();
    IMU20948_Init();
    Madgwick_Filter_Init();
    delay_ms(3000);
    
    /* ------------------------------ */
    
    
    imu_sample_t imu_sample = {.accel = {0}, 
                               .gyro = {0}, 
                               .mag = {0}, 
                               .time_us = 0};
    
    uint32_t imu_time_us = 0;
    uint8_t imu_sample_cnt = 0;
                                
    /* run timers  */
    App_Timer_Start();
    IMU_Timer_Start();
                               
	while(1){
        if (cur_spi_state == SPI_READY){
            cur_spi_state = SPI_READING;
            imu_time_us = ticks_to_us(App_GetTicks()); 
            // save time + measurements in imu sample
            imu_sample.time_us = imu_time_us;
            //read measurements using dma
            start_reading_imu_measurement();
        }
        
        if(cur_spi_state == SPI_DATA_READY){
            imu_scaled_meas_t* meas = get_imu_corrected_measurement();
            for(uint16_t i = 0; i < 3; i++){
                imu_sample.accel[i] = meas->s_accel[i];
                imu_sample.gyro[i] = meas->s_gyro[i];
                imu_sample.mag[i] = meas->s_mag[i];
            }
            imu_sample_cnt++;   
            update_orientation(&imu_sample);
            cur_spi_state = SPI_FREE;
        }
        
        if ((imu_sample_cnt * TIM9_PERIOD_MS >= UART_TX_PERIOD_MS) &&
             (cur_usart3_state == USART3_FREE) && (cur_spi_state == SPI_FREE)){
                imu_sample_cnt = 0;
                //transmit_imu_sample_usart3( &imu_sample );
                 transmit_imu_orient_usart3( get_euler_angles() );
        }
    }
}


