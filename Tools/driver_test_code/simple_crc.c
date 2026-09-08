#include <stdio.h>
#include <stdint.h>

#define POLY_CRC8   0x07

void get_bits(uint8_t* data, char* buf, uint8_t quant){
    buf[quant] = '\0'; //add end of str 
    for(uint8_t indx = 0; indx < quant; indx++){
        uint8_t cur_byte = indx / 8;
        uint8_t cur_bit = data[cur_byte] >> indx % 8;
        buf[quant - 1 - indx] = (cur_bit & 0x01) + 48; //fill from back
    }
    // printf("data = %17s", buf);
}


void simple_bit_shifting_crc8(uint8_t* data_byte, uint8_t init_crc_register){
    /* ONE BYTE ALGO*/
    /*"basic" crc8 algo with manual shifting to crc register*/
    uint8_t crc = init_crc_register;
    // uint8_t test_data = 0x0C; 
    char buffer_data[17];  // 16 bits +  '\0' (buffer for printing bits)
    char buffer_crc[9];  // 8 bits +  '\0' (buffer for printing bits)
    // test data should be padded with 8 bits as crc8 is 8th degree polinomial 
    uint16_t padded_data = ((uint16_t)(*data_byte)) << 8;
    //printf("%u\n", padded_data);
    uint8_t step = 0;
    uint8_t crc_msb = 0, data_msb = 0;

    while(step<16){
        data_msb = (padded_data & 0x8000) ? 1 : 0;  
        crc_msb = (crc & 0x80) ? 1 : 0;
        get_bits((uint8_t*)&padded_data, buffer_data, 16);
        printf("Step #%2d: data msb = %d, data = %17s, ", step+1,  data_msb, buffer_data);
        crc = (crc << 1) | data_msb;
        if (crc_msb)
            crc ^= POLY_CRC8;
        get_bits(&crc, buffer_crc, 8);
        printf("crc_msb = %d, crc_register = %9s = 0x%02x\n", crc_msb, buffer_crc, crc);
        padded_data <<= 1;
        step++;

    }
}



void merge_shift_crc8(uint8_t* data_byte, uint8_t init_crc_register){
    uint8_t crc = init_crc_register;
    crc = crc ^ (*data_byte); //MERGING old crc with new data byte
    char buffer_crc[9];  // 8 bits +  '\0' (buffer for printing bits)
    uint8_t step = 0, crc_msb = 0;
    while(step<8){
        crc_msb = (crc & 0x80) ? 1 : 0;
        crc<<=1;
        if (crc_msb)
            crc ^= POLY_CRC8;      
        get_bits(&crc, buffer_crc, 8);
        printf("Step #%2d: crc_msb = %d, crc_register = %9s = 0x%02x\n", step+1, crc_msb, buffer_crc, crc);   
        step++;  
    }
}


void simple_table_crc8(uint8_t* data_byte, uint8_t init_crc_register, uint8_t* table_crc8){
    /* ONE BYTE ALGO*/
    uint8_t crc = init_crc_register;
    char buffer_crc[9];  // 8 bits +  '\0' (buffer for printing bits)
    //algo needs adding 8 zeros to the end of data -> 2 iterations
    uint8_t padded_data[2] = {*data_byte, 0x00}; 
    crc = table_crc8[crc] ^ (crc << 8 | padded_data[0]); //take data
    get_bits(&crc, buffer_crc, 8);
    printf("crc_register = %9s = 0x%02x\n", buffer_crc, crc);   
    crc = table_crc8[crc] ^ (crc << 8 | padded_data[1]); //take last 8 zero bits
    get_bits(&crc, buffer_crc, 8);
    printf("crc_register = %9s = 0x%02x\n", buffer_crc, crc);   
}

void merge_table_crc8(uint8_t* data_byte, uint8_t init_crc_register, uint8_t* table_crc8){
    uint8_t crc = init_crc_register;
    char buffer_crc[9];  // 8 bits +  '\0' (buffer for printing bits)
    //MERGING old crc with new data byte
    uint8_t mrg_byte = crc ^ (*data_byte);  //only for one byte
    crc = table_crc8[mrg_byte]; 
    get_bits(&crc, buffer_crc, 8);
    printf("crc_register = %9s = 0x%02x\n", buffer_crc, crc);   

}

void generate_table_crc8(uint8_t* table_crc8){
    uint8_t crc = 0, msb = 0;
    for(uint16_t i = 0; i <= 255; i++){
        crc = (uint8_t)i;
        for(uint8_t j = 0; j < 8; j++){
            msb = ((crc >> 7) & 0x01);
            crc = (uint8_t)(crc << 1); 
            if (msb) crc ^= POLY_CRC8;
        }
        table_crc8[i] = crc;
    }
}

int main(void){
    uint8_t table_crc8[256] = {0};
    generate_table_crc8(table_crc8);
    uint8_t data = 0x02;
    uint8_t init_crc = 0x01;
    //Adding 8 zero bits to data + 16 shifts crc 
    printf("Approach 1\n"); 
    simple_bit_shifting_crc8(&data, init_crc);
    
    // Combine init_crc and data straight in the begining + 8 shift
    printf("Approach 2\n"); 
    merge_shift_crc8(&data, init_crc);
    
    //Adding 8 zero bits to data, but use table(2 times) instead of 8 shifts  
    printf("Approach 3\n"); // Combine init_crc and data staright in the begining (neglect adding 8 zero bits)
    simple_table_crc8(&data, init_crc, table_crc8);
    
    // Combine init_crc and data straight in the begining + use table (1 time) instead of 8 shifts
    printf("Approach 4\n"); // Combine init_crc and data staright in the begining (neglect adding 8 zero bits)
    merge_table_crc8(&data, init_crc, table_crc8);
    return 0;
}

// текущий crc: 0x05 0x01 0x02 0x03
// забираем 3ий бит crc , то есть 0x05 (0b00000101)
// смотрим по таблице что получится, если применить 8 цилкических сдвигов
// во время этих циклических сдвигов отслеживать станет ли верхний бит = 1,
// если да то делаем XOR с полиномом.
// Например для 0x05 первые 5 шагов попускаеютися, так как там просто нули, но вот
// на 6ом шаге регистр будет выглядеть так:
// 0x101????? (здесь ведь должны быть биты следующего байта сообщения, но таблица из игнорирует)
// и тогда мы делаем XOR:
// 0x101????? << 1 -> 0x01??????? XOR POLY - и эта операция дожна влиять на биты следующего байта.
// Или если ? можно протсо замениь нулями, так как после того 0x05 полностью выйдет, останется:
// только то что было после нескольких операций XOR с POLY:
// 1 - 2й бит изначлаьного регистра равен единице -> 0x01000000 XOR POLY(crc8 = 0x07) = 0x01000111
// 2 - 0ой бит равен единице + то что осталось после первого XOR c POLY: 0x10001110 -> 
// -> сдвиг -> 0x00011100 XOR 0x07 = 0x00011011 - и это то что лежит в таблице по индексу 0x05?
