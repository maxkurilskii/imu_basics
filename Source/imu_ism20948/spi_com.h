#ifndef SPI_NONBLOCKING_H
#define SPI_NONBLOCKING_H

#include "common.h"

//pull down ncc/cs (chip select) pin - start of communication
#define SPI1_CS_LOW     GPIOA->BSRR = GPIO_BSRR_BR4
//pull up ncc/cs (chip select) pin - end of communication
#define SPI1_CS_HIGH    GPIOA->BSRR = GPIO_BSRR_BS4

typedef enum{
    FREE,
    WRITING,
    READING,
    DATA_READY
} spi_state_t;

extern volatile spi_state_t cur_spi_state;

void SPI1_Init(void);
void transmit_byte_spi(uint8_t data);
void spi_write(uint8_t reg_add, uint8_t data);
void spi_read(uint8_t reg_add, uint8_t* result_buf, uint8_t byte_quant);

#endif

