#include "imu_ism20948.h"

uint8_t whoAmIValue = 0;
uint32_t tim9_counter = 0;

//double buffers
imu_scaled_t buffer_a = {
    .acc_meas = {0},
    .gyro_meas = {0},
    .mag_meas = {0},
    .timestamp_ms = 0
};

imu_scaled_t buffer_b = {
    .acc_meas = {0},
    .gyro_meas = {0},
    .mag_meas = {0},
    .timestamp_ms = 0
};

// addresses of measurement buffers read and write 
imu_scaled_t* write_buf = &buffer_a; //buffer to write requesting meas
imu_scaled_t* read_buf = &buffer_b; //buffer to read meas from other periph


gyro_calib_info_t gyro_cal_pars = {
    .bias = {0}
};


const accel_calib_info_t acc_cal_pars = {
    .bias = {0},
    .mtx = {0},
};

const mag_calib_info_t mag_cal_pars = {
//    .bias = {-39.011639, 9.399178, 9.811616},  
//    .mtx = {
//                {1.47478077, 0.06500727,  0.02306534}, 
//                {0.06500727,  1.40433028, -0.01804741}, 
//                {0.02306534, -0.01804741,  1.49679913}}
    
//    .bias = { 0.549387, -0.532294,  4.648358},  
//    .mtx = {
//                {0.632762, -0.100000, -0.029320},
//                {-0.100000, 0.740875, -0.031670},
//                {-0.029320, -0.031670, 0.634742}}

    .bias = {-25.612981,   8.784697, 5.968302},  
        .mtx = {
             {1.464867, 0.107111, 0.087680},
             {0.107111, 1.299269, 0.195612},
             {0.087680, 0.195612, 1.579408}} 
    
//    .bias = {-18.232859, -3.765254, 8.891346},  
//    .mtx = {
//             {0.631648, -0.033543, -0.049528},
//            {-0.033543, 0.789297, -0.069299},
//            {-0.049528, -0.069299, 0.660749}} 
};


void powerup_imu(void){ 
    uint8_t reg_value = 0;//var to verify written data 
	//reset all registers to def state
    spi_write(PWR_MGMT_1_ADD, PWR_MGMT_1_DEVICE_RESET);
    delay_ms(100); //NECCESSARY TO WAIT pin timout read/write !!!
    
    spi_write(REG_BANK_SEL_ADD, (0x00 << REG_BANK_SEL_USER_BANK_Pos));     
    spi_read(REG_BANK_SEL_ADD, &reg_value, 1);    // ожидаем 0x00
    transmit_byte_usart3_debug(reg_value);

    //disable sleep mode + select clock PLL to run gyro in best performance
    spi_write(PWR_MGMT_1_ADD, (PWR_MGMT_1_SLEEP_OFF | PWR_MGMT_1_CLKSEL_PLL));
    delay_ms(1); //NECCESSARY TO WAIT A BIT !!!

    //chose SPI mode only (immediately after restart) + enable i2c master
	spi_write(USER_CTRL_ADD, 
                           (1U << USER_CTRL_I2C_IF_DIS_Pos) | 
                           (1U << USER_CTRL_I2C_MST_EN_Pos));
                           
    spi_read(PWR_MGMT_1_ADD, &reg_value, 1); //exp 0x01 | 0x02 (if 0x00 - gyro won't work)
    transmit_byte_usart3_debug(reg_value);
    
    spi_read(USER_CTRL_ADD, &reg_value, 1); // exp 0x30
    transmit_byte_usart3_debug(reg_value);
}



