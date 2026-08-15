#include "spi_nonblocking.h"


volatile spi_state_t cur_spi_state = FREE;
uint8_t spi_tx_buffer[30] = {0};
volatile uint8_t spi_rx_buffer[30] = {0};
uint8_t spi_rx_data_cnt = 0;

//pointer to function type variable
spi_rx_callback_t spi_rx_callback = NULL;


void register_spi_rx_callback(spi_rx_callback_t cb){
    spi_rx_callback = cb;
}

volatile uint8_t spi_byte_read = 0;

uint32_t dma_status = 0, dma_cr = 0;
uint32_t spi_status = 0;

void DMA2_Stream0_IRQHandler(void){
    if (DMA2->LISR & DMA_LISR_TCIF0){
        while (SPI1->SR & SPI_SR_BSY);
        SPI1_CS_HIGH; 
        dma_clear_flags();
        if (cur_spi_state == READING){    
            if (spi_rx_callback != NULL)
                spi_rx_callback(&spi_rx_buffer[1], spi_rx_data_cnt-1); //skip first junk byte
            else spi_byte_read = spi_rx_buffer[1]; // only for one byte read operations
        } 
        cur_spi_state = DATA_READY;
        spi_rx_data_cnt = 0;
    }
}

void spi_write(uint8_t reg_addr, uint8_t tx_byte){
    /*
    Func accepts only ONE byte.
    Uses blocking operation to garantee end of writing to register 
    before next transaction.
    */
    cur_spi_state = WRITING;
   
    //DMA2 Tx Stream3 and RX Stream0 must be disabled during reconfig
    DMA2_Stream3->CR &= ~DMA_SxCR_EN;
    while(DMA2_Stream3->CR & DMA_SxCR_EN);
    DMA2_Stream0->CR &= ~DMA_SxCR_EN;
    while(DMA2_Stream0->CR & DMA_SxCR_EN);
    dma_clear_flags();
    
    /*config num of data that would be send and read*/
    DMA2_Stream3->NDTR = 2; //address byte + data bytes
    DMA2_Stream0->NDTR = 2; //junk bytes
    spi_rx_data_cnt = 2;
    
    spi_tx_buffer[0] = reg_addr;
    spi_tx_buffer[1] = tx_byte;

//    dma_status = DMA2->LISR;
//    spi_status = SPI1->SR;
    
    //SPI start comm sequence (alr should be SPI_EN = 1, SPI_RXDMA=SPI_TXDMA=1)
    SPI1_CS_LOW; //start spi com
    //little delay for slave 
    for(uint8_t i = 0; i < 3;i++) __NOP(); //~30 ns at 108 MHz
    
    DMA2_Stream0->CR |= DMA_SxCR_EN; //dma2 str0 is ready for rx transactions (listens to rx request)
    DMA2_Stream3->CR |= DMA_SxCR_EN; //dma2 str3 is ready for tx transactions (listens to tx request)
    while(cur_spi_state != DATA_READY) __NOP();
}

void spi_read_async(uint8_t reg_addr, uint8_t byte_quant){
//    if (!(cur_spi_state == FREE || cur_spi_state == DATA_READY)) return;
    cur_spi_state = READING;
    
    //DMA2 Tx Stream3 and RX Stream0 must be disabled during reconfig
    DMA2_Stream0->CR &= ~DMA_SxCR_EN;
    while(DMA2_Stream0->CR & DMA_SxCR_EN);
    DMA2_Stream3->CR &= ~DMA_SxCR_EN;
    while(DMA2_Stream3->CR & DMA_SxCR_EN);
    dma_clear_flags();
    
    //config num of data that would be send and read (SPI DR should be read to clear RX_FIFO before next transfer)
    DMA2_Stream3->NDTR = byte_quant + 1; //address byte + data bytes
    DMA2_Stream0->NDTR = byte_quant + 1; //junk bytes
    spi_rx_data_cnt = byte_quant + 1;
    
    spi_tx_buffer[0] = reg_addr | 0x80;
//    dma_status = DMA2->LISR;
//    spi_status = SPI1->SR;
   
   //SPI start comm sequence (alr should be SPI_EN = 1, SPI_RXDMA=SPI_TXDMA=1)
    SPI1_CS_LOW; //start spi com
    
    //little delay for slave 
    for(uint8_t i = 0; i < 3;i++) __NOP(); //~30 ns at 108 MHz

    DMA2_Stream0->CR |= DMA_SxCR_EN; //dma2 str0 is ready for rx transactions (listens to rx request)
    for(uint8_t i = 0; i < 3;i++) __NOP(); //~30 ns at 108 MHz
    DMA2_Stream3->CR |= DMA_SxCR_EN; //dma2 str3 is ready for tx transactions (listens to tx request)
    //  while(cur_spi_state != DATA_READY) __NOP();

}

