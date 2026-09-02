#ifndef IMU_DATA_TYPES_H
#define IMU_DATA_TYPES_H

#include "common.h"


//base imu data type struct
typedef struct{
    float	    acc_meas[3];
    float	    gyro_meas[3];
    float       mag_meas[3];
    uint32_t   timestamp_ms;
}imu_scaled_t;


typedef struct{
    float bias[3];
}gyro_calib_info_t;

typedef struct{
    float bias[3];
    float mtx[3][3]; //matrix
}accel_calib_info_t;

typedef struct{
    //hard iron
    float bias[3];
    //soft iron + scale + no-orthoganality 
    float mtx[3][3]; //matrix
}mag_calib_info_t;

typedef struct{
    float roll;
    float pitch;
    float yaw;   
    uint32_t   timestamp_ms;    
}imu_orient_t;

//typedef struct{
//    int16_t 	acc_meas[3];
//    int16_t	    gyro_meas[3];
//    int16_t     ag_meas[3];
//    uint32_t    timestamp_ms;
//}imu_raw_data_t;


//typedef struct{
//    float	acc_meas[3];
//    float	gyro_meas[3];
//    float   mag_meas[3];
//    float   timestamp;
//}imu_filtered_data_t;


#endif

