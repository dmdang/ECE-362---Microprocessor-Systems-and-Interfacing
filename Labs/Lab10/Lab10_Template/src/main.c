#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdarg.h>
#include "fifo.h"

#define UNK -1
#define NON_INTR 0
#define INTR 1

int __io_putchar(int ch);
static int putchar_nonirq(int ch);

void step3(void);
void step4(void);
void step5(void);

void generic_lcd_startup(void);
void spi_cmd(char b);

static struct fifo input_fifo;  // input buffer
static struct fifo output_fifo; // output buffer
int interrupt_mode = UNK;   // which version of putchar/getchar to use.
int echo_mode = 1;          // should we echo input characters?

uint16_t dispmem[34] = {
        0x080 + 0,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x080 + 64,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
        0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
};

//=============================================================================
// This is a version of printf() that will disable interrupts for the
// USART and write characters directly.  It is intended to handle fatal
// exceptional conditions.
// It's also an example of how to create a variadic function.
static void safe_printf(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    char buf[80];
    int len = vsnprintf(buf, sizeof buf, format, ap);
    int saved_txeie = USART1->CR1 & USART_CR1_TXEIE;
    USART1->CR1 &= ~USART_CR1_TXEIE;
    int x;
    for(x=0; x<len; x++) {
        putchar_nonirq(buf[x]);
    }
    USART1->CR1 |= saved_txeie;
    va_end(ap);
}

//=======================================================================
// Simply write a string one char at a time.
//=======================================================================
static void putstr(const char *s) {
    while(*s)
        __io_putchar(*s++);
}

//=======================================================================
// Insert a character and echo it.
// (or, if it's a backspace, remove a char and erase it from the line).
// If echo_mode is turned off, just insert the character and get out.
//=======================================================================
static void insert_echo_char(char ch) {
    if (ch == '\r')
        ch = '\n';
    if (!echo_mode) {
        fifo_insert(&input_fifo, ch);
        return;
    }
    if (ch == '\b' || ch == '\177') {
        if (!fifo_empty(&input_fifo)) {
            char tmp = fifo_uninsert(&input_fifo);
            if (tmp == '\n')
                fifo_insert(&input_fifo, '\n');
            else if (tmp < 32)
                putstr("\b\b  \b\b");
            else
                putstr("\b \b");
        }
        return; // Don't put a backspace into buffer.
    } else if (ch == '\n') {
        __io_putchar('\n');
    } else if (ch == 0){
        putstr("^0");
    } else if (ch == 28) {
        putstr("^\\");
    } else if (ch < 32) {
        __io_putchar('^');
        __io_putchar('A'-1+ch);
    } else {
        __io_putchar(ch);
    }
    fifo_insert(&input_fifo, ch);
}


//-----------------------------------------------------------------------------
// Section 6.2
//-----------------------------------------------------------------------------
// This should should perform the following
// 1) Enable clock to GPIO port A
// 2) Configure PA9 and PA10 to alternate function to use a USART
//    Note: Configure both MODER and AFRL registers
// 3) Enable clock to the USART module, it is up to you to determine
//    which RCC register to use
// 4) Disable the USART module (hint UE bit in CR1)
// 5) Configure USART for 8 bits, 1 stop bit and no parity bit
// 6) Use 16x oversampling
// 7) Configure for 115200 baud rate
// 8) Enable the USART for both transmit and receive
// 9) Enable the USART
// 10) Wait for TEACK and REACK to be set by hardware in the ISR register
// 11) Set the 'interrupt_mode' variable to NON_INTR
void tty_init(void) {
    // Disable buffers for stdio streams.  Otherwise, the first use of
    // each stream will result in a *malloc* of 2K.  Not good.
    setbuf(stdin,0);
    setbuf(stdout,0);
    setbuf(stderr,0);

    // Student code goes here...
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;

    // set moder bits
    GPIOA -> MODER |= (2 << 18);
    GPIOA -> MODER |= (2 << 20);

    // set afr bits
    GPIOA -> AFR[1] |= (1 << 4);
    GPIOA -> AFR[1] |= (1 << 8);

    // enable clock to USART mode
    RCC -> APB2ENR |= RCC_APB2ENR_USART1EN;

    // disable usart module
    USART1 -> CR1 &= ~(USART_CR1_UE);

    // config usart for 8 bits, 1 stop bit, no parity bit
    USART1 -> CR1 &= ~(USART_CR1_M);
    USART1 -> CR2 &= ~(USART_CR2_STOP);
    USART1 -> CR1 &= ~(USART_CR1_PCE);

    // use 16x oversampling
    USART1 -> CR1 &= ~(USART_CR1_OVER8);

    // config for 115200 baud rate
    USART1 -> BRR = 0x1A0;

    // enable usart for transmit and receive
    USART1 -> CR1 |= USART_CR1_TE;
    USART1 -> CR1 |= USART_CR1_RE;

    // enable usart
    USART1 -> CR1 |= USART_CR1_UE;

    // wait for teack and reack to be set in isr
    while((USART1 -> ISR & USART_ISR_TEACK) == 0);
    while((USART1 -> ISR & USART_ISR_REACK) == 0);

    // set interrupt_mode variable to NON_INTR
    interrupt_mode = NON_INTR;
}

