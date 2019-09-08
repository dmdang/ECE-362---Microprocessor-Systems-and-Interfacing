#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdint.h>
#include <stdio.h>

#define SPI_DELAY 1337

// Extern declarations of function pointers in main.c.
extern void (*cmd)(char b);
extern void (*data)(char b);
extern void (*display1)(const char *);
extern void (*display2)(const char *);

void spi_cmd(char);
void spi_data(char);

//void spi_display1(const char *);
//void spi_display2(const char *);
void dma_display1(const char *);
void circdma_display1(const char *);
void circdma_display2(const char *);

void spi_init_lcd(void);
void dma_spi_init_lcd(void);

void init_tim2(void);
volatile int portApin15Counter = 1;
volatile char buttonPressed = 0;
volatile int buttonPressedConfidencelevel = 0;
volatile int buttonReleasedConfidencelevel = 0;
volatile int confidenceThreshold = 200;

//=========================================================================
// An inline assembly language version of nano_wait.
//=========================================================================
void nano_wait(unsigned int n) {
    asm(    "        mov r0,%0\n"
            "repeat: sub r0,#83\n"
            "        bgt repeat\n" : : "r"(n) : "r0", "cc");
}

//=========================================================================
// Generic subroutine to configure the LCD screen.
//=========================================================================
void generic_lcd_startup(void) {
    nano_wait(100000000); // Give it 100ms to initialize
    cmd(0x38);  // 0011 NF00 N=1, F=0: two lines
    cmd(0x0c);  // 0000 1DCB: display on, no cursor, no blink
    cmd(0x01);  // clear entire display
    nano_wait(6200000); // clear takes 6.2ms to complete
    cmd(0x02);  // put the cursor in the home position
    cmd(0x06);  // 0000 01IS: set display to increment
}

//=========================================================================
// Bitbang versions of cmd(), data(), init_lcd(), display1(), display2().
//=========================================================================
void bitbang_sendbit(int b) {
    const int SCK = 1<<13;
    const int MOSI = 1<<15;
    // We do this slowly to make sure we don't exceed the
    // speed of the device.
    GPIOB->BRR = SCK;
    if (b)
        GPIOB->BSRR = MOSI;
    else
        GPIOB->BRR = MOSI;
    //GPIOB->BSRR = b ? MOSI : (MOSI << 16);
    nano_wait(SPI_DELAY);
    GPIOB->BSRR = SCK;
    nano_wait(SPI_DELAY);
}

//===========================================================================
// Send a byte.
//===========================================================================
void bitbang_sendbyte(int b) {
    int x;
    // Send the eight bits of a byte to the SPI channel.
    // Send the MSB first (big endian bits).
    for(x=8; x>0; x--) {
        bitbang_sendbit(b & 0x80);
        b <<= 1;
    }
}

//===========================================================================
// Send a 10-bit command sequence.
//===========================================================================
void bitbang_cmd(char b) {
    const int NSS = 1<<12;
    GPIOB->BRR = NSS; // NSS low
    nano_wait(SPI_DELAY);
    bitbang_sendbit(0); // RS = 0 for command.
    bitbang_sendbit(0); // R/W = 0 for write.
    bitbang_sendbyte(b);
    nano_wait(SPI_DELAY);
    GPIOB->BSRR = NSS; // set NSS back to high
    nano_wait(SPI_DELAY);
}

//===========================================================================
// Send a 10-bit data sequence.
//===========================================================================
void bitbang_data(char b) {
    const int NSS = 1<<12;
    GPIOB->BRR = NSS; // NSS low
    nano_wait(SPI_DELAY);
    bitbang_sendbit(1); // RS = 1 for data.
    bitbang_sendbit(0); // R/W = 0 for write.
    bitbang_sendbyte(b);
    nano_wait(SPI_DELAY);
    GPIOB->BSRR = NSS; // set NSS back to high
    nano_wait(SPI_DELAY);
}

