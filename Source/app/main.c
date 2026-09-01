#include "common.h"
#include "uart_unit.h"
#include "imu_ism20948.h"
//#include "filter_proces.h"

volatile uint32_t msCounter = 0;
uint32_t start_delay = 0;
uint32_t start_uart = 0;

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
	LEDs_Init();
    USART3_Init();
    SPI1_Init();
    uint8_t imu_resp = 0;
    Imu20948_Init();
//  Madgwick_Filter_Init();
    
    /*init imu calibration */
    if (EXECUTE_CALIB){
        calibrate_gyro();
        //calibrate_accel(); not impl yet
        //calibrate_mag(); not impl yet    
    }
    
    
    /* run imu and uart */
    imu_timer_start();
    usart3_timer_start();   
    
	while(1){
        if (cur_spi_state == READING){
            //get_register_value(WHO_AM_I); 
            update_imu_meas(); //blocking!!!
            imu_scaled_t* scaled_meas = get_imu_scaled_meas();
            get_corrected_imu_meas(scaled_meas);       
            //change state 
            cur_spi_state = FREE;
        }   
        
    }
}



