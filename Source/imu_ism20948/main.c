#include "common.h"
#include "uart_unit.h"
#include "spi_nonblocking.h"
#include "imu_ism20948.h"

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
	SysClockInit(); //make sys clock initialization
	SysTickInit();
	LEDs_Init();
    USART3_Init();
    SPI1_Init();
    
    //startup delay for PC to begin monitoring STM UART output 
    delay_ms(3000);
    debug_imu_startup();
    // last output on PC: ea 41 02 02 30 02 20 01 1b 02 00 02 
    // 2 write attempts are typically required to configure the IMU registers
    // using spi write (dma tx config + blocking with flag)
    
    return 0;
}