void SPI1_Init(void){
    //Enable DMA2 streams for SPI1
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
    
    /*DMA2 TX(Stream3) and RX(Stream0) steams initialization*/
    DMA2_Stream0->CR &= ~DMA_SxCR_EN; //disable stream0 during config
    while(DMA2_Stream0->CR & DMA_SxCR_EN);
    DMA2_Stream3->CR &= ~DMA_SxCR_EN; //disable stream3 during config
    while(DMA2_Stream3->CR & DMA_SxCR_EN);
    
    dma_clear_flags();
    
    //Select channel 3 for both streams 
    DMA2_Stream0->CR |= (3U << DMA_SxCR_CHSEL_Pos);
    DMA2_Stream3->CR |= (3U << DMA_SxCR_CHSEL_Pos);
    
    //MSIZE = PSIZE = 8 bit (def 0x00); Priority level(PL) - High (0b10)
    DMA2_Stream0->CR |= DMA_SxCR_PL_1;
    DMA2_Stream3->CR |= DMA_SxCR_PL_0;
    
    //Rx: perif->memory direction (def) + mem incrementation (MINC)
    DMA2_Stream0->CR |= DMA_SxCR_MINC;
    //Tx: memory->perif direction + mem incrementation (MINC)
    DMA2_Stream3->CR |= DMA_SxCR_DIR_0 | DMA_SxCR_MINC;
    
    NVIC_EnableIRQ(DMA2_Stream0_IRQn); //global interrupt enable
    //Enable interrupts only for RX Stream
    DMA2_Stream0->CR |= DMA_SxCR_TCIE;
    
    
    //select mem address for dma TX transfer
    DMA2_Stream3->M0AR = (uint32_t)spi_tx_buffer;
    //SPI DR has to be read after TX transfer to clear RX_FIFO before next transfer
    //select mem address for dma RX transfer 
    DMA2_Stream0->M0AR = (uint32_t)spi_rx_buffer;
    
    //Set peripheral addresses
    DMA2_Stream0->PAR = (uint32_t)(volatile uint8_t*)&SPI1->DR; // take only LSB of spi data reg ADDRESS(!)
    DMA2_Stream3->PAR = (uint32_t)(volatile uint8_t*)&SPI1->DR; // take only LSB of spi data reg ADDRESS(!)
   
    
    //DO NOT ENABLE ANY DMA STREAM IN INITIALIZATION !!!
    
    /*SPI1 initialization*/
    //Enable SPI1 clock from APB2 (108 MHz)
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
    //    SPI1->CR1 |= SPI_CR1_BR_2;
    //Clock prescaler: 108 Mhz (APB2) / 256  (0b111) =  422 kHz
    SPI1->CR1 |= (7U << SPI_CR1_BR_Pos);
    
    //Enable SPI1 only rx requests to DMA + Data size (DS) = 8 bit (default)
    //Select threshold of SPI_RX_FIFO to 8 bit (1/4 of FIFO) -> this sets RXNE flag 
    SPI1->CR2 |= SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN | SPI_CR2_FRXTH;
//    SPI1->CR2 &= ~(SPI_CR2_TXDMAEN | SPI_CR2_RXDMAEN);
//    SPI1->CR2 |=  SPI_CR2_FRXTH;
    
    //Enable SPI1
    SPI1->CR1 |= SPI_CR1_SPE;
   
}

void dma_clear_flags(void){
    DMA2->LIFCR =
          DMA_LIFCR_CFEIF0
        | DMA_LIFCR_CDMEIF0
        | DMA_LIFCR_CTEIF0
        | DMA_LIFCR_CHTIF0
        | DMA_LIFCR_CTCIF0

        | DMA_LIFCR_CFEIF3
        | DMA_LIFCR_CDMEIF3
        | DMA_LIFCR_CTEIF3
        | DMA_LIFCR_CHTIF3
        | DMA_LIFCR_CTCIF3;
}