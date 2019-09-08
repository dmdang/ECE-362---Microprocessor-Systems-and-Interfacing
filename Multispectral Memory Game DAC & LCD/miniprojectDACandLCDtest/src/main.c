/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#define COL1 (1 << 0)
#define COL2 (1 << 1)
#define COL3 (1 << 2)
#define COL4 (1 << 3)

#define ROW1 (1 << 4)
#define ROW2 (1 << 5)
#define ROW3 (1 << 6)
#define ROW4 (1 << 7)

#define SAMPLE_TIME_MS 10
#define SAMPLE_COUNT (SAMPLE_TIME_MS)

#define THRESHOLD_TIME_MS 2
#define THRESHOLD (THRESHOLD_TIME_MS)

#define KEY_PRESS_MASK  0b11000111
#define KEY_REL_MASK    0b11100011

int col = 0;
int row = -1;
int red = 0, blue = 0, grn = 0;

char char_array[4][4] = { {'1', '2', '3', 'A'},
                          {'4', '5', '6', 'B'},
                          {'7', '8', '9', 'C'},
                          {'*', '0', '#', 'D'} };


int int_array[4][4] =   { {1,2,3,0xa},
                          {4,5,6,0xb},
                          {7,8,9,0xc},
                          {0xf, 0, 0xe, 0xd} };

uint8_t key_samples[4][4]  = { {0}, {0}, {0}, {0} };
uint8_t key_pressed[4][4]  = { {0}, {0}, {0}, {0} };
uint8_t key_released[4][4]  = { {0}, {0}, {0}, {0} };

void update_key_press() {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if ((key_samples[i][j] & KEY_PRESS_MASK) == 0b00000111) {
                key_pressed[i][j] = 1;
                key_samples[i][j] = 0xFF;
                //DAC -> CR |= DAC_CR_EN1;
                //freq = buttonPress[0];
            }

            if ((key_samples[i][j] & KEY_REL_MASK) == 0b11100000) {
                key_released[i][j] = 1;
                key_samples[i][j] = 0x00;
                //DAC -> CR &= ~DAC_CR_EN1;
            }
        }
    }
}

char get_char_key() {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(key_released[i][j] == 1 && key_pressed[i][j] == 1) {
                key_released[i][j] = 0;
                key_pressed[i][j] = 0;
                return char_array[i][j];
            }
        }
    }

    return '\0';
}

int get_key_pressed() {
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            if(key_released[i][j] == 1 && key_pressed[i][j] == 1) {
                key_released[i][j] = 0;
                key_pressed[i][j] = 0;
                return int_array[i][j];
            }
        }
    }

    return -1;
}

void update_samples(int row) {
    // Update the current column with new values
    for(int i = 0; i < 4; i++) {
        uint8_t curr_history = key_samples[i][col];
        curr_history = curr_history << 1;

        if(row == i)
            curr_history |= 1;

        key_samples[i][col] = curr_history;
    }
}

// These are function pointers.  They can be called like functions
// after you set them to point to other functions.
// e.g.  cmd = bitbang_cmd;
// They will be set by the stepX() subroutines to point to the new
// subroutines you write below.
void (*cmd)(char b) = 0;
void (*data)(char b) = 0;
void (*display1)(const char *) = 0;
void (*display2)(const char *) = 0;
void circdma_display1(const char *);
void circdma_display2(const char *);

// Prototypes for subroutines in support.c
void generic_lcd_startup(void);
void clock(void);
void step6(void);
volatile int portApin15Counter = 1;
volatile int portApin15Counter2 = 1;
void spi_cmd(char);
void spi_data(char);
void init_tim2(void);
void dma_spi_init_lcd(void);
void spi_init_lcd(void);
volatile uint32_t frequency;
volatile uint32_t angle;
void setup_dac(void);
void setup_timer3(void);
void setup_tim6(void);
void make_wavetable(void);
void init_keypad(void);
int16_t wavetable[256];
int flag = 0;
int modeNum = 0; // easy
char mode[] = "Easy";
static int tenths = 0;
static int seconds = 5;
static int minutes = 0;
static int hours = 0;
int lvl1 = 1;
int offset1 = 0;
int step1[] = {3.5 * (1<<16), 3.5 * (1<<16), 3.5 * (1<<16), 5.207 * (1<<16)};
int bpm[] = {1, 1, 1, 1};
int lose[] = {2.207 * (1<<16), 1.86 * (1<<16)};
int losebpm[] = {1, 1};
int buttonPress[] = {0.77 * (1<<16), 1.03 * (1<<16), 1.29 * (1<<16), 1.55 * (1<<16)};
int pressbpm[] = {1, 1, 1, 1};
volatile int freq;
int countdownFlag = 0;
int gameoverFlag = 0;


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


