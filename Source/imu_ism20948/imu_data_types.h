#ifndef IMU_DATA_TYPES_H
#define IMU_DATA_TYPES_H

#include "common.h"

//base imu data type struct
typedef struct{
    float	acc_meas[3];
    float	gyro_meas[3];
    float   mag_meas[3];
    uint32_t   timestamp_ms;
}imu_data_t;

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