void configure_gyro(void){
    uint8_t reg_value = 0; //var to verify written data 

    //change user bank to 2 (new register table)
    spi_write(REG_BANK_SEL_ADD, (0x02 << REG_BANK_SEL_USER_BANK_Pos));
    spi_read(REG_BANK_SEL_ADD, &reg_value, 1); // exp 0x20
    transmit_byte_usart3_debug(reg_value);
    
	/*Gyroscope config*/
	//Select def ODR freq = 1.1 kHz -> GYRO_SMPLRT_DIV = 0 (default)
	//spi_write(GYRO_SMPLRT_DIV_ADD, 0x00);
	
	//Select ODR freq = 75 Hz -> GYRO_SMPLRT_DIV = 14 (0x0E)
    //spi_write(GYRO_SMPLRT_DIV_ADD, 0x0E);

	//Enable gyro DLPF(FCHOICE = 1) + and NBW(noise bandwidth) = 73.3 (3) 
    uint8_t gyro_cfg1 = 0;
    gyro_cfg1 = (1U << GYRO_CONFIG_1_GYRO_FCHOICE_Pos) | (3U << GYRO_CONFIG_1_GYRO_DLPFCFG_Pos);
    //Gyro full scale +/-500 dps
    gyro_cfg1 |= GYRO_FULL_SCALE_500_DPS;
    spi_write(GYRO_CONFIG_1_ADD, gyro_cfg1);
    spi_read(GYRO_CONFIG_1_ADD, &reg_value, 1); // exp 0x1B
    transmit_byte_usart3_debug(reg_value);

    //reset USER BANK reg to default bank (0)
    spi_write(REG_BANK_SEL_ADD, (0x00 << REG_BANK_SEL_USER_BANK_Pos));
    spi_read(REG_BANK_SEL_ADD, &reg_value, 1); // exp 0x00
    transmit_byte_usart3_debug(reg_value);   	
}


void configure_accel(void){
    uint8_t reg_value = 0; //var to verify written data 
     
    //change user bank to 2 (new register table)
    spi_write(REG_BANK_SEL_ADD, (0x02 << REG_BANK_SEL_USER_BANK_Pos));
    spi_read(REG_BANK_SEL_ADD, &reg_value, 1); // exp 0x20
    transmit_byte_usart3_debug(reg_value);
    
	/*Accelerometer config*/
	//Select def ODR freq = 1.125 kHz -> ACCEL_SMPLRT_DIV[0:1] = 0 (default)
    spi_write(ACCEL_SMPLRT_DIV_1_ADD, 0x00);
    spi_write(ACCEL_SMPLRT_DIV_2_ADD, 0x00);
	
	//Enable accel DLPF(FCHOICE = 1) + and NBW(noise bandwidth) = 68.8  (3) 
	uint8_t acc_cfg = 0;
	acc_cfg = (1U << ACCEL_CONFIG_ACCEL_FCHOICE_Pos) | (3U << ACCEL_CONFIG_ACCEL_DLPFCFG_Pos); 
	//Acc full scale +/-2g
	acc_cfg |= ACCEL_FULL_SCALE_2G;
	spi_write(ACCEL_CONFIG_ADD, acc_cfg);
    spi_read(ACCEL_CONFIG_ADD, &reg_value, 1); // exp 0x19
    transmit_byte_usart3_debug(reg_value);
    
    //reset USER BANK reg to default bank (0)
    spi_write(REG_BANK_SEL_ADD, (0x00 << REG_BANK_SEL_USER_BANK_Pos));
    spi_read(REG_BANK_SEL_ADD, &reg_value, 1); // exp 0x00
    transmit_byte_usart3_debug(reg_value);
}




