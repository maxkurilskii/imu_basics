#include "spi_com.h"

volatile spi_state_t cur_spi_state = FREE;
//uint8_t spi_tx_buffer[30] = {0};
//volatile uint8_t spi_rx_buffer[30] = {0};
//uint8_t spi_rx_data_cnt = 0;


void transmit_byte_spi(uint8_t tx_byte){
//	delay_ms(5);
	while (!(SPI1->SR & SPI_SR_TXE));
	*(volatile uint8_t *)&SPI1->DR = tx_byte;
}

void spi_write(uint8_t reg_add, uint8_t data){	
	//Catch slave
	SPI1_CS_LOW;
    for(uint8_t i = 0; i < 3; i++) __NOP(); //~30ns
	transmit_byte_spi(reg_add); 
    while (!(SPI1->SR & SPI_SR_RXNE)); //WAIT for spi transaction to finish!!!
    (void)*(volatile uint8_t *)&SPI1->DR; //read dummy to clear rx buffer

    transmit_byte_spi(data);
    while (!(SPI1->SR & SPI_SR_RXNE)); //WAIT for spi transaction to finish!!!
    (void)*(volatile uint8_t *)&SPI1->DR; //read dummy to clear rx buffer
    while (SPI1->SR & SPI_SR_BSY);  //WAIT for spi to finish all transactions!!
	//Free slave
	SPI1_CS_HIGH;
}


void spi_read(uint8_t reg_add, uint8_t* result_buf, uint8_t byte_quant){
    //Catch slave
	SPI1_CS_LOW;
    for(uint8_t i = 0; i < 3; i++) __NOP(); //~30ns
    
    transmit_byte_spi(reg_add | 0x80);
    // discard dummy saved in SPI1->DR(RX FIFO)
    while (!(SPI1->SR & SPI_SR_RXNE)); //WAIT for spi transaction to finish!!!
    (void)*(volatile uint8_t *)&SPI1->DR; //read dummy
   
    
    for(uint8_t i = 0; i < byte_quant; i++){
        transmit_byte_spi(0x00); //transmit dummy byte
        while (!(SPI1->SR & SPI_SR_RXNE));//WAIT for spi transaction to finish!!!
        *(result_buf+i) = *(volatile uint8_t *)&SPI1->DR; //read
    }

    while (SPI1->SR & SPI_SR_BSY); //WAIT for spi to finish all transactions!!
	//Free slave
	SPI1_CS_HIGH;
}

void SPI1_Init(void){
    /*Enable SPI1 clock  from APB2*/
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    //Set mosi(GPIOA7), miso(GPIOA6), sclk(GPIOA5) pins in alt mode, except nss(chip select) pin
    //Enable GPIOA clock from AHB1
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    //config gpio for spi1 pins
    GPIOA->MODER |= 2U << GPIO_MODER_MODER5_Pos; //set sclk in alt mode (ob10)
    GPIOA->AFR[0] |= 5U << GPIO_AFRL_AFRL5_Pos; //define af5 for gpio5(0b101)
    GPIOA->MODER |= 2U << GPIO_MODER_MODER6_Pos; //set miso in alt mode (ob10)
    GPIOA->AFR[0] |= 5U << GPIO_AFRL_AFRL6_Pos; //define af5 for gpio6(0b101)
    GPIOA->MODER |= 2U << GPIO_MODER_MODER7_Pos; //set mosi in alt mode (ob10)
    GPIOA->AFR[0] |= 5U << GPIO_AFRL_AFRL7_Pos; //define af5 for gpio7(0b101)
    //select GPIOA4 as nss pin, and set in OUTPUT_PUSH_PULL mode
    GPIOA->MODER |= 1U << GPIO_MODER_MODER4_Pos; //set nss in output mode (ob01)
    
    //No start spi com during init
    SPI1_CS_HIGH;
    for(uint8_t i= 0; i < 3; i++) __NOP();
    
    //Software slave mgmnt(ssm = 1, ssi= 1) + master selection  + cpha and cpol are def 
    SPI1->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_MSTR;

    //Clock prescaler: 108 Mhz (APB2) / 32  (0b100) =  3.375 MHz
    SPI1->CR1 |= SPI_CR1_BR_2;
    //Clock prescaler: 108 Mhz (APB2) / 256  (0b111) =  422 kHz
    //SPI1->CR1 |= (7U << SPI_CR1_BR_Pos);
    
    //Data size (DS) = 8 bit (def), thres of SPI_RX_FIFO to 8 bit
    SPI1->CR2 |=  SPI_CR2_FRXTH;
    
    //Enable SPI1
    SPI1->CR1 |= SPI_CR1_SPE;
}
