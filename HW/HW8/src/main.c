//
// All of these exercises should be completed using the Standard Peripheral
// firmware.  Use only direct I/O register manipulation to implement these
// subroutines.  (Do not use Standard Peripheral macros.)  You should also
// assume that the system clock frequency, and the input to all timers,
// is 48 MHz.
//

// This homework will be graded by a test module.  That test module will
// also give you information on what works and what doesn't.  If you connect
// a CFAL1602 OLED LCD to SPI2 channel as you did for Lab 8, it will report
// information when the autotest() subroutine is invoked with an argument of 2.
//

// The end product of this homework will be a DAC output fed by DMA,
// triggered by TIM6, continually outputting a sine wave from a table.
// The frequency of the sine wave will be about 877 Hz.  Timer 3 will
// periodically invoke an interrupt handler that will increase and
// decrease the TIM6 trigger frequency, thereby decreasing and increasing
// the pitch to create a warbling siren-like tone.

// The tone will be accompanied by a blue light that blinks at
// a rate of 4 Hz (.125 seconds on, .125 seconds off).
//

#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include "sinewave.h"

void autotest(int);

// Configuration problem 1: pc8_init()
// Configure PC8 as a general-purpose output.
// Set its output to zero.
// Do not change the pull-up/pull-down resistor
// configuration, speed, type, etc.
// Do not change the configuration of any other pins on port C.
void pc8_init(void) {

    RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;

    // configure pc8 as gp output
    GPIOC -> MODER &= ~(3<<16);
    GPIOC -> MODER |= 1<<16;

    // set output to zero
    GPIOC -> ODR &= ~(1<<8);

}

// Configuration problem 2: TIM2_IRQHandler()
// Write the TIM2 IRQHandler ISR so that it acknowledges (turns off the
// cause of) the update interrupt and toggles the PC8 pin.  i.e. if PA8
// is high, set it low.  If PC8 is low, set it high.
void TIM2_IRQHandler(void) {

    TIM2 -> SR &= ~(1<<0);

    // toggle pc8
    int testbit;
    testbit = ((GPIOC -> ODR) & (1<<8));
    testbit >>= 8;

    if (testbit == 1){

        GPIOC -> ODR &= ~(1<<8);
    }

    else{

        GPIOC -> ODR |= 1<<8;
    }

}

// Configuration problem 3: tim2_init()
// Configure TIM2 to:
// - invoke an update (counter overflow) interrupt every .125 seconds (8 Hz).
// Do not change the configuration of any channel registers.
// Be sure to enable the interrupt in the NVIC.
// Be sure to configure the DIER register.
void tim2_init(void) {

    RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;

    // trigger update every 0.125 sec
    TIM2 -> PSC = 999;
    TIM2 -> ARR = 5999;

    // enable interrupt and configure DIER
    TIM2 -> DIER |= TIM_DIER_UIE;
    NVIC -> ISER[0] = 1 << TIM2_IRQn;

    // start timer
    TIM2 -> CR1 |= TIM_CR1_CEN;

}

// Configuration problem 4: dac_init()
// Configure the DAC channel 1:
// -  so that its output pin is enabled for analog operation.
//    - and the GPIO group it is part of is clocked.
// -  so that the DAC clock is enabled.
// -  to trigger on a Timer 6 TRGO event.
//    (Look up how to do this in the Fam Ref Manual.)
// -  enable the trigger for channel 1.
// -  and enable DMA for channel 1.
// Leave all other options their default.  (e.g. leave the output buffer enabled)
void dac_init(void) {

    // enable output pa4 for analog op
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA -> MODER &= ~(3<<8);
    GPIOA -> MODER |= 3<<8;

    // enable DAC clock
    RCC -> APB1ENR |= RCC_APB1ENR_DACEN;

    // trigger tim6 TRGO event
    DAC -> CR |= DAC_CR_TEN1;
    DAC -> CR &= ~(DAC_CR_TSEL1);

    // enable trigger for channel 1
    DAC -> CR |= DAC_CR_EN1;

    // enable DMA for channel 1
    DAC -> CR |= DAC_CR_DMAEN1;


}

// Configuration problem 5: tim6_init()
// Configure timer 6 so that:
// - The clock to the timer 6 subsystem is enabled.
// - It invokes a TRGO event on every counter update (overflow).
//      (Look at TIM6 CR2 in the Fam. Ref. Manual for TRGO configuration.)
// - The TRGO event should happen at a frequency of 315789 Hz.
//   (or as close to that frequency as you can get).
// - Enable the Auto-Reload Preload.
// - Just for the sake of making this work well,
//   let PSC be a 1 digit decimal number.
// Do not configure any timer 6 channels.
void tim6_init(void) {

    // enable clock
    RCC -> APB1ENR |= RCC_APB1ENR_TIM6EN;

    // invoke TRGO on every counter update
    TIM6 -> CR2 |= TIM_CR2_MMS_1;

    // TRGO event at f = 315789 Hz (3.167 e -6 seconds)
    TIM6 -> PSC = 0;
    TIM6 -> ARR = 151;

    // enable auto-reload preload
    TIM6 -> CR1 |= TIM_CR1_ARPE;

    TIM6 -> CR1 |= TIM_CR1_CEN;

}

