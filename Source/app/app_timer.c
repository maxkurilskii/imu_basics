#include "app_timer.h"

uint32_t prev_timer_ticks = 0;
volatile uint32_t ovf_cnt = 0;


uint32_t App_GetTicks(void){
    uint32_t ovf = 0, cur_ticks = 0;
    __disable_irq();
    ovf = ovf_cnt;
    cur_ticks = TIM10->CNT;
    __enable_irq();
    
    uint32_t total_ticks = ovf * 65536 + cur_ticks;
    if (total_ticks < prev_timer_ticks){
//        toggle_led(LED1);
        prev_timer_ticks = (ovf + 1) * 65536 + cur_ticks;
    }
    else{  
        prev_timer_ticks = total_ticks; 
    }
    return prev_timer_ticks;
}
    
void TIM1_UP_TIM10_IRQHandler(void){
    
    if(TIM10->SR & TIM_SR_UIF){
        TIM10->SR &= ~TIM_SR_UIF;
        ovf_cnt++;     
    }
}


void APP_Timer10_Init(void){
    // Enable TIM10 clock from APB2 (108 Mhz)
    RCC->APB2ENR |= RCC_APB2ENR_TIM10EN;
    // Prescaler: 108 Mhz / 54000 =  2 kHz -> Tcnt_tick = 500 us
    TIM10->PSC = 53999;
    // AUto-reload: 65535 * Tcnt_tick ~ 33 sec
    TIM10->ARR = 65535;
    // Update Prescaler and ARR registers before start
    TIM10->EGR |= TIM_EGR_UG;
    TIM10->SR &= ~TIM_SR_UIF; //clear flag!
    // Enable interrupts
    TIM10->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM1_UP_TIM10_IRQn);
    
    TIM10->CNT = 0;  
    TIM10->CR1 |= TIM_CR1_CEN;
}

void App_Timer_Start(void){
    TIM10->CNT = 0;  
    TIM10->CR1 |= TIM_CR1_CEN;
    
}


void App_Timer_Stop(void){
    TIM10->CR1 &= ~TIM_CR1_CEN;
    TIM10->EGR |= TIM_EGR_UG;
    TIM9->SR &= ~TIM_SR_UIF; //clear flag!?
    TIM10->CNT = 0;  
    
}
