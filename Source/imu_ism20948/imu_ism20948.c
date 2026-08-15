#include "imu_ism20948.h"

imu_data_t imu_meas = {
    .acc_meas = {0},
    .gyro_meas = {0},
    .mag_meas = {0},
    .timestamp = 0
};


uint8_t whoAmIValue = 10;

void TIM1_BRK_TIM9_IRQHandler(void){
    if (TIM9->SR & TIM_SR_UIF){
        TIM9->SR &= ~TIM_SR_UIF; //clear flag!?
        //imu_meas.timestamp = msCounter;
        //read ACCEL_XOUT_H_ADD(6)->GYRO_XOUT_H_ADD(6)->TEMP_OUT_H_ADD(2)->EXT_SLV_SENS_DATA_00(9) 
        //spi_read_async(ACCEL_XOUT_H_ADD, 23);
        //spi_read_async(EXT_SENS_DATA_00_ADD, 9); 
        //spi_read_async(ACCEL_XOUT_H_ADD, 12); 
        spi_read_async(WHO_AM_I, 1);
    }
}



void convert_who_am_i(uint8_t* spi_rx_buf, uint8_t data_len){
    /*Decode 1 BYTE(expected)*/
    whoAmIValue = *spi_rx_buf;
}


void convert_imu_meas(uint8_t* spi_rx_buf, uint8_t data_len){
	/*  
    Decode 20 BYTES data from spi_rx_buf() and encode in imu_data struct:
    accel(3 ax)[0:5] + gyro data(3 ax)[6:11] + temp[12,13] + mag_st1[14] + mag_data(3 ax)[15:20] + junk[21] + mag_st2[22] 
    FOR ACCEL and GYRO <MSB first>: GYRO_X_OUT_H -> GYRO_X_OUT_L.
    FOR MAGNET //LITTLE ENDIAN frm AK09916 <LSB first>: H_X_OUT_L -> H_X_OUT_H
    */
    if (data_len != 23) return;
	//Account accel data SENS for chosen FULL SCALE range (+/- 2g)
	imu_meas.acc_meas[0] =  (int16_t)((uint16_t)*(spi_rx_buf)	<< 8 | *(spi_rx_buf+1))	/ 16384.0f; 
	imu_meas.acc_meas[1] =  (int16_t)((uint16_t)*(spi_rx_buf+2)	<< 8 | *(spi_rx_buf+3))	/ 16384.0f;
	imu_meas.acc_meas[2] =  (int16_t)((uint16_t)*(spi_rx_buf+4)	<< 8 | *(spi_rx_buf+5))	/ 16384.0f;
	//Account gyro data SENS for chosen FULL SCALE range (+/- 500dps)
	imu_meas.gyro_meas[0] = (int16_t)((uint16_t)*(spi_rx_buf+6)	  << 8 | *(spi_rx_buf+7)) 	/ 65.5f;
	imu_meas.gyro_meas[1] = (int16_t)((uint16_t)*(spi_rx_buf+8)	  << 8 | *(spi_rx_buf+9)) 	/ 65.5f;
	imu_meas.gyro_meas[2] = (int16_t)((uint16_t)*(spi_rx_buf+10)  << 8 | *(spi_rx_buf+11)) 	/ 65.5f;
    
    /*MAGNET decoding*/
    float mag_ovf_resp[3] = {4912.0, 4912.0, 4912.0};
    uint8_t mag_sr1 = spi_rx_buf[14];
    if (mag_sr1 & (1U << MAG_ST1_DOR_Pos)) toggle_led(LED2);
    uint8_t mag_sr2 = spi_rx_buf[22];
    //mag_responce[21] is dummy byte
    
    //check for magne field overflow (data are incorrect)
    if (mag_sr2 & (1U << MAG_ST2_HOFL_Pos)){
        toggle_led(LED3);
        memcpy(imu_meas.mag_meas, mag_ovf_resp, 12);
        return;
    }
    imu_meas.mag_meas[0] = (int16_t)((uint16_t)*(spi_rx_buf+16)  << 8  | *(spi_rx_buf+15)) * 0.15; 
    imu_meas.mag_meas[1] = (int16_t)((uint16_t)*(spi_rx_buf+18)  << 8  | *(spi_rx_buf+17)) * 0.15; 
    imu_meas.mag_meas[2] = (int16_t)((uint16_t)*(spi_rx_buf+20)  << 8  | *(spi_rx_buf+19)) * 0.15; 

}