//=======================================================================
// Enable the USART RXNE interrupt.
// Remember to enable the right bit in the NVIC registers
//=======================================================================
void enable_tty_irq(void) {
    // Student code goes here...
    USART1 -> CR1 |= USART_CR1_RXNEIE;
    NVIC -> ISER[0] = 1 << USART1_IRQn;
    interrupt_mode = INTR;
}

//-----------------------------------------------------------------------------
// Section 6.3
//-----------------------------------------------------------------------------
//=======================================================================
// This method should perform the following
// Transmit 'ch' using USART1, remember to wait for transmission register to be
// empty. Also this function must check if 'ch' is a new line character, if so
// it must transmit a carriage return before transmitting 'ch' using USART1.
// Think about this, why must we add a carriage return, what happens otherwise?
//=======================================================================
static int putchar_nonirq(int ch) {
    // Student code goes here...
    if (ch == '\n')
    {
        // wait for transmitter to be empty
        while(!(USART1 -> ISR & USART_ISR_TXE));


        // transmit \r
        USART1 -> TDR = '\r';

    }

    // wait for usart transmitter to be empty
    while(!(USART1 -> ISR & USART_ISR_TXE));


    // transmit char in ch variable
    USART1 -> TDR = ch;

    // return ch variable
    return ch;

}

//-----------------------------------------------------------------------------
// Section 6.4
//-----------------------------------------------------------------------------
// See lab document for description
static int getchar_nonirq(void) {
    // Student code goes here...

    if (((USART1 -> ISR) & USART_ISR_ORE) == USART_ISR_ORE){

        // clear overrun flag
        USART1 -> ISR &= ~(USART_ISR_ORE);
    }

    while (!fifo_newline(&input_fifo))
    {
        // wait for rxne flag to be set
        while(!(USART1 -> ISR & USART_ISR_RXNE));

        // read character from usart, put it into input_fifo
        insert_echo_char(USART1 -> RDR);
    }

    // remove character from input_fifo and return it
    return fifo_remove(&input_fifo);
}

//-----------------------------------------------------------------------------
// Section 6.5
//-----------------------------------------------------------------------------
// See lab document for description
//=======================================================================
// IRQ invoked for USART1 activity.
void USART1_IRQHandler(void) {
    // Student code goes here...

    if (((USART1 -> ISR) & USART_ISR_RXNE) == USART_ISR_RXNE){

        // read char from usart and pass it to insert_echo_char()
        insert_echo_char((char) USART1 -> RDR);
    }

    if (((USART1 -> ISR) & USART_ISR_TXE) == USART_ISR_TXE){

        if (fifo_empty(&output_fifo)){

            // turn off txeie interrupt enable
            USART1 -> CR1 &= ~(USART_CR1_TXEIE);
        }

        else{

            USART1 -> TDR = fifo_remove(&output_fifo);

        }
    }

    //-----------------------------------------------------------------
    // Leave this checking code here to make sure nothing bad happens.
    if (USART1->ISR & (USART_ISR_RXNE|
            USART_ISR_ORE|USART_ISR_NE|USART_ISR_FE|USART_ISR_PE)) {
        safe_printf("Problem in USART1_IRQHandler: ISR = 0x%x\n", USART1->ISR);
    }
}

// See lab document for description
static int getchar_irq(void) {
    // Student code goes here...

    while(!fifo_newline(&input_fifo)){
        asm("wfi");
    }

    return fifo_remove(&input_fifo);

}

// See lab document for description
static int putchar_irq(char ch) {
    // Student code goes here...

    while (fifo_full(&output_fifo)){
        asm("wfi");
    }

    if (ch == '\n'){

        fifo_insert(&output_fifo, '\r');

    }

    else{
        fifo_insert(&output_fifo, ch);
    }

    if (((USART1 -> CR1) & USART_CR1_TXEIE) != USART_CR1_TXEIE){

        USART1 -> CR1 |= USART_CR1_TXEIE;
        USART1_IRQHandler();
    }

    if (ch == '\n'){
        while(fifo_full(&output_fifo)){
            asm("wfi");
        }

        fifo_insert(&output_fifo, '\n');
    }

}

