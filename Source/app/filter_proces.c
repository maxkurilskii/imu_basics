#include "filter_proces.h"

FusionAhrs ahrs;

imu_orient_t buf_a = { .roll = 0.0f, .pitch = 0.0f, .yaw = 0.0f, 
                         .timestamp_ms = 0};

imu_orient_t buf_b = { .roll = 0.0f, .pitch = 0.0f, .yaw = 0.0f, 
                         .timestamp_ms = 0};

imu_orient_t* eu_write_buf = &buf_a;   
imu_orient_t* eu_read_buf = &buf_b;
                         
//FusionEuler write_buf = {.angle = {0.0, 0.0, 0.0}};
//FusionEuler read_buf = {.angle = {0.0, 0.0, 0.0}};


void swap_orientation_buffers(void){
    imu_orient_t* temp = eu_write_buf;
    eu_write_buf = eu_read_buf;
    eu_read_buf = temp;
}

imu_orient_t* get_orientation(void){
    return eu_read_buf;
}

void update_orientation(imu_scaled_t* imu_meas){
    float gx = imu_meas->gyro_meas[0];
    float gy = imu_meas->gyro_meas[1];
    float gz = imu_meas->gyro_meas[2];
    FusionVector gyroscope = {gx, gy, gz};
    
    float ax = imu_meas->acc_meas[0];
    float ay = imu_meas->acc_meas[1];
    float az = imu_meas->acc_meas[2];
    FusionVector accelerometer = {ax, ay, az};
    
    float mx = imu_meas->mag_meas[0];
    float my = imu_meas->mag_meas[1];
    float mz = imu_meas->mag_meas[2];
    FusionVector magnetometer = {mx, my, mz};

    FusionAhrsUpdate(&ahrs, gyroscope, accelerometer, magnetometer);
    FusionEuler euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));
    eu_write_buf->roll = euler.angle.roll;
    eu_write_buf->pitch = euler.angle.pitch;
    eu_write_buf->yaw = euler.angle.yaw;
    eu_write_buf->timestamp_ms = imu_meas->timestamp_ms;
    swap_orientation_buffers();
}


void Madgwick_Filter_Init(void){
    /*Madgwick filter cfg*/
    //FusionAhrs ahrs;
    FusionAhrsInitialise(&ahrs);
    FusionAhrsSettings settings = fusionAhrsDefaultSettings;
    settings.sampleRate = 200; //
    FusionAhrsSetSettings(&ahrs, &settings);
}

void TIM1_TRG_COM_TIM11_IRQHandler(void){
//    if (TIM11->SR & (1 << TIM_SR_UIF)){
//        
//    }

}

void Timer11_Init(void){
//    Enable clock on tim11 from apb2 (108 Mhz)
    RCC->APB2ENR |= RCC_APB2ENR_TIM11EN;
    
    //prescaler: 108 Mhz / 108 = 1Mhz timer clock freq
    TIM11->PSC = 107;
    
    //auto-reload: filter_period_ms * 1000 < 65535
    TIM11->ARR = FILTER_PERIOD_MS * 1000 - 1;
    
    //interrupt on update enable
    TIM11->DIER |= 1 << TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM1_TRG_COM_TIM11_IRQn);
    
    //generate update event to update arr, presc register values
    TIM11->EGR |= 1 << TIM_EGR_UG;
    TIM11->SR &= ~(1 << TIM_SR_UIF);
    
}



void filter_timer_start(void){
    TIM11->CR1 |= TIM_CR1_CEN;
}

void filter_timer_stop(void){
    TIM11->CR1 &= ~TIM_CR1_CEN;
    TIM11->SR &= ~TIM_SR_UIF; //???
    TIM11->CNT = 0;
}