void configure_magnetometer(void){
    uint8_t reg_value = 0; //var to verify written data 
    /*Magnetometer (external sensor) over I2C configuration (ISM20948 is master)*/
    //set user bank 3
    spi_write(REG_BANK_SEL_ADD, (0x03 << REG_BANK_SEL_USER_BANK_Pos));
    spi_read(REG_BANK_SEL_ADD, &reg_value, 1); // exp 0x30
    transmit_byte_usart3_debug(reg_value);
    
    /*I2C Master configuration*/
     //ODR settings: when gyro is active -> external odr is the same 
     
     //i2c master freq: according to recommended in table 23 is 345.60 kHz(46.67%)
    spi_write(I2C_MST_CTRL_ADD, (7U << I2C_MST_CTRL_I2C_MST_CLK_Pos));
    
     /* -------- Magnetometer internal setting configuration ---------------- */
    //set transfer for write +  mag i2c address  
    spi_write(I2C_SLV0_ADDR, 
                            (0 << I2C_SLV0_ADDR_I2C_SLV0_RNW_Pos) |
                            (MAG_I2C_ADD << I2C_SLV0_ADDR_I2C_ID_0_Pos));
    
    //Reset magnitometer (continous mode 4)
    spi_write(I2C_SLV0_REG_ADD, MAG_CNTL3_ADD); //select add of reg to write data to
    spi_write(I2C_SLV0_DO_ADD, (1 << MAG_CNTL3_SRST_Pos)); //reset magnetometer
    //tranfer len = 1 byte + enable i2c write
    spi_write(I2C_SLV0_CTRL_ADD, 
                            (1U << I2C_SLV0_CTRL_I2C_SLV0_LENG_Pos)|
                            (1U << I2C_SLV0_CTRL_I2C_SLV0_EN_Pos));
    delay_ms(1);
    spi_write(I2C_SLV0_CTRL_ADD, (0U << I2C_SLV0_CTRL_I2C_SLV0_EN_Pos));
    
    // Set mag freq to 100 Hz (continous mode 4)
    spi_write(I2C_SLV0_REG_ADD, MAG_CNTL2_ADD);
    spi_write(I2C_SLV0_DO_ADD, (1 << MAG_CNTL2_MODE_3_Pos)); 
    spi_write(I2C_SLV0_CTRL_ADD, 
                            (1U << I2C_SLV0_CTRL_I2C_SLV0_LENG_Pos)|
                            (1U << I2C_SLV0_CTRL_I2C_SLV0_EN_Pos));
    delay_ms(1);
    spi_write(I2C_SLV0_CTRL_ADD, (0U << I2C_SLV0_CTRL_I2C_SLV0_EN_Pos));
    /* --------------------------------------------------------------------- */
     
    //set transfer for read +  mag i2c address  
    spi_write(I2C_SLV0_ADDR, 
                            (1U << I2C_SLV0_ADDR_I2C_SLV0_RNW_Pos) |
                            (MAG_I2C_ADD << I2C_SLV0_ADDR_I2C_ID_0_Pos));
                            
    //set init address of data transfer from ST1 -> HXL_OUT -> ... ST2 (autoinc)
    spi_write(I2C_SLV0_REG_ADD, MAG_ST1_ADD);
    spi_read(I2C_SLV0_REG_ADD, &reg_value, 1);  // exp 0x10 (MAG_ST1_ADD)
    transmit_byte_usart3_debug(reg_value); 
    
    //set length of read data (9 bytes): necessarry to read st2 reg when receiving measurements 
    uint8_t slave0_cntrl_data = (9U << I2C_SLV0_CTRL_I2C_SLV0_LENG_Pos);
    //enable reading for slave0 (magnetometer) -> data are stored in EXT_SENS_DATA_00 -> EXT_SENS_DATA_8
    slave0_cntrl_data |=  (1U << I2C_SLV0_CTRL_I2C_SLV0_EN_Pos);
    spi_write(I2C_SLV0_CTRL_ADD, slave0_cntrl_data);
   
                            
    //reset USER BANK reg to default bank (0)
    spi_write(REG_BANK_SEL_ADD, (0x00 << REG_BANK_SEL_USER_BANK_Pos));
    spi_read(REG_BANK_SEL_ADD, &reg_value, 1); // exp 0x00
    transmit_byte_usart3_debug(reg_value);
}





void TIM1_BRK_TIM9_IRQHandler(void){
    if (TIM9->SR & TIM_SR_UIF){
        TIM9->SR &= ~TIM_SR_UIF; //clear flag!?
        tim9_counter += 1;
        if (cur_spi_state == FREE){
            cur_spi_state = READING;
        }
    }
}