//void convert_accel_gyro_meas(uint8_t* spi_rx_buf, uint8_t data_len){
//	//buffer to read accel(3 ax) + gyro data(3 ax) + mag_data(3 ax)
//	/*  Parse data from spi_rx_buf: accel(3 ax) + gyro data(3 ax)
//    MSB first: GYRO_X_OUT_H -> GYRO_X_OUT_L.
//    */
//    if (data_len != 12) return;
//	//Account accel data SENS for chosen FULL SCALE range (+/- 2g)
//	acc_meas[0] =  (int16_t)((uint16_t)*(spi_rx_buf)	<< 8 	| *(spi_rx_buf+1))	/ 16384.0f; 
//	acc_meas[1] =  (int16_t)((uint16_t)*(spi_rx_buf+2)	<< 8 	| *(spi_rx_buf+3))	/ 16384.0f;
//	acc_meas[2] =  (int16_t)((uint16_t)*(spi_rx_buf+4)	<< 8 	| *(spi_rx_buf+5))	/ 16384.0f;
//	//Account gyro data SENS for chosen FULL SCALE range (+/- 500dps)
//	gyro_meas[0] = (int16_t)((uint16_t)*(spi_rx_buf+6)	<< 8 	| *(spi_rx_buf+7)) 	/ 65.5f;
//	gyro_meas[1] = (int16_t)((uint16_t)*(spi_rx_buf+8)	<< 8 	| *(spi_rx_buf+9)) 	/ 65.5f;
//	gyro_meas[2] = (int16_t)((uint16_t)*(spi_rx_buf+10) << 8 	| *(spi_rx_buf+11)) / 65.5f;
//	
//}



//void convert_magnet_meas(uint8_t* spi_rx_buf, uint8_t data_len){
//    if (data_len != 9) return;
//    float mag_ovf_resp[6] = {4095, 4095, 4095};
//    uint8_t mag_sr1 = spi_rx_buf[0];
//    if (mag_sr1 & (1U << MAG_ST1_DOR_Pos)) toggle_led(LED3);
//    uint8_t mag_sr2 = spi_rx_buf[8];
//    //mag_responce[7] is dummy byte
//    
//    //check for magne field overflow (data are incorrect)
//    if (mag_sr2 & (1U << MAG_ST2_HOFL_Pos)){
//        toggle_led(LED2);
//        memcpy((void*)mag_meas, (void*)mag_ovf_resp, 12);
//        return;
//    }
//    //LITTLE ENDIAN frm AK09916 -> spi_rx_buffer is the same 
//    mag_meas[0] = (int16_t)((uint16_t)*(spi_rx_buf+2)  << 8  | *(spi_rx_buf+1)) * 0.15; 
//    mag_meas[1] = (int16_t)((uint16_t)*(spi_rx_buf+4)  << 8  | *(spi_rx_buf+3)) * 0.15; 
//    mag_meas[2] = (int16_t)((uint16_t)*(spi_rx_buf+6)  << 8  | *(spi_rx_buf+5)) * 0.15; 
//}


void Imu20948_Init(void){
	/* Configure imu ism20948*/
    
    spi_read_async(WHO_AM_I, 1);    // ожидаем 0xEA
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);

    powerup_imu();
    delay_ms(10);
    configure_gyro();
    delay_ms(10);
    configure_accel();
    delay_ms(10);
    configure_magnetometer();
    delay_ms(10);
    
    //reset USER BANK reg to default bank (0)
    //spi_write(REG_BANK_SEL_ADD, (0x00 << REG_BANK_SEL_USER_BANK_Pos));
    
    /* INITIALIZE IMU TIMER9*/
    //Timer9_Init();
    
    //set spi state to FREE (ready to read data)
    cur_spi_state = FREE;
	
}


void powerup_imu(void){
	//reset all registers to def state
    spi_write(PWR_MGMT_1_ADD, PWR_MGMT_1_DEVICE_RESET);
    delay_ms(100);
    
    spi_write(REG_BANK_SEL_ADD, (0x00 << REG_BANK_SEL_USER_BANK_Pos));     
//    spi_read_async(REG_BANK_SEL_ADD, 1);    // ожидаем 0x00
//    while(cur_spi_state != DATA_READY) __NOP();
//    transmit_byte_usart3_debug(spi_byte_read);

    //disable sleep mode + select clock PLL to run gyro in best performance
    spi_write(PWR_MGMT_1_ADD, (PWR_MGMT_1_SLEEP_OFF | PWR_MGMT_1_CLKSEL_PLL));
    delay_ms(1); //NECCESSARY TO WAIT A BIT !!!

    
    //chose SPI mode only (immediately after restart) + enable i2c master
	spi_write(USER_CTRL_ADD, 
                           (1U << USER_CTRL_I2C_IF_DIS_Pos) | 
                           (1U << USER_CTRL_I2C_MST_EN_Pos));
                           
    spi_read_async(PWR_MGMT_1_ADD, 1);    // ожидаем 0x01 | 0x02 | 0x00
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
    
    spi_read_async(USER_CTRL_ADD, 1);    // 0x30 
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
}

void configure_gyro(void){
    //uint8_t reg_value = 0; //var to verify written data 

    //change user bank to 2 (new register table)
    spi_write(REG_BANK_SEL_ADD, (0x02 << REG_BANK_SEL_USER_BANK_Pos));
    spi_read_async(REG_BANK_SEL_ADD, 1);    // ожидаем 0x20
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
    
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
    spi_read_async(GYRO_CONFIG_1_ADD, 1);    // ожидаем 0x1B
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
     

    //reset USER BANK reg to default bank (0)
    spi_write(REG_BANK_SEL_ADD, (0x00 << REG_BANK_SEL_USER_BANK_Pos));
    spi_read_async(REG_BANK_SEL_ADD, 1);    // ожидаем 0x00
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
   
   	
}