int main(void)
{
    //step1();
    //step2();
    //step3();
    //step4();
    step6();
}

void init_tim2(void) {
    // Your code goes here.

    // enable clock to timer 2
    RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;

    // trigger interrupt 10x per sec
    TIM2 -> PSC = 999;
    TIM2 -> ARR = 4799;

    TIM2 -> DIER |= TIM_DIER_UIE;
    NVIC -> ISER[0] = 1 << TIM2_IRQn;
    //NVIC_SetPriority(TIM2_IRQn, 1);

    // start timer2
    TIM2 -> CR1 |= TIM_CR1_CEN;
}

int flagTIM2 = 0;

void TIM2_IRQHandler(){

    clock();

    TIM2 -> SR &= ~(1<<0);
}

void countdown(void){
    char line2a[20];
    int num = 3;
    int i, duration;

    for (i = 0; i < 4; i++){

        if (num == 0){
            sprintf(line2a, "START");
        }
        else{
            sprintf(line2a, "%d", num);
        }

         DAC -> CR |= DAC_CR_EN1;
         freq = step1[i];
         duration = bpm[i];
         display2(line2a);
         micro_wait(duration * 500000);
         DAC -> CR &= ~DAC_CR_EN1;
         display2(" ");
         micro_wait(duration * 500000);
        num--;
    }
}

void loser(void){

    int i, duration;
    char line1[20];
    char line2[20];

    for (i = 0; i < 2; i++){
        DAC -> CR |= DAC_CR_EN1;
        freq = lose[i];
        duration = losebpm[i];
        micro_wait(duration * 1000000);
        DAC -> CR &= ~DAC_CR_EN1;
    }

    micro_wait(2000000);

    portApin15Counter = 1;
    strcpy(mode, "Easy");
    sprintf(line1, "Level %d - %s", portApin15Counter, mode);
    display1(line1);
    sprintf(line2, " ");
    display2(line2);

}