// Configuration problem 6: dma3_init()
// Configure DMA channel 3 so that:
// -  The clock to the DMA controller is enabled.
// -  It copies 16-bit elements.
// -  The source is the array sine_table.
// -  The count of elements is the array size.
// -  The destination is the DAC ch1 12-bit right-aligned data holding reg.
// -  Operation is circular (continuous) and increments through memory.
// -  There is no need to invoke an interrupt for half or total completion.
// (At the update rate of Timer 6, this should produce an 877 Hz tone.)
void dma3_init(void) {

    // enable clock to dma controller
    RCC -> AHBENR |= RCC_AHBENR_DMA1EN;

    DMA1_Channel3 -> CCR &= ~(DMA_CCR_EN);

    // copy 16-bit elements
    DMA1_Channel3 -> CCR |= DMA_CCR_MSIZE_0;
    DMA1_Channel3 -> CCR |= DMA_CCR_PSIZE_0;

    // source is array sin_table
    DMA1_Channel3 -> CMAR = (uint32_t) (sine_table);

    // count of elements is array size
    int table_size = sizeof sine_table / sizeof sine_table[7];
    DMA1_Channel3 -> CNDTR = table_size;

    // destination is DAC ch1 12-bit right-aligned data holding reg.
    DMA1_Channel3 -> CPAR = (uint32_t) (&(DAC -> DHR12R1));

    // set direction
    DMA1_Channel3 -> CCR |= DMA_CCR_DIR;

    // circular operation
    DMA1_Channel3 -> CCR |= DMA_CCR_CIRC;

    // increment through memory
    DMA1_Channel3 -> CCR |= DMA_CCR_MINC;

    DMA1_Channel3 -> CCR |= DMA_CCR_EN;

}

// These variables are used below.
int t6_dir = 1;   // Which direction are we changing the offset? (up/down)
int t6_offset = 0;// This is the current period offset.
int t6_max = 100; // This configures the maximum period (minimum frequency).
int t6_min = -40; // This configures the minimum period (maximum frequency).

// Configuration problem 7: TIM3_IRQHandler()
// Set up the Timer 3 ISR to do the following:
// - Acknowledge (clear the cause of) the update interrupt for Timer 3.
// - Add t6_dir to t6_offset.
// - If t6_offset >= t6_max:
//   - Set t6_dir = -1
//   - Set t6_offset = t6_max - 1
// - If t6_offset <= t6_min:
//   - Set t6_dir = 1
//   - Set t6_offset = t6_min + 1
// - Add t6_dir = TIM6->ARR
// (Just to verify, yes, I mean add it to TIM6->ARR.)
// (If you attach a speaker to the DAC output, it will be very annoying.)
void TIM3_IRQHandler(void) {

    TIM3 -> SR &= ~(1<<0);

    t6_offset += t6_dir;

    if (t6_offset >= t6_max){

        t6_dir = -1;
        t6_offset = t6_max - 1;
    }

    if (t6_offset <= t6_min){

        t6_dir = 1;
        t6_offset = t6_min + 1;
    }

    int a;
    a = (TIM6 -> ARR);
    a = a + t6_dir;
    (TIM6 -> ARR) = a;

}

// Configuration problem 8: tim3_init()
// Configure Timer 3:
// - so that the clock is enabled to the TIM3 subsystem.
// - so that TIM3 produces an update interrupts (counter overflows)
//   at a rate of 500 Hz.  (2 ms period)
// - Remember to configure the DIER.
// - Remember to configure the NVIC to allow the interrupt.
// Do not configure any channel registers.
void tim3_init(void) {

    RCC -> APB1ENR |= RCC_APB1ENR_TIM3EN;

    // 2 ms period
    TIM3 -> PSC = 99;
    TIM3 -> ARR = 959;

    // configure DIER and NVIC
    TIM3 -> DIER |= TIM_DIER_UIE;
    NVIC -> ISER[0] = 1 << TIM3_IRQn;


    TIM3 -> CR1 |= TIM_CR1_CEN;

}

// The main subroutine invokes all of the configuration
// subroutines.  The only thing you need to do is turn on
// the autotest() when you're ready to try it.
int main(void)
{
    // Uncomment autotest when you're ready.
    // Use an argument of 0 for basic testing.
    // Use an argument of 1 for fault/timeout handling.
    // Use an argument of 2 for explanations.
    //
    // But it is in your best interest to do this the hard way...
    // Implement the subroutines.  Test them.  Listen to the output on a
    // speaker connected between ground and PA4.  Once you have something
    // working, THEN download autotest.o, and uncomment the call below
    // to find out if you did everything right.
    //
    autotest(2);

    pc8_init();
    tim2_init();
    dac_init();
    tim6_init();
    dma3_init();
    tim3_init();

    for(;;) asm("wfi");
}