//=======================================================================
// Called by the Standard Peripheral library for a write()
int __io_putchar(int ch) {
    if (interrupt_mode == INTR)
        return putchar_irq(ch);
    else
        return putchar_nonirq(ch);
}

//=======================================================================
// Called by the Standard Peripheral library for a read()
int __io_getchar(void) {
    // Choose the right implementation.
    if (interrupt_mode == INTR)
        return getchar_irq();
    else
        return getchar_nonirq();
}

//-----------------------------------------------------------------------------
// Section 6.6
//-----------------------------------------------------------------------------
//===========================================================================
// Act on a command read by testbench().
static void action(char **words) {
    if (words[0] != 0) {
        if (strcasecmp(words[0],"alpha") == 0) {
            // Print the alphabet repeatedly until you press <Enter>.
            char buf[81];
            for(int x=0; x<80; x++)
                buf[x] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[x%26];
            buf[80] = '\0';
            echo_mode = 0;
            for(;;) {
                putstr(buf);
                if (fifo_newline(&input_fifo)) {
                    echo_mode = 1;
                    return;
                }
            }
        }
        if (strcasecmp(words[0],"init") == 0) {
            if (strcasecmp(words[1],"lcd") == 0) {
                //printf("lcd command not implemenented yet\n");

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

                return;
            }
        }
        if (strcasecmp(words[0],"display1") == 0) {

            const char *s;
            s = words[1];

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

            return;
        }
        if (strcasecmp(words[0],"display2") == 0) {

            const char *s;
            s = words[1];

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

            return;
        }

        if (strcasecmp(words[0], "lightsOn") == 0){

            RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;

            GPIOC -> MODER |= (1 << 18);
            GPIOC -> MODER |= (1 << 16);

            GPIOC -> ODR |= (1 << 8);
            GPIOC -> ODR |= (1 << 9);

            return;
        }

        if (strcasecmp(words[0], "lightsOff") == 0){

            GPIOC -> ODR &= ~(1 << 8);
            GPIOC -> ODR &= ~(1 << 9);

            return;
        }

        printf("Unrecognized command: %s\n", words[0]);
    }
}

void generic_lcd_startup(void) {
    micro_wait(100000); // Give it 100ms to initialize
    spi_cmd(0x38);  // 0011 NF00 N=1, F=0: two lines
    spi_cmd(0x0c);  // 0000 1DCB: display on, no cursor, no blink
    spi_cmd(0x01);  // clear entire display
    micro_wait(6200); // clear takes 6.2ms to complete
    spi_cmd(0x02);  // put the cursor in the home position
    spi_cmd(0x06);  // 0000 01IS: set display to increment
}

void spi_cmd(char b) {
    // Your code goes here.
    while ((SPI2 -> SR & SPI_SR_TXE) == 0){

        ;
    }

    SPI2 -> DR = b;

}

//===========================================================================
// Interact with the hardware.
// This subroutine waits for a line of input, breaks it apart into an
// array of words, and passes that array of words to the action()
// subroutine.
// The "display1" and "display2" are special words that tell it to
// keep everything after the first space together into words[1].
//
void testbench(void) {
    printf("STM32 testbench.\n");
    for(;;) {
        char buf[60];
        printf("> ");
        fgets(buf, sizeof buf - 1, stdin);
        int sz = strlen(buf);
        if (sz > 0)
            buf[sz-1] = '\0';
        char *words[7] = { 0,0,0,0,0,0,0 };
        int i;
        char *cp = buf;
        for(i=0; i<6; i++) {
            // strtok tokenizes a string, splitting it up into words that
            // are divided by any characters in the second argument.
            words[i] = strtok(cp," \t");
            // Once strtok() is initialized with the buffer,
            // subsequent calls should be made with NULL.
            cp = 0;
            if (words[i] == 0)
                break;
            if (i==0 && strcasecmp(words[0], "display1") == 0) {
                words[1] = strtok(cp, ""); // words[1] is rest of string
                break;
            }
            if (i==0 && strcasecmp(words[0], "display2") == 0) {
                words[1] = strtok(cp, ""); // words[1] is rest of string
                break;
            }
        }
        action(words);
    }
}

int main(void)
{
    tty_init();
    //step3();
    //step4();
    step5();
    //testbench();

    for(;;)
        asm("wfi");
    return 0;
}