void step6(void) {
    // Configure the function pointers.
    char line1[20];
    char line2[20];
    cmd = spi_cmd;
    data = spi_data;
    display1 = circdma_display1;
    display2 = circdma_display2;
    // Initialize the display.
    dma_spi_init_lcd();
    // Initialize timer 2.
    init_tim2();
    setup_tim6();
    setup_timer3();
    make_wavetable();
    setup_dac();
    DAC -> CR &= ~DAC_CR_EN1;
    init_keypad();

    // code for button interrupt
    // use cmd to increment number
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA -> MODER &= ~GPIO_MODER_MODER15;
    GPIOA -> PUPDR &= ~GPIO_PUPDR_PUPDR15;
    GPIOA -> MODER &= ~GPIO_MODER_MODER11;
    GPIOA -> PUPDR &= ~GPIO_PUPDR_PUPDR11;

    SYSCFG -> EXTICR[3] = SYSCFG_EXTICR2_EXTI5_PA;

    EXTI -> IMR |= EXTI_IMR_MR15;
    EXTI -> RTSR |= EXTI_RTSR_TR15;
    EXTI -> IMR |= EXTI_IMR_MR11;
    EXTI -> RTSR |= EXTI_RTSR_TR11;

    NVIC_EnableIRQ(EXTI4_15_IRQn);
    NVIC_SetPriority(EXTI4_15_IRQn, 0);

    TIM2 -> DIER &= ~(TIM_DIER_UIE);

    if ((RCC -> CSR & RCC_CSR_PINRSTF) == RCC_CSR_PINRSTF){

        sprintf(line1, "Level %d - %s", portApin15Counter, mode);
        display1(line1);
        sprintf(line2, " ");
        display2(line2);
    }

    EXTI -> IMR &= ~EXTI_IMR_MR15;
    EXTI -> RTSR &= ~EXTI_RTSR_TR15;
    EXTI -> IMR &= ~EXTI_IMR_MR11;
    EXTI -> RTSR &= ~EXTI_RTSR_TR11;

    EXTI -> IMR |= EXTI_IMR_MR15;
    EXTI -> RTSR |= EXTI_RTSR_TR15;
    EXTI -> IMR |= EXTI_IMR_MR11;
    EXTI -> RTSR |= EXTI_RTSR_TR11;


    int i, duration;
    int current_row = -1;
    int current_col = -1;
    int testRow;
    int testCol;
    int z = 0; // int i
    int a;
    int pin1;
    int pin2;
    int y;

    for(;;){
        if ((GPIOA -> IDR & GPIO_IDR_15) == GPIO_IDR_15){

            micro_wait(10000);
            if ((GPIOA -> IDR & GPIO_IDR_15) != GPIO_IDR_15){

                    if (portApin15Counter == 6){
                        portApin15Counter = 5;
                    }

                    char line1[20];
                    sprintf(line1, "Level %d - %s", portApin15Counter, mode);
                    display1(line1);

                    if (portApin15Counter > 5){
                        portApin15Counter = 5;
                    }
                    else{
                        portApin15Counter += 1;
                    }

                    TIM2 -> DIER &= ~(TIM_DIER_UIE);
                    countdownFlag = 1;
                    countdown();
                    TIM2 -> DIER |= (TIM_DIER_UIE);

                    tenths = 0;
                    seconds = 5;
                    minutes = 0;
                    hours = 0;

                DAC -> CR &= ~DAC_CR_EN1;
            }
    }

    if((GPIOA -> IDR & GPIO_IDR_11) == GPIO_IDR_11){

        micro_wait(10000);
       if ((GPIOA -> IDR & GPIO_IDR_11) != GPIO_IDR_11){

            portApin15Counter -= 1;

            if (portApin15Counter == 0){
                NVIC_SystemReset();
            }

            if (modeNum == 0){
                modeNum = 1;
                strcpy(mode, "Hard");
                char line1[20];
                sprintf(line1, "Level %d - %s", portApin15Counter, mode);
                display1(line1);
            }
            else if (modeNum == 1){
                modeNum = 0;
                strcpy(mode, "Easy");
                char line1[20];
                sprintf(line1, "Level %d - %s", portApin15Counter, mode);
                display1(line1);
            }

            portApin15Counter += 1;

            TIM2 -> DIER &= ~(TIM_DIER_UIE);
            countdownFlag = 1;
            countdown();
            TIM2 -> DIER |= (TIM_DIER_UIE);

            tenths = 0;
            seconds = 5;
            minutes = 0;
            hours = 0;

            DAC -> CR &= ~DAC_CR_EN1;

        }

    }

    if ((GPIOF -> MODER & (1 << 14)) == (1 << 14)){

        gameoverFlag = 0;
        GPIOF -> MODER &= ~(3 << 14);
        loser();
    }

    testRow = GPIOA -> IDR & (0xfff);
    testRow >>= 5;

    testCol = GPIOA -> IDR & (0xf);

    for (z = 0; z < 4; z++)
    {

        pin1 = testRow & 1;

        if (pin1 == 1)
        {
            current_row = z;

            DAC -> CR |= DAC_CR_EN1;
            freq = buttonPress[z];
            duration = pressbpm[z];
            micro_wait(duration * 50000);
            DAC -> CR &= ~DAC_CR_EN1;
        }

        testRow >>= 1;

    }

    update_samples(current_row);

    update_key_press();

    col += 1;

    if (col > 3)
    {
        col = 0;
    }

    GPIOA -> ODR = (1 << col);

    }
}

void EXTI4_15_IRQHandler(void){
    if (EXTI -> PR & EXTI_PR_PR15){
        if ((GPIOA -> IDR & GPIO_IDR_15) == GPIO_IDR_15){

                EXTI -> PR |= EXTI_PR_PR15;

        }
        else{
            micro_wait(100000);
        }

    }
    if (EXTI -> PR & EXTI_PR_PR11){
        if ((GPIOA -> IDR & GPIO_IDR_11) == GPIO_IDR_11){


                EXTI -> PR |= EXTI_PR_PR11;
          }

        }
        else{
            micro_wait(100000);
        }
    }

