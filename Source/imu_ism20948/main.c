#include "common.h"
#include "imu_ism20948.h"
#include "uart_unit.h"

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

void swap_imu_buffers(imu_data_t** pointer_array){
    imu_data_t* temp = *pointer_array; 
    *(pointer_array) = *(pointer_array+1);
    *(pointer_array+1) = temp;
}

int main(void){
	SysClockInit(); //make sys clock initialization
	SysTickInit();
	LEDs_Init();
    USART3_Init();
    SPI1_Init();
    uint8_t imu_resp = 0;
   
    Imu20948_Init();
  
    imu_timer_start();
    usart3_timer_start();
    
	while(1){
        if (cur_spi_state == READING){
            //get_raw_imu_meas(&imu_raw_meas);
            get_raw_imu_meas(imu_raw_meas[WRITE_PART]);
            //get_register_value(WHO_AM_I);
            swap_imu_buffers(imu_raw_meas); //swap read and write meas parts addresses
            //change state 
            cur_spi_state = FREE;
        }   
    }
}