void configure_accel(void){
     uint8_t reg_value = 0; //var to verify written data 
     
    //change user bank to 2 (new register table)
    spi_write(REG_BANK_SEL_ADD, (0x02 << REG_BANK_SEL_USER_BANK_Pos));
    spi_read_async(REG_BANK_SEL_ADD, 1);    // ожидаем 0x20
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
    

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
    spi_read_async(ACCEL_CONFIG_ADD, 1);    // ожидаем 0x19
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
    
    //reset USER BANK reg to default bank (0)
    spi_write(REG_BANK_SEL_ADD, (0x00 << REG_BANK_SEL_USER_BANK_Pos));
    spi_read_async(REG_BANK_SEL_ADD, 1);    // ожидаем 0x00
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
}




void configure_magnetometer(void){
    /*Magnetometer (external sensor) over I2C configuration (ISM20948 is master)*/
    //set user bank 3
    spi_write(REG_BANK_SEL_ADD, (0x03 << REG_BANK_SEL_USER_BANK_Pos));
    spi_read_async(REG_BANK_SEL_ADD, 1);    // ожидаем 0x30
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
    
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
    spi_read_async(I2C_SLV0_REG_ADD, 1);    // ожидаем 0x10 (MAG_ST1_ADD)
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
    
    //set length of read data (9 bytes): necessarry to read st2 reg when receiving measurements 
    uint8_t slave0_cntrl_data = (9U << I2C_SLV0_CTRL_I2C_SLV0_LENG_Pos);
    //enable reading for slave0 (magnetometer) -> data are stored in EXT_SENS_DATA_00 -> EXT_SENS_DATA_8
    slave0_cntrl_data |=  (1U << I2C_SLV0_CTRL_I2C_SLV0_EN_Pos);
    spi_write(I2C_SLV0_CTRL_ADD, slave0_cntrl_data);
   
                            
    //reset USER BANK reg to default bank (0)
    spi_write(REG_BANK_SEL_ADD, (0x00 << REG_BANK_SEL_USER_BANK_Pos));
    spi_read_async(REG_BANK_SEL_ADD, 1);    // ожидаем 0x00
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
}

void Timer9_Init(void){
    //Enable tim9(16bit timer) clock from APB2 (108 MHz)
    RCC->APB2ENR |= RCC_APB2ENR_TIM9EN;
    
    //Prescaler = 107: 108 Mhz / (53 + 1) = 1 Mhz (1us per tick)
    TIM9->PSC = 53; 
    
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

uint8_t test_imu_startup(void){    
    /*1 - is error and 0 - imu is ok*/
    spi_write(PWR_MGMT_1_ADD, PWR_MGMT_1_DEVICE_RESET);
    delay_ms(100); 
    
    spi_write(REG_BANK_SEL_ADD, 0x00);      // Bank0
    spi_read_async(WHO_AM_I, 1);    // ожидаем 0xEA
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
    if (spi_byte_read != 0xEA) return 1;
    
    spi_read_async(REG_BANK_SEL_ADD, 1);    // ожидаем 0x00
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
    if (spi_byte_read != 0x00) return 1;
    
    spi_read_async(PWR_MGMT_1_ADD, 1);    // ожидаем 0x41 или 0x01 или 0x02 или 0x00?
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
    if (!(spi_byte_read == 0x41 ||
          spi_byte_read == 0x01 || 
          spi_byte_read == 0x02 ||
          spi_byte_read == 0x00)) return 1;
    
    spi_write(PWR_MGMT_1_ADD, (PWR_MGMT_1_SLEEP_OFF | PWR_MGMT_1_CLKSEL_PLL));
    spi_read_async(PWR_MGMT_1_ADD, 1);    // ожидаем 0x01 или 0x02
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
    if (!(spi_byte_read == 0x01 || spi_byte_read == 0x02)) return 1;
    
    spi_write(USER_CTRL_ADD, 
                           (1U << USER_CTRL_I2C_IF_DIS_Pos) | 
                           (1U << USER_CTRL_I2C_MST_EN_Pos));
    spi_read_async(USER_CTRL_ADD, 1);    // ожидаем 0x30
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
    if (!(spi_byte_read == 0x30)) return 1;
    
    
    spi_write(REG_BANK_SEL_ADD, 0x20);      // Bank2
    spi_read_async(REG_BANK_SEL_ADD, 1);    // ожидаем 0x20
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
    if (!(spi_byte_read == 0x20)) return 1;
    
    spi_write(GYRO_CONFIG_1_ADD, 0x1B);
    spi_read_async(GYRO_CONFIG_1_ADD, 1);   // ожидаем gyro_cfg1 = 0x1B
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
    if (!(spi_byte_read == 0x1B)) return 1;
    
    spi_write(REG_BANK_SEL_ADD, 0x00);      // Bank0
    spi_read_async(REG_BANK_SEL_ADD, 1);    // ожидаем 0x00
    while(cur_spi_state != DATA_READY) __NOP();
    transmit_byte_usart3_debug(spi_byte_read);
    if (!(spi_byte_read == 0x00)) return 1;
    
    return 0;
}
