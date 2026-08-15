#include "timers_unit.h"


void Timer9_Init(void){
    //Enable tim9(16bit timer) clock from APB2 (108 MHz)
    RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
    
    //Prescaler = 107: 108 Mhz / (53 + 1) = 1 Mhz (1us per tick)
    TIM9->PSC = 53; 
    
    //ARR is TIM9_PERIOD_MS / Tcnt_tick = TIM9_PERIOD_MS * Fcnt_tick
    uint16_t arr_val = TIM9_PERIOD_MS * 1000 - 1;
     //in case of overflow(65535 max) tim9_period = 10 ms (100 Hz)
    if (arr_val > 65535) {
        toggle_led(LED2);
        arr_val = 10000-1;
    }
    TIM9->ARR = arr_val; 
    
    //Update Prescaler and ARR registers before start
    TIM9->EGR |= TIM_EGR_UG;
    TIM9->SR &= ~TIM_SR_UIF; //cleat flag!
    
    //Enable interrupts
    TIM9->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);
    
    
    //DO NOT Start timer in INIT!
    //TIM9->CR1 |= TIM_CR1_CEN;
}

void imu_timer_start(){
    TIM9->CR1 |= TIM_CR1_CEN;
}

void imu_timer_stop(){
    TIM9->CR1 &= ~TIM_CR1_CEN;
    //need???
    if (TIM9->SR & TIM_SR_UIF) TIM9->SR &= ~TIM_SR_UIF; 
}