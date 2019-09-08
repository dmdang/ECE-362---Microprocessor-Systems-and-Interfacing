#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdint.h>
#include <stdio.h>

// These are function pointers.  They can be called like functions
// after you set them to point to other functions.
// e.g.  cmd = bitbang_cmd;
// They will be set by the stepX() subroutines to point to the new
// subroutines you write below.
void (*cmd)(char b) = 0;
void (*data)(char b) = 0;
void (*display1)(const char *) = 0;
void (*display2)(const char *) = 0;

// Prototypes for subroutines in support.c
void generic_lcd_startup(void);
void clock(void);
void step1(void);
void step2(void);
void step3(void);
void step4(void);
void step6(void);

// This array will be used with dma_display1() and dma_display2() to mix
// commands that set the cursor location at zero and 64 with characters.
//
uint16_t dispmem[34] = {
        0x080 + 0,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x080 + 64,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
};

//=========================================================================
// Subroutines for step 2.
//=========================================================================
void spi_cmd(char b) {
    // Your code goes here.
    while ((SPI2 -> SR & SPI_SR_TXE) == 0){

        ;
    }

    SPI2 -> DR = b;

}

void spi_data(char b) {
    // Your code goes here.
    while ((SPI2 -> SR & SPI_SR_TXE) == 0){

        ;
    }

    SPI2 -> DR = 0x200 + b;

}

void spi_init_lcd(void) {
    // Your code goes here.

    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;

    // configure pb12 (SS), pb13 (SCK), pb15 (MOSI) as alt fxns
    // for SPI2

    GPIOB -> MODER &= ~(3<<24);
    GPIOB -> MODER &= ~(3<<26);
    GPIOB -> MODER &= ~(3<<30);

    GPIOB -> MODER |= 2<<24;
    GPIOB -> MODER |= 2<<26;
    GPIOB -> MODER |= 2<<30;

    // use alternate function 0
    GPIOB -> AFR[1] &= ~((0xf)<<16);
    GPIOB -> AFR[1] &= ~((0xf)<<20);
    GPIOB -> AFR[1] &= ~((0xf)<<28);

    // use bidrxnal mode, bidrxnal OE (set master?)
    RCC -> APB1ENR |= RCC_APB1ENR_SPI2EN;
    SPI2 -> CR1 |= SPI_CR1_BIDIMODE;
    SPI2 -> CR1 |= SPI_CR1_BIDIOE;

    SPI2 -> CR1 |= SPI_CR1_MSTR;

    // use slowest baud rate
    SPI2 -> CR1 |= SPI_CR1_BR;

    // BR value at which display still works
    //SPI2 -> CR1 |= (SPI_CR1_BR_1 | SPI_CR1_BR_0);

    // set clock = 0 when idle
    SPI2 -> CR1 &= ~(SPI_CR1_CPOL);

    // use 1st clock trans as capture edge
    SPI2 -> CR1 &= ~(SPI_CR1_CPHA);

    // use 10 bit word size
    SPI2 -> CR2 = (SPI_CR2_DS_0 | SPI_CR2_DS_3);

    // use slave select OE
    SPI2 -> CR2 |= SPI_CR2_SSOE;

    // set auto NSS gen
    SPI2 -> CR2 |= SPI_CR2_NSSP;

    // enable SPI2 with SPE bit
    SPI2 -> CR1 |= SPI_CR1_SPE;

    generic_lcd_startup();
}

//===========================================================================
// Subroutines for step 3.
//===========================================================================

