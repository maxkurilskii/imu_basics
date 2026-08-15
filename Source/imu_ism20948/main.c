#include "common.h"
#include "spi_nonblocking.h"
#include "imu_ism20948.h"
//#include "uart_unit.h"

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
//    delay_ms(2000);
    SPI1_Init();
    delay_ms(100);
    uint8_t imu_err1 = 0, imu_err2 = 0;
//    spi_write(PWR_MGMT_1_ADD, PWR_MGMT_1_DEVICE_RESET);
//    delay_ms(100);
//    imu_err1 = test_imu_startup();
//    if (imu_err1){
//        toggle_led(LED3);
//        return 0;
//    }
//    
//    imu_err2 = test_imu_startup();
//    if (imu_err2){
//        toggle_led(LED2);
//        return 0;
//    }
    Imu20948_Init();
   
//    spi_write(REG_BANK_SEL_ADD, 0x00);      // Bank0
//    spi_read_async(WHO_AM_I, 1);    // ожидаем 0xEA
//    while(cur_spi_state != DATA_READY) __NOP();
//    transmit_byte_usart3_debug(spi_byte_read);
//    if (spi_byte_read != 0xEA) return 0;
    
    
//    register_spi_rx_callback(convert_imu_meas); 
//    imu_timer_start();
//    usart3_timer_start();
    
	while(1){
//        toggle_led(LED2);
//        delay_ms(500);
    }
	
}



