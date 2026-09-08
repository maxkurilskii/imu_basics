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
extern volatile uint8_t spi_byte_read;

void dma_clear_flags(void);
void SPI1_Init(void);
void DMA2_Stream0_IRQHandler(void);
void spi_write_blocking(uint8_t reg_addr, uint8_t tx_data);
void spi_read_async(uint8_t reg_addr, uint8_t byte_quant);

#endif

