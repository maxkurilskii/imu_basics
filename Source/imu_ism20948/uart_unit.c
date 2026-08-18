#include "uart_unit.h"

volatile usart3_state_t cur_usart3_state;
volatile uint8_t tx_buffer[100] = {0};

void TIM1_UP_TIM10_IRQHandler(void){
    
    if(TIM10->SR & TIM_SR_UIF){
        TIM10->SR &= ~TIM_SR_UIF;
        if(cur_usart3_state == USART3_FREE){
            cur_usart3_state = USART3_TRANSMITING;
            //transmit_byte_usart3(whoAmIValue);
            //transmit_mag_meas_usart3(mag_meas);
            //transmit_acc_gyro_meas_usart3(acc_meas, gyro_meas);
            transmit_imu_meas_usart3(imu_raw_meas[READ_PART]);
        }
    }

}

void transmit_byte_usart3(uint8_t data){
    cur_usart3_state = USART3_TRANSMITING;
    DMA1_Stream3->CR &= ~DMA_SxCR_EN;
    dma_clear_flags();
    DMA1_Stream3->NDTR = 1;
    tx_buffer[0] = data;
    //Start transmitting
    DMA1_Stream3->CR |= DMA_SxCR_EN;
    
}

void transmit_byte_usart3_debug(uint8_t data){
    while (!(USART3->ISR & USART_ISR_TXE));
    USART3->TDR = data;
    //while (!(USART3->ISR & USART_ISR_TC));
}

void transmit_imu_meas_usart3(imu_data_t* imu_s){
        DMA1_Stream3->CR &= ~DMA_SxCR_EN;
        dma_clear_flags();
        DMA1_Stream3->NDTR = 43;
		uint8_t *p_acc  = (uint8_t*)imu_s->acc_meas;
		uint8_t *p_gyro = (uint8_t*)imu_s->gyro_meas;
        uint8_t *p_mag  = (uint8_t*)imu_s->mag_meas;
        uint8_t *p_time = (uint8_t*)&(imu_s->timestamp_ms);
	
		tx_buffer[0] = 0x23; //start byte
		tx_buffer[1] = 0x42; //imu cmd code
		tx_buffer[2] = 0x28; //length of data =  36 bytes(ac+gyro+mag) + 4 timestmp
        //fill tx_buffer with measuremnets: 3-14(12 bytes) - accel
		for(uint8_t i = 0; i < 12; i++){
			tx_buffer[3+i]   = *(p_acc + i);
            tx_buffer[15+i]  = *(p_gyro + i);
            tx_buffer[27+i] = *(p_mag + i);
		}
        for(uint8_t i = 0; i < 4; i++) 
            tx_buffer[39+i] = *(p_time + i);
        
        //Start transmitting
        DMA1_Stream3->CR |= DMA_SxCR_EN;
}

void transmit_acc_gyro_meas_usart3(float* acc_meas, float* gyro_meas){
		uint8_t *p_acc = (uint8_t*)acc_meas;
		uint8_t *p_gyro = (uint8_t*)gyro_meas;
	
		tx_buffer[0] = 0x23; //start byte
		tx_buffer[1] = 0x42; //imu cmd code
		tx_buffer[2] = 0x18; //length of data =  24 bytes
        //fill tx_buffer with measuremnets: 3-14(12 bytes) - accel
		for(uint8_t i = 0; i < 12; i++){
			tx_buffer[3+i] = *(p_acc + i);
            tx_buffer[15+i] = *(p_gyro + i);
		}
 
		//transmit all collected bytes 
		for(uint8_t i = 0; i < 27; i++){
			transmit_byte_usart3(tx_buffer[i]);
		}
		
		//Enable interrupt on TXE (TDR is empty and ready to take data)
		//USART3->CR1 |=  USART_CR1_TXEIE;
}

void transmit_mag_meas_usart3(float* mag_meas){
		uint8_t *p_mag = (uint8_t*)mag_meas;
	
		tx_buffer[0] = 0x23; //start byte
		tx_buffer[1] = 0x42; //imu cmd code
		tx_buffer[2] = 0x0C; //length of data =  12 bytes
        //fill tx_buffer with measuremnets: 3-14(12 bytes) - mag
		for(uint8_t i = 0; i < 12; i++){
			tx_buffer[3+i] = *(p_mag + i);
		}
		//transmit all collected bytes 
		for(uint8_t i = 0; i < 15; i++){
			transmit_byte_usart3(tx_buffer[i]);
		}
		
} 