void Timer9_Init(void){
    //Enable tim9(16bit timer) clock from APB2 (108 MHz)
    RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
    
    //Prescaler = 107: 108 Mhz / (107 + 1) = 1 Mhz (1us per tick)
    TIM9->PSC = 107; 
    
    //ARR is TIM9_PERIOD_MS / Tcnt_tick = TIM9_PERIOD_MS * Fcnt_tick
    uint16_t arr_val = TIM9_PERIOD_MS * 1000 - 1;
    //in case of ovf: max period is 10 ms
    if (arr_val > 65535) {
        toggle_led(LED1);
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



void imu_timer_start(void){
    TIM9->CR1 |= TIM_CR1_CEN;
}

void imu_timer_stop(void){
    TIM9->CR1 &= ~TIM_CR1_CEN;
    TIM9->SR &= ~TIM_SR_UIF; //???
    TIM9->CNT = 0;
}






void Imu20948_Init(void){
	/* Configure imu ism20948*/
    powerup_imu();
    configure_gyro();
    configure_accel();
    configure_magnetometer();
    //reset USER BANK reg to default bank (0)
    spi_write(REG_BANK_SEL_ADD, (0x00 << REG_BANK_SEL_USER_BANK_Pos));
    /* INITIALIZE IMU TIMER9*/
    Timer9_Init();
    cur_spi_state = FREE;
}



void get_register_value(uint8_t reg_addr){
    uint8_t imu_resp = 0;
    spi_read(reg_addr, &imu_resp, 1);
    /*Decode 1 BYTE(expected)*/
    whoAmIValue = imu_resp;
}


void swap_buffers(void){
    //swap addresses of write and read imu_scaled_t structures
    imu_scaled_t* temp = write_buf;
    write_buf = read_buf; 
    read_buf = temp;
}


void update_imu_meas(void){
	/*  
    Read ACCEL_XOUT_H_ADD(6)->GYRO_XOUT_H_ADD(6)->TEMP_OUT_H_ADD(2)->EXT_SLV_SENS_DATA_00(9) 
    Decode 23 BYTES data from spi and encode in imu_data struct:
    accel(3 ax)[0:5] + gyro data(3 ax)[6:11] + temp[12,13] + mag_st1[14] + mag_data(3 ax)[15:20] + junk[21] + mag_st2[22] 
    FOR ACCEL and GYRO <MSB first>: GYRO_X_OUT_H -> GYRO_X_OUT_L.
    FOR MAGNET //LITTLE ENDIAN frm AK09916 <LSB first>: H_X_OUT_L -> H_X_OUT_H
    */
    imu_scaled_t* new_meas = write_buf;
    //timestamp
    new_meas->timestamp_ms = TIM9_PERIOD_MS * tim9_counter;
    
    uint8_t imu_resp[23] = {0};
    spi_read(ACCEL_XOUT_H_ADD, imu_resp, 23);
	//Account accel data SENS for chosen FULL SCALE range (+/- 2g)
	new_meas->acc_meas[0] =  (int16_t)((uint16_t)imu_resp[0]	<< 8 | imu_resp[1])	/ 16384.0f; 
	new_meas->acc_meas[1] =  (int16_t)((uint16_t)imu_resp[2]	<< 8 | imu_resp[3])	/ 16384.0f;
	new_meas->acc_meas[2] =  (int16_t)((uint16_t)imu_resp[4]	<< 8 | imu_resp[5])	/ 16384.0f;
	//Account gyro data SENS for chosen FULL SCALE range (+/- 500dps)
	new_meas->gyro_meas[0] = (int16_t)((uint16_t)imu_resp[6]	<< 8 | imu_resp[7])	 / 65.5f;
	new_meas->gyro_meas[1] = (int16_t)((uint16_t)imu_resp[8]	<< 8 | imu_resp[9])	 / 65.5f;
	new_meas->gyro_meas[2] = (int16_t)((uint16_t)imu_resp[10]	<< 8 | imu_resp[11]) / 65.5f;
    
    /*MAGNET decoding*/
    float mag_ovf_resp[3] = {4912.0, 4912.0, 4912.0};
    uint8_t mag_sr1 = imu_resp[14];
    //if (mag_sr1 & (1U << MAG_ST1_DOR_Pos)) toggle_led(LED2);
    uint8_t mag_sr2 = imu_resp[22];
    //mag_responce[21] is dummy byte
    
    //check for magne field overflow (data are incorrect)
    if (mag_sr2 & (1U << MAG_ST2_HOFL_Pos)){
        toggle_led(LED3);
        memcpy(new_meas->mag_meas, mag_ovf_resp, 12);
    }
    else{
        new_meas->mag_meas[0] = (int16_t)((uint16_t)imu_resp[16]  << 8  | imu_resp[15]) * 0.15; 
        new_meas->mag_meas[1] = (int16_t)((uint16_t)imu_resp[18]  << 8  | imu_resp[17]) * 0.15; 
        new_meas->mag_meas[2] = (int16_t)((uint16_t)imu_resp[20]  << 8  | imu_resp[19]) * 0.15; 
    }
    
    swap_buffers(); 
}

//public function to get meas from read only buffer
imu_scaled_t* get_imu_scaled_meas(void){
    return read_buf;
}



void calibrate_gyro(void){
    uint16_t meas_cnt = 0;
    //take mean for each axis
    float meas_sum[3] = {0};
    imu_timer_start();
    while(meas_cnt < GYRO_CALIB_MEAS_NUMBER){
        if (cur_spi_state == READING){
            update_imu_meas(); //blocking!!!
            meas_sum[0] += read_buf->gyro_meas[0];
            meas_sum[1] += read_buf->gyro_meas[1];
            meas_sum[2] += read_buf->gyro_meas[2];
            cur_spi_state = FREE;
            meas_cnt++;
        }
    }
    
    imu_timer_stop();
    gyro_cal_pars.bias[0] = meas_sum[0] / GYRO_CALIB_MEAS_NUMBER;
    gyro_cal_pars.bias[1] = meas_sum[1] / GYRO_CALIB_MEAS_NUMBER;
    gyro_cal_pars.bias[2] = meas_sum[2] / GYRO_CALIB_MEAS_NUMBER;
    
}


imu_scaled_t* get_corrected_imu_meas(imu_scaled_t* meas){
    //GYRO
    meas->gyro_meas[0] -= gyro_cal_pars.bias[0];
    meas->gyro_meas[1] -= gyro_cal_pars.bias[1];
    meas->gyro_meas[2] -= gyro_cal_pars.bias[2];
    
   //ACCEL - no correction yet
    
    //Magnet
    float* mag_meas = meas->mag_meas; //short form
    mag_meas[0]  -= mag_cal_pars.bias[0];
    mag_meas[1]  -= mag_cal_pars.bias[1];
    mag_meas[2]  -= mag_cal_pars.bias[2];
    
    //multiply by the transposed of the matrix
    float mag0  = mag_meas[0]*mag_cal_pars.mtx[0][0]  + mag_meas[1]*mag_cal_pars.mtx[0][1] + mag_meas[2]*mag_cal_pars.mtx[0][2];
    float mag1  = mag_meas[0]*mag_cal_pars.mtx[1][0]  + mag_meas[1]*mag_cal_pars.mtx[1][1] + mag_meas[2]*mag_cal_pars.mtx[1][2];
    float mag2  = mag_meas[0]*mag_cal_pars.mtx[2][0]  + mag_meas[1]*mag_cal_pars.mtx[2][1] + mag_meas[2]*mag_cal_pars.mtx[2][2];
        
    mag_meas[0] = mag0;
    mag_meas[1] = mag1;
    mag_meas[2] = mag2;
    
}