// Display a string on line 1 using DMA.
void dma_display1(const char *s) {
    // Your code goes here.

    cmd(0x80 + 0);
    int x;
    for(x=0; x<16; x+=1)
        if (s[x])
            dispmem[x+1] = s[x] | 0x200;
            //data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        dispmem[x+1] = 0x220;
        //data(' ');

    RCC -> AHBENR |= RCC_AHBENR_DMA1EN;

    DMA1_Channel5 -> CCR &= ~(DMA_CCR_EN);

    // copy from address in CMAR
    DMA1_Channel5 -> CMAR = (uint32_t) (dispmem);

    // copy to address in CPAR
    DMA1_Channel5 -> CPAR = (uint32_t) (&(SPI2 -> DR));

    // set peripheral and mem transf. size to 16 bit
    DMA1_Channel5 -> CCR |= DMA_CCR_PSIZE_0;
    DMA1_Channel5 -> CCR |= DMA_CCR_MSIZE_0;

    // transfer 17 16-bit values
    DMA1_Channel5 -> CNDTR = (sizeof(char)) * 34;

    // set drxn as mem to periph
    DMA1_Channel5 -> CCR |= DMA_CCR_DIR;

    // increment mem addr. after each transfer
    DMA1_Channel5 -> CCR |= DMA_CCR_MINC;

    // set priority to low
    DMA1_Channel5 -> CCR &= ~(DMA_CCR_PL);

    // modify spi ch2 so DMA request made when buffer is empty
    SPI2 -> CR2 |= SPI_CR2_TXDMAEN;

    // enable DMA channel
    DMA1_Channel5 -> CCR |= DMA_CCR_EN;

}

void dma_spi_init_lcd(void) {
    // Your code goes here.

    spi_init_lcd();

    RCC -> AHBENR |= RCC_AHBENR_DMA1EN;

    DMA1_Channel5 -> CCR &= ~(DMA_CCR_EN);

    // copy from address in CMAR
    DMA1_Channel5 -> CMAR = (uint32_t) (dispmem);

    // copy to address in CPAR
    DMA1_Channel5 -> CPAR = (uint32_t) (&(SPI2 -> DR));

    // set peripheral and mem transf. size to 16 bit
    DMA1_Channel5 -> CCR |= DMA_CCR_PSIZE_0;
    DMA1_Channel5 -> CCR |= DMA_CCR_MSIZE_0;

    // transfer 17 16-bit values
    DMA1_Channel5 -> CNDTR = (sizeof(char)) * 34;

    // set drxn as mem to periph
    DMA1_Channel5 -> CCR |= DMA_CCR_DIR;

    // increment mem addr. after each transfer
    DMA1_Channel5 -> CCR |= DMA_CCR_MINC;

    // set priority to low
    DMA1_Channel5 -> CCR &= ~(DMA_CCR_PL);

    // modify spi ch2 so DMA request made when buffer is empty
    SPI2 -> CR2 |= SPI_CR2_TXDMAEN;

    // choose circular mode
    DMA1_Channel5 -> CCR |= DMA_CCR_CIRC;

    // enable DMA channel
    DMA1_Channel5 -> CCR |= DMA_CCR_EN;


}

//===========================================================================
// Subroutines for Step 4.
//===========================================================================

// Display a string on line 1 by copying a string into the
// memory region circularly moved into the display by DMA.
void circdma_display1(const char *s) {
    // Your code goes here.

    //cmd(0x80 + 0);
    int x;
    for(x=0; x<16; x+=1)
        if (s[x])
            dispmem[x+1] = s[x] | 0x200;
            //data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        dispmem[x+1] = 0x220;
        //data(' ');

}

//===========================================================================
// Display a string on line 2 by copying a string into the
// memory region circularly moved into the display by DMA.
void circdma_display2(const char *s) {
    // Your code goes here.

    //cmd(0x80 + 0);
    int x;
    for(x=0; x<16; x+=1)
        if (s[x])
            dispmem[x+18] = s[x] | 0x200;
            //data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        dispmem[x+18] = 0x220;
        //data(' ');
}

//===========================================================================
// Subroutines for Step 6.
//===========================================================================


void init_tim2(void) {
    // Your code goes here.

    // enable clock to timer 2
    RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;

    // trigger interrupt 10x per sec
    TIM2 -> PSC = 999;
    TIM2 -> ARR = 4799;

    TIM2 -> DIER |= TIM_DIER_UIE;
    NVIC -> ISER[0] = 1 << TIM2_IRQn;

    // start timer2
    TIM2 -> CR1 |= TIM_CR1_CEN;
}

void TIM2_IRQHandler(){

    clock();

    TIM2 -> SR &= ~(1<<0);
}

int main(void)
{
    //step1();
    //step2();
    //step3();
    //step4();
    step6();
}
