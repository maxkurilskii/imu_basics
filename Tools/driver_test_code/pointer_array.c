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
    uint32_t   timestamp;
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


imu_data_t* write_buf_p = &imu_raw_meas_a;
imu_data_t* read_buf_p = &imu_raw_meas_b;


void swap_buffers(imu_data_t** arr){
    imu_data_t* temp = *arr;
    *(arr) = *(arr+1);
    *(arr+1) = temp;
}

void swap_buffers_content(){
    imu_data_t temp = imu_raw_meas_a;
    imu_raw_meas_a = imu_raw_meas_b;
    imu_raw_meas_b = temp;
}

void get_measurement(imu_data_t data){
    // data is new meas
    imu_raw_meas_a = data;
    swap_buffers_2();
}

void print_buf_content(imu_data_t* buf){
        printf("\tX: %f, Y: %f, Z:%f\n", buf->acc_meas[0], buf->acc_meas[1], 
            buf->acc_meas[2]);
        printf("\tX: %f, Y: %f, Z:%f\n", buf->acc_meas[0], buf->acc_meas[1], 
        buf->acc_meas[2]);
        printf("\tX: %f, Y: %f, Z:%f\n", buf->acc_meas[0], buf->acc_meas[1], 
        buf->acc_meas[2]);
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

    return 0;
}