//===========================================================================
// Configure the GPIO for bitbanging the LCD.
// Invoke the initialization sequence.
//===========================================================================
void bitbang_init_lcd(void) {
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    GPIOB->BSRR = 1<<12; // set NSS high
    GPIOB->BRR = (1<<13) + (1<<15); // set SCK and MOSI low
    // Now, configure pins for output.
    GPIOB->MODER &= ~(3<<(2*12));
    GPIOB->MODER |=  (1<<(2*12));
    GPIOB->MODER &= ~( (3<<(2*13)) | (3<<(2*15)) );
    GPIOB->MODER |=    (1<<(2*13)) | (1<<(2*15));

    generic_lcd_startup();
}

//===========================================================================
// Display a string on line 1.
//===========================================================================
void nondma_display1(const char *s) {
    // put the cursor on the beginning of the first line (offset 0).
    cmd(0x80 + 0);
    int x;
    for(x=0; x<16; x+=1)
        if (s[x])
            data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        data(' ');
}

//===========================================================================
// Display a string on line 2.
//===========================================================================
void nondma_display2(const char *s) {
    // put the cursor on the beginning of the second line (offset 64).
    cmd(0x80 + 64);
    int x;
    for(x=0; x<16; x+=1)
        if (s[x] != '\0')
            data(s[x]);
        else
            break;
    for(   ; x<16; x+=1)
        data(' ');
}
//=========================================================================
// A subroutine to initialize and test the display.
//=========================================================================
void step1(void) {
    // Configure the function pointers
    cmd = bitbang_cmd;
    data = bitbang_data;
    display1 = nondma_display1;
    display2 = nondma_display2;
    // Initialize the display.
    bitbang_init_lcd();
    // Write text.
    const char *msg2 = "                Scrolling text...               ";
    int offset = 0;
    while(1) {
        display1("Hello, World!");
        display2(&msg2[offset]);
        nano_wait(100000000);
        offset += 1;
        if (offset == 32)
            offset = 0;
    }
}

//=========================================================================
//=========================================================================
void step2(void) {
    // Configure the function pointers.
    cmd = spi_cmd;
    data = spi_data;
    display1 = nondma_display1;
    display2 = nondma_display2;
    // Initialize the display.
    spi_init_lcd();
    // Display some interesting things.
    int offset = 20;
    int dir = -1;
    const char *msg1 = "                ===>                ";
    const char *msg2 = "                <===                ";
    while(1) {
        if (dir == -1) {
            display1(&msg1[offset]);
            if (offset == 0)
                dir = 1;
            else
                offset -= 1;
        } else {
            display2(&msg2[offset]);
            if (offset == 20)
                dir = -1;
            else
                offset += 1;
        }
        nano_wait(40000000);
    }
}

//=========================================================================
//=========================================================================
void step3(void) {
    // Configure the function pointers.
    cmd = spi_cmd;
    data = spi_data;
    display1 = dma_display1;
    // Initialize the display using the SPI init function.
    spi_init_lcd();
    // Display something simple.
    while(1) {
        display1("Hello, World!");
        nano_wait(100000000);
    }
}

void step4(void) {
    // Configure the function pointers
    cmd = spi_cmd;
    data = spi_data;
    display1 = circdma_display1;
    display2 = circdma_display2;
    // Initialize the display.
    dma_spi_init_lcd();
    // Write text.
    const char *msg2 = "                Scrolling text...               ";
    int offset = 0;
    while(1) {
        if ((offset / 2) & 1)
            display1("Hello, World!");
        else
            display1("");
        display2(&msg2[offset]);
        nano_wait(100000000);
        offset += 1;
        if (offset == 32)
            offset = 0;
    }
}

static int tenths = 0;
static int seconds = 5;
static int minutes = 0;
static int hours = 0;