void DMA1_Stream3_IRQHandler(void){
    if (DMA1->LISR & DMA_LISR_TCIF3){
        dma_clear_flags();
        DMA1_Stream3->CR &= ~DMA_SxCR_EN;
        cur_usart3_state = USART3_FREE;
    }
}


void USART3_Init(void){
    
    /* ---------------- DMA1 for USART3 init -------------- */
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
    
    DMA1_Stream3->CR &= ~DMA_SxCR_EN;
    while(DMA1_Stream3->CR & DMA_SxCR_EN);
    
    //Enable usart3 TX channel 4 stream 3
    DMA1_Stream3->CR |= 4U << DMA_SxCR_CHSEL_Pos;
    
    //MSize = PSize = 8 bit; Mem incr enabled; mem -> perif; TC int enable
    DMA1_Stream3->CR |= DMA_SxCR_MINC | DMA_SxCR_TCIE | (1U << DMA_SxCR_DIR_Pos);
    NVIC_EnableIRQ(DMA1_Stream3_IRQn);
    
    DMA1_Stream3->PAR = (uint32_t)&USART3->TDR;
    DMA1_Stream3->M0AR = (uint32_t)tx_buffer;
	/* ------------- USART3 initialization ---------------- */
	
	//Enable USART3 on APB1(54 Mhz)
	RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
	
    //Enable clock on GPIOD from AHB1
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
	//Pin GPIOD8 (USART3_TX) connected to ST-LINK: alt mode + pull up + no push-pull
	GPIOD->MODER |= 2U << GPIO_MODER_MODER8_Pos;
    GPIOD->AFR[1] |= 7U << GPIO_AFRH_AFRH0_Pos;

	//Baud rate = 115200 + oversampling = 16(over8 = 0)
	//54000000/(16 * 115200) = 29.29687 -> matisa = 29; frac = 0.29 * 16 = 4.75 = 5
    //USART3->BRR = 54000000UL / 115200UL;
    USART3->BRR =(29U << USART_BRR_DIV_MANTISSA_Pos) | (5U << USART_BRR_DIV_FRACTION_Pos);
   
    //Baud rate = 921600 + oversampling = 16(over8 = 0)
	//54000000/(16 * 921600) = 3.66 -> matisa = 3; frac = 0.66 * 16 ~ 11    
    //USART3->BRR = (3U << USART_BRR_DIV_MANTISSA_Pos) | (11U << USART_BRR_DIV_FRACTION_Pos);
	
    //Word len = 8bit (M1=M2=0)+enable UART3+enable UART3 transmit and receive 
	USART3->CR1 |= USART_CR1_UE | USART_CR1_TE;// | USART_CR1_RE;
	USART3->CR3 |= USART_CR3_DMAT; //use dma for tx
	
    /* ------------- Timer 10 for USART3 init -------------- */
    Timer10_Init();
 
}



    
void Timer10_Init(void){
    //Enable TIM10 clock from APB2 (108 Mhz)
    RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
    //Prescaler: 108 Mhz / 54000 = 2 kHz = Ftim_tick 
    TIM10->PSC = 53999;
    //AUto-reload: (UART_TX_PERIOD_MS / 1000) * Ftim_tick  = ARR
    //in case of ovf: max period is 1 sec
    uint16_t arr_val  = UART_TX_PERIOD_MS * 2 - 1;
    if (arr_val > 65535) {
        toggle_led(LED1);
        arr_val = 1999; // 1000 ms * 2 -1
    }
    TIM10->ARR = arr_val;
    
    //Update Prescaler and ARR registers before start
    TIM10->EGR |= TIM_EGR_UG;
    TIM10->SR &= ~TIM_SR_UIF; //cleat flag!
    
    //Enable interrupts
    TIM10->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
    
    
    //DO NOT Start timer in INIT!
    //TIM10->CR1 |= TIM_CR1_CEN;
    
}

void usart3_timer_start(void){
    cur_usart3_state = USART3_FREE;
    TIM10->CR1 |= TIM_CR1_CEN;
}

void usart3_timer_stop(void){
    TIM10->CR1 &= ~TIM_CR1_CEN;
    TIM10->SR &= ~TIM_SR_UIF; 
    TIM10->CNT = 0;
}


void dma_clear_flags(void){
    DMA1->LIFCR |= DMA_LIFCR_CFEIF3
        | DMA_LIFCR_CDMEIF3
        | DMA_LIFCR_CTEIF3
        | DMA_LIFCR_CHTIF3
        | DMA_LIFCR_CTCIF3;
}