void clock(void) {

    char line1[20];
    char line2[20];
    tenths -= 1;

    if (tenths < 0){
        tenths = 9;
        seconds -= 1;
    }
    if (seconds < 0){
        seconds = 59;
        minutes -= 1;
    }
    if (minutes < 0){
        minutes = 59;
        hours -= 1;
    }

    sprintf(line2, "%02d:%02d:%02d.%d", hours, minutes, seconds, tenths);
    display2(line2);

    if ((tenths == 0) && (seconds == 0) && (minutes == 0) && (hours == 0)){

        RCC -> AHBENR |= RCC_AHBENR_GPIOFEN;
        GPIOF -> MODER &= ~(3 << 14);
        GPIOF -> MODER |= (1 << 14);

        TIM2 -> DIER &= ~(TIM_DIER_UIE);

        sprintf(line1, "GAME OVER");
        display1(line1);

        portApin15Counter -= 1;

        sprintf(line2, "Level %d - %s", portApin15Counter, mode);
        display2(line2);

        portApin15Counter += 1;
    }

}


void TIM3_IRQHandler()
{

    TIM3 -> SR &= ~(1<<0);

}

void TIM6_DAC_IRQHandler(void){

    TIM6 -> SR &= ~(1 << 0);

    int sample = 0;
    offset1 += freq;

    DAC -> SWTRIGR |= DAC_SWTRIGR_SWTRIG1;

    if ((offset1>>16) >= (sizeof wavetable / sizeof wavetable[0])){

        offset1 -= ((sizeof wavetable / sizeof wavetable[0])<<16);
    }

    sample = (wavetable[offset1>>16]);
    sample = sample / 16 + 2048;

    DAC -> DHR12R1 = sample;
}

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

void generic_lcd_startup(void) {
    nano_wait(100000000); // Give it 100ms to initialize
    cmd(0x38);  // 0011 NF00 N=1, F=0: two lines
    cmd(0x0c);  // 0000 1DCB: display on, no cursor, no blink
    cmd(0x01);  // clear entire display
    nano_wait(6200000); // clear takes 6.2ms to complete
    cmd(0x02);  // put the cursor in the home position
    cmd(0x06);  // 0000 01IS: set display to increment
}

void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

void setup_dac(void){
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC -> APB1ENR |= RCC_APB1ENR_DACEN;

    GPIOA -> MODER &= ~(3 << 8);
    GPIOA -> MODER |= (3 << 8);

    DAC -> CR |= (DAC_CR_TSEL1_2 | DAC_CR_TSEL1_1 | DAC_CR_TSEL1_0);
    DAC -> CR |= DAC_CR_TEN1;

    DAC -> CR |= DAC_CR_EN1;
}

void make_wavetable(void){
    int x;
    for (x = 0; x < sizeof wavetable / sizeof wavetable[0]; x += 1){

        wavetable[x] = 32767 * sin(x * 2 * M_PI / 256);

    }
}

void init_keypad() {

    // enable clock to port A
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;

    // configure pins 0,1,2,3 as output
    GPIOA -> MODER &= ~(3);
    GPIOA -> MODER &= ~(3<<2);
    GPIOA -> MODER &= ~(3<<4);
    GPIOA -> MODER &= ~(3<<6);

    GPIOA -> MODER |= 1<<0;
    GPIOA -> MODER |= 1<<2;
    GPIOA -> MODER |= 1<<4;
    GPIOA -> MODER |= 1<<6;

    // configure pins 5,6,7,8 to use pull-down resistor
    GPIOA -> PUPDR &= ~(3<<10);
    GPIOA -> PUPDR &= ~(3<<12);
    GPIOA -> PUPDR &= ~(3<<14);
    GPIOA -> PUPDR &= ~(3<<16);

    GPIOA -> PUPDR |= 2<<10;
    GPIOA -> PUPDR |= 2<<12;
    GPIOA -> PUPDR |= 2<<14;
    GPIOA -> PUPDR |= 2<<16;

}

void setup_timer3() {

    // enable clock to timer 3
    RCC -> APB1ENR |= RCC_APB1ENR_TIM3EN;

    // trigger interrupt every 1ms
    TIM3 -> PSC = 9;
    TIM3 -> ARR = 4799;

    TIM3 -> DIER |= TIM_DIER_UIE;
    NVIC -> ISER[0] = 1 << TIM3_IRQn;

    // start timer3
    TIM3 -> CR1 |= TIM_CR1_CEN;

}

void setup_tim6(void){

    RCC -> APB1ENR |= RCC_APB1ENR_TIM6EN;

    TIM6 -> ARR = 1;
    TIM6 -> PSC = 374;

    TIM6 -> DIER |= TIM_DIER_UIE;
    NVIC -> ISER[0] |= 1 << TIM6_DAC_IRQn;

    TIM6 -> CR1 |= TIM_CR1_CEN;
}