void clock(void) {
    //static int tenths = 0;
    //static int seconds = 30;
    //static int minutes = 0;
    //static int hours = 0;
    //tenths += 1;
    tenths -= 1;
 /*
    if (tenths == 10) {
        tenths = 0;
        seconds += 1;
    }
    if (seconds > 59) {
        seconds = 0;
        minutes += 1;
    }
    if (minutes > 59) {
        minutes = 0;
        hours += 1;
    }
    */

    if ((tenths == 0) && (seconds == 0) && (minutes == 0) && (hours == 0)){
        TIM2 -> DIER &= ~(TIM_DIER_UIE);
    }

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

    char line1[20];
    sprintf(line1, "Level %d - Easy", portApin15Counter);
    display1(line1);
    char line2[20];
    sprintf(line2, "%02d:%02d:%02d.%d", hours, minutes, seconds, tenths);
    display2(line2);
}

int flag = 0;

void step6(void) {
    // Configure the function pointers.
    cmd = spi_cmd;
    data = spi_data;
    display1 = circdma_display1;
    display2 = circdma_display2;
    // Initialize the display.
    dma_spi_init_lcd();
    // Initialize timer 2.
    init_tim2();

    // code for button interrupt
    // use cmd to increment number
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA -> MODER &= ~GPIO_MODER_MODER15;
    GPIOA -> PUPDR &= ~GPIO_PUPDR_PUPDR15;

    SYSCFG -> EXTICR[3] = SYSCFG_EXTICR2_EXTI5_PA;
    EXTI -> IMR |= EXTI_IMR_MR15;
    EXTI -> RTSR |= EXTI_RTSR_TR15;

    NVIC_EnableIRQ(EXTI4_15_IRQn);
    NVIC_SetPriority(EXTI4_15_IRQn, 0);

    TIM2 -> DIER &= ~(TIM_DIER_UIE);
    asm("exti_exit:");
    //EXTI -> IMR &= ~EXTI_IMR_MR15;
    //EXTI -> RTSR &= ~EXTI_RTSR_TR15;

    char line1[20];
    sprintf(line1, "Level %d - Easy", portApin15Counter);
    display1(line1);

    micro_wait(1000000);
    char line2a[20];
    sprintf(line2a, "3");
    display2(line2a);
    micro_wait(1000000);
    sprintf(line2a, "2");
    display2(line2a);
    micro_wait(1000000);
    sprintf(line2a, "1");
    display2(line2a);
    micro_wait(1000000);
    sprintf(line2a, "START");
    display2(line2a);
    micro_wait(1000000);

    TIM2 -> DIER |= (TIM_DIER_UIE);
    //EXTI -> IMR |= EXTI_IMR_MR15;
    //EXTI -> RTSR |= EXTI_RTSR_TR15;
    if (flag == 1){
        flag = 0;
        asm("bl exti_return");
    }
    for(;;){
        asm("wfi");
    }
}
/*
volatile int portApin15Counter = 1;
volatile char buttonPressed = 0;
volatile int buttonPressedConfidencelevel = 0;
volatile int buttonReleasedConfidencelevel = 0;
volatile int confidenceThreshold = 200;
 */
void EXTI4_15_IRQHandler(void){
    if (EXTI -> PR & EXTI_PR_PR15){
        if ((GPIOA -> IDR & GPIO_IDR_15) == GPIO_IDR_15){

            micro_wait(50000);
            if ((GPIOA -> IDR & GPIO_IDR_15) != GPIO_IDR_15){

                TIM2 -> DIER |= (TIM_DIER_UIE);

                if (portApin15Counter == 5){
                    portApin15Counter = 5;
                }
                else{
                    portApin15Counter += 1;
                }

                char line1[20];
                sprintf(line1, "Level %d - Easy", portApin15Counter);
                display1(line1);
                flag = 1;
                asm("bl exti_exit");
/*
                char line2a[20];
                sprintf(line2a, "3");
                display2(line2a);
                micro_wait(1000000);
                sprintf(line2a, "2");
                display2(line2a);
                micro_wait(1000000);
                sprintf(line2a, "1");
                display2(line2a);
                micro_wait(1000000);
                sprintf(line2a, "START");
                display2(line2a);
                micro_wait(1000000);
*/
                asm("exti_return:");
                tenths = 0;
                seconds = 5;
                minutes = 0;
                hours = 0;


                EXTI -> PR |= EXTI_PR_PR15;

            }
        }
        else{
            micro_wait(100000);
        }

    }
}
