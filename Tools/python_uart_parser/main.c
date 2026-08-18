#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#define IMU_WRITE_BUF       0
#define IMU_READ_BUF        1

typedef struct{
    float	acc_meas[3];
    float	gyro_meas[3];
    float   mag_meas[3];
    float   timestamp;
}imu_data_t;

imu_data_t imu_raw_meas_a = {
    .acc_meas = {0},
    .gyro_meas = {0},
    .mag_meas = {0},
    .timestamp = 0
};

imu_data_t imu_raw_meas_b = {
    .acc_meas = {0},
    .gyro_meas = {0},
    .mag_meas = {0},
    .timestamp = 0
};

void swap_buffers(imu_data_t** arr){
    imu_data_t* temp = *arr;
    *(arr) = *(arr+1);
    *(arr+1) = temp;
}

int main(void){
    /*Pointer array*/
    imu_data_t* imu_raw_measurements[2] = {0};
    imu_raw_measurements[IMU_WRITE_BUF] = &imu_raw_meas_a;
    printf("%p\n", imu_raw_measurements[IMU_WRITE_BUF]);
    imu_raw_measurements[IMU_READ_BUF] = &imu_raw_meas_b;
    printf("%p\n", imu_raw_measurements[IMU_READ_BUF]);
    imu_raw_measurements[IMU_WRITE_BUF]->acc_meas[0] = 10.0f;
    imu_raw_measurements[IMU_READ_BUF]->acc_meas[0] = 5.0f;
    printf("write buf %.3f\n", imu_raw_measurements[IMU_WRITE_BUF]->acc_meas[0]);
    printf("read buf %.3f\n", imu_raw_measurements[IMU_READ_BUF]->acc_meas[0]);
    printf("-- SWAP --\n");
    swap_buffers(imu_raw_measurements);
    printf("write buf %f\n",imu_raw_measurements[IMU_WRITE_BUF]->acc_meas[0]);
    printf("read buf %f\n",imu_raw_measurements[IMU_READ_BUF]->acc_meas[0]);


    /*Raw bytes from imu to float values convertation*/
    // uint8_t g_l = 0xC4;
    // uint8_t g_h = 0xE5;
    // float gyro_x = (int16_t)((uint16_t)g_h << 8 | (uint16_t)g_l) / 65.5f;
    // printf("Gyro X axis data: %f\n", gyro_x);

    /*Consequent coping of bytes of float number from one buffer to another*/
    // float mag_dummy[3] = {4912, 4912, 4912};
    // float imu_resp_mag[3] = {0};
    // uint8_t *p_mag = (uint8_t*)imu_resp_mag
    // memcpy(imu_resp_mag, mag_dummy, 12);
    // for (uint8_t i = 0; i < 3; i++){
    //     printf("mag data(%d): %.3f\n", i, imu_resp_mag[i]);
    // }
    return 0;
}