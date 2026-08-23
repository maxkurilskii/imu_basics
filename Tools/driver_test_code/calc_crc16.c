#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <iso646.h>

#define POLY_CRC16  0x1021 // CRC-16/IBM-3740; CRC-16/AUTOSAR; CRC-16/CCITT-FALSE


uint16_t calculate_crc16(uint8_t* bytearray, uint8_t data_len, uint16_t* table_crc16){
    uint16_t crc = 0xFFFF;
    for(uint8_t i=0;i<data_len;i++){
        uint8_t mrg_byte = (uint8_t)((crc >> 8) ^ bytearray[i]); //index of table
        crc = (crc << 8) ^ table_crc16[mrg_byte]; //new_crc XOR (shifted to right crc)
    }
    return crc;
}

void generate_table_crc16(uint16_t* table_crc16){
    uint16_t crc = 0;
    uint8_t msb = 0;
    for(uint16_t i = 0; i <= 255; i++){
        crc = i << 8; //pad to higher byte
        for(uint8_t j = 0; j < 8; j++){
            msb = (crc & 0x8000) ? 1 : 0;
            crc = (crc << 1); 
            if (msb) crc ^= POLY_CRC16;
        }
        table_crc16[i] = crc;
    }
}

void get_bits(uint8_t* data, char* buf, uint8_t quant){
    buf[quant] = '\0'; //add end of str 
    for(uint8_t indx = 0; indx < quant; indx++){
        uint8_t cur_byte = indx / 8;
        uint8_t cur_bit = data[cur_byte] >> indx % 8;
        buf[quant - 1 - indx] = (cur_bit & 0x01) + 48; //fill from back
    }
}


void print_bits_16(uint16_t* data){
    printf("Value %u = 0x%04x in bits from highest to lowest:\n", *data, *data);
    char var_buf[17] = {0};
    var_buf[16]  = '\0';
    get_bits((uint8_t*)data, var_buf, 16);

    for(uint8_t i = 0; i < 8; i++)
        printf("%c", var_buf[i]);
    printf(" ");
    for(uint8_t i = 8; i < 16; i++)
        printf("%c", var_buf[i]);
    printf("\n");
}


void print_bits_32(uint32_t* data){
    printf("Value %u = 0x%8x in bits from highest to lowest:\n", *data, *data);
    char var_buf[33] = {0};
    var_buf[32]  = '\0';
    get_bits((uint8_t*)data, var_buf, 32);

    for(uint8_t i = 0; i < 8; i++)
        printf("%c", var_buf[i]);
    printf(" ");
    for(uint8_t i = 8; i < 16; i++)
        printf("%c", var_buf[i]);
    printf(" ");
    for(uint8_t i = 16; i < 24; i++)
        printf("%c", var_buf[i]);
    printf(" ");
    for(uint8_t i = 24; i < 32; i++)
        printf("%c", var_buf[i]);
    printf("\n");
}


int main(void){
    // uint32_t my_var = (0x05 << 24) | (0x07 << 16) | (0x09 << 8) | 0x01;
    // print_bits_32(&my_var);

    uint16_t table_crc16[256] = {0};
    generate_table_crc16(table_crc16);
    for(uint16_t i = 0; i < 256; i++)
        printf("0x%04x,", table_crc16[i]);
    // uint8_t msg[2] = {0x01, 0x02};
    // uint8_t msg[9] = {49, 50, 51, 52, 53, 54, 55, 56, 57}; //"123456789" -> crc16 = 0x29B1
    // uint16_t crc16 =  calculate_crc16(msg, 9, table_crc16);
    // print_bits_16(&crc16); 
    return 0;
}