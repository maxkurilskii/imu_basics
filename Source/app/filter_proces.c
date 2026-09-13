#include "filter_proces.h"

FusionAhrs ahrs;
FusionAhrsFlags ahrs_flags;

imu_orient_t euler_buf = { .roll = 0.0f, .pitch = 0.0f, .yaw = 0.0f, 
                         .time_us = 0};


//uint32_t* meas_time = 0;
  

void Madgwick_Filter_Init(void){
    /*Madgwick filter cfg*/
    FusionAhrsInitialise(&ahrs);
    FusionAhrsSettings settings = {
        .sampleRate = SAMPLE_RATE,
        .convention = FusionConventionEnu, // FusionConventionEnu FusionConventionNwu FusionConventionNed,
        .gain = GAIN_BETA,
        .gyroscopeRange = 500.0f,
        .accelerationRejection = 20.0f,
        .magneticRejection = 20.0f,
        .rejectionTimeout = 5.0f
    };
    FusionAhrsSetSettings(&ahrs, &settings);
    
    ahrs_flags = FusionAhrsGetFlags(&ahrs);
    if (ahrs_flags.startup)
        set_led(LED1);
    else
        reset_led(LED1);
    //
    if (ahrs_flags.overrangeRecovery)
        set_led(LED2);
    else
        reset_led(LED2);
    //
    if (ahrs_flags.accelerationRecovery)
        set_led(LED3);
    else
        reset_led(LED3);
}

void update_orientation(imu_sample_t* meas){
    //Set real time delta
    //    uint32_t deltaTime = (meas->time_us) - meas_time;
    //meas_time = &meas->time_us;
    //FusionAhrsSetSamplePeriod(&ahrs, deltaTime);
    FusionVector gyroscope = {meas->gyro[0], 
                              meas->gyro[1], 
                              meas->gyro[2]};
    FusionVector accelerometer = {meas->accel[0], 
                                  meas->accel[1], 
                                  meas->accel[2]};

    FusionVector magnetometer = {meas->mag[0], 
                                 meas->mag[1],
                                 meas->mag[2]};
    FusionAhrsUpdate(&ahrs, gyroscope, accelerometer, magnetometer);

    //FusionAhrsUpdateNoMagnetometer(&ahrs, gyroscope, accelerometer);

//    ahrs_flags = FusionAhrsGetFlags(&ahrs);
//    if (ahrs_flags.startup)
//        set_led(LED1);
//    else
//        reset_led(LED1);
    //
//    if (ahrs_flags.overrangeRecovery)
//        set_led(LED2);
//    else
//        reset_led(LED2);
    //
//    if (ahrs_flags.accelerationRecovery)
//            set_led(LED3);
//    else
//        reset_led(LED3);
}


imu_orient_t* get_euler_angles(uint32_t* meas_time){
    // Convertion from quaternion to euler angle 
    const FusionEuler euler = FusionQuaternionToEuler(FusionAhrsGetQuaternion(&ahrs));
    // Save data in euler_buf
    euler_buf.roll = euler.angle.roll;
    euler_buf.pitch = euler.angle.pitch;
    euler_buf.yaw = euler.angle.yaw;
    euler_buf.time_us = *meas_time;
    return &euler_buf;
}




