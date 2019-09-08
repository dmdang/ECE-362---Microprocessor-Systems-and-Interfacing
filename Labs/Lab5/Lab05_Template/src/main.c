#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

void micro_wait(int);

void prob1(void) {

     // enable port c
     RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;

     // set mode for pc9 as output mode
     GPIOC -> MODER &= ~(3<<18);
     GPIOC -> MODER |= 1<<18;

     while(1){

     GPIOC -> ODR |= (1<<9);
     micro_wait(1000000);
     GPIOC -> ODR &= ~(1<<9);
     micro_wait(1000000);

     }

     return;

}

void prob2(void) {

    // enable port c
    RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;

    // set mode for pc9 as "alternate fxn"
    GPIOC -> MODER &= ~(3<<18);
    GPIOC -> MODER |= 2<<18;

    // enable system clock for timer 3
    RCC -> APB1ENR |= RCC_APB1ENR_TIM3EN;

    // counting direction: 0 = up, 1 = down
    TIM3 -> CR1 &= ~TIM_CR1_DIR; // clear it to count up

    // set prescaler output to 4khz (48MHz / 12000)
    TIM3 -> PSC = 12000 - 1;

    // auto-reload 4000
    TIM3 -> ARR = 4000 - 1;

    // any value between 0 and 4000
    TIM3 -> CCR3 = 3456;

    // channel 4 of the timer is configured in ccmr2
    // set the bits to select toggle mode (011)
    TIM3 -> CCMR2 &= ~TIM_CCMR2_OC4M_2; // turn off bit 2
    TIM3 -> CCMR2 |= TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_0;

    //enable output for channel 4 active-high output
    TIM3 -> CCER |= TIM_CCER_CC4E;

    // enable timer 3
    TIM3 -> CR1 |= TIM_CR1_CEN;

    while(1)
        asm ("wfi");

    return;
}

void prob3(void) {

        // enable port c
        RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;

        // set mode for pc9 as "alternate fxn"
        GPIOC -> MODER &= ~(3<<18);
        GPIOC -> MODER |= 2<<18;

        // set mode for pc8 as "alternative fxn"
        GPIOC -> MODER &= ~(3<<16);
        GPIOC -> MODER |= 2<<16;

        // enable system clock for timer 3
        RCC -> APB1ENR |= RCC_APB1ENR_TIM3EN;

        // counting direction: 0 = up, 1 = down
        TIM3 -> CR1 &= ~TIM_CR1_DIR; // clear it to count up

        // set prescaler output to 4khz (48MHz / 12000)
        TIM3 -> PSC = 12000 - 1;

        // auto-reload 4000
        TIM3 -> ARR = 4000 - 1;

        // any value between 0 and 4000
        TIM3 -> CCR3 = 3456;
        TIM3 -> CCR4 = 3456;

        // channel 4 of the timer is configured in ccmr2
        // set the bits to select toggle mode (011)
        TIM3 -> CCMR2 &= ~TIM_CCMR2_OC4M_2; // turn off bit 2
        TIM3 -> CCMR2 |= TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_0;

        // do same for blue led
        TIM3 -> CCMR2 &= ~TIM_CCMR2_OC3M_2; // turn off bit 2
        TIM3 -> CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_0;

        //enable output for channel 4 active-high output
        TIM3 -> CCER |= TIM_CCER_CC4E;

        // enable output for channel 3 active-high output
        TIM3 -> CCER |= TIM_CCER_CC3E;

        // enable timer 3
        TIM3 -> CR1 |= TIM_CR1_CEN;

        while(1)
            asm ("wfi");

        return;
}

void prob4(void) {

    // enable port c
            RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;

            // set mode for pc9 as "alternate fxn"
            GPIOC -> MODER &= ~(3<<18);
            GPIOC -> MODER |= 2<<18;

            // set mode for pc8 as "alternative fxn"
            GPIOC -> MODER &= ~(3<<16);
            GPIOC -> MODER |= 2<<16;

            // enable system clock for timer 3
            RCC -> APB1ENR |= RCC_APB1ENR_TIM3EN;

            // counting direction: 0 = up, 1 = down
            TIM3 -> CR1 &= ~TIM_CR1_DIR; // clear it to count up

            // set prescaler output to 4khz (48MHz / 12000)
            TIM3 -> PSC = 12000 - 1;

            // auto-reload 4000
            TIM3 -> ARR = 4000 - 1;

            // any value between 0 and 4000
            TIM3 -> CCR3 = 1728;
            TIM3 -> CCR4 = 3456;

            // channel 4 of the timer is configured in ccmr2
            // set the bits to select toggle mode (011)
            TIM3 -> CCMR2 &= ~TIM_CCMR2_OC4M_2; // turn off bit 2
            TIM3 -> CCMR2 |= TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_0;

            // do same for blue led
            TIM3 -> CCMR2 &= ~TIM_CCMR2_OC3M_2; // turn off bit 2
            TIM3 -> CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_0;

            //enable output for channel 4 active-high output
            TIM3 -> CCER |= TIM_CCER_CC4E;

            // enable output for channel 3 active-high output
            TIM3 -> CCER |= TIM_CCER_CC3E;

            // enable timer 3
            TIM3 -> CR1 |= TIM_CR1_CEN;

            while(1)
                asm ("wfi");

            return;
}

void display(int x) {
    x &= 0xf; // Only look at the low 4 bits.

    //    _____
    //   |  A  |
    //  F|     |B
    //   |_____|
    //   |  G  |
    //  E|     |C
    //   |_____|
    //      D
    static const char output[16] = {
            0x40, // 0: _FEDCBA 1000000
            0x79, // 1: ____CB_ 1111001
            // complete the rest of the table...
            0x24, // 2: G_ED_BA
            0x30, // 3: G__DCBA
            0x19, // 4: GF__CB_
            0x12, // 5: GF_DC_A
             0x2, // 6: GFEDC_A
            0x78, // 7: ____CBA
             0x0, // 8: GFEDCBA
            0x18, // 9: GF__CBA
             0x8, // a: GFE_CBA
             0x3, // b: GFEDC__
            0x46, // c: _FED__A
            0x21, // d: G_EDCB_
             0x6, // e: GFED__A
             0xe, // f: GFE___A
    };

    // You need to look up the value of output[x] here
    // and then somehow assign it to the ODR.

    GPIOC -> ODR |= 0xff;
    GPIOC -> ODR &= output[x];
}

void init_display(void) {

    RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;

    GPIOC -> MODER &= ~(3<<18);
    GPIOC -> MODER |= 0x5555;

    GPIOC -> ODR |= 0xff;

}

void test_display(void) {
    init_display();
    // Enable Port C

    int x;
    for(;;)
        for(x=0; x<16; x++) {
            display(x);
            micro_wait(500000);
        }
}

int count = 0;
void increment(void) {
    count += 1;
    display(count);
}

void TIM2_IRQHandler(void){

    increment();
    int i;
    i = TIM2 -> CCR1;
    i = TIM2 -> CCR2;

}

void prob6(void) {

    // configure pa0 to be alt fxn as input to timer2
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    GPIOA -> MODER &= ~(3);
    GPIOA -> MODER |= 2;

    GPIOA -> AFR[0] &= ~0xf;
    GPIOA -> AFR[0] |= 0x2;

    // call init_display and display
    init_display();
    display(0);

    // enable timer2 clock in APB1ENR
    RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;

    // set timer2 prescaler to 1
    TIM2 -> PSC = 0;

    // set timer2 auto-reload reg to 0xffffffff
    TIM2 -> ARR = 0xffffffff;

    // set drxn of timer 2 channel 1
    TIM2 -> CCMR1 &= ~TIM_CCMR1_CC1S;
    TIM2 -> CCMR1 |= TIM_CCMR1_CC1S_0;

    // enable timer 2 channel 1
    TIM2 -> CCER |= TIM_CCER_CC1E;

    // enable interrupt generation for timer 2 channel 1
    TIM2 -> DIER |= TIM_DIER_CC1IE;

    // enable timer 2 by using CEN
    TIM2 -> CR1 |= TIM_CR1_CEN;

    // enable interrupts
    NVIC -> ISER[0] = 1 << TIM2_IRQn;

    for(;;)
        asm("wfi");

    return;
}

void prob7(void) {

        // configure pa1 to be alt fxn as input to timer2
        RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
        GPIOA -> MODER &= ~(3<<2);
        GPIOA -> MODER |= (2<<2);

        GPIOA -> AFR[0] &= ((~0xf)<<4);
        GPIOA -> AFR[0] |= ((0x2)<<4);

        // call init_display and display
        init_display();
        display(0);

        // enable timer2 clock in APB1ENR
        RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;

        // use pull-down resistor
        GPIOA -> PUPDR |= (2<<2);

        // set timer2 prescaler to 1
        TIM2 -> PSC = 0;

        // set timer2 auto-reload reg to 0xffffffff
        TIM2 -> ARR = 0xffffffff;

        // set drxn of timer 2 channel 2
        TIM2 -> CCMR1 &= ~TIM_CCMR1_CC2S ;
        TIM2 -> CCMR1 |= TIM_CCMR1_CC2S_0;

        // enable timer 2 channel 2
        TIM2 -> CCER |= TIM_CCER_CC2E;

        // enable interrupt generation for timer 2 channel 2
        TIM2 -> DIER |= TIM_DIER_CC2IE;

        // debounce '*' button (turn on input filtering)
        TIM2 -> CCMR1 |= TIM_CCMR1_IC2F_3 | TIM_CCMR1_IC2F_2 | TIM_CCMR1_IC2F_1 | TIM_CCMR1_IC2F_0;
        TIM2 -> CR1 |= (2<<8);

        // enable timer 2 by using CEN
        TIM2 -> CR1 |= TIM_CR1_CEN;

        // enable interrupts
        NVIC -> ISER[0] = 1 << TIM2_IRQn;

        for(;;)
            asm("wfi");

        return;
}

void TIM3_IRQHandler(void){

    // handler
    GPIOC -> ODR ^= 1<<7;
    TIM3 -> SR = 0;

}

void prob8(void) {

    // enable clock to timer3
    RCC -> APB1ENR |= RCC_APB1ENR_TIM3EN;

    // set timer3 prescaler to divide by 48000
    TIM3 -> PSC = 47999;

    // set auto reload register (FIX)
    TIM3 -> ARR = 499; //???

    // enable update interrupt
    TIM3 -> DIER |= TIM_DIER_UIE;

    // enable timer 3 interrupt in NVIC
    NVIC -> ISER[0] = 1 << TIM3_IRQn;

    // enable timer 3 by using CEN
    TIM3 -> CR1 |= TIM_CR1_CEN;

    prob7();

}

int main(void) {
    //prob1();
    //prob2();
    //prob3();
    //prob4();
    //test_display();
    //prob6();
    //prob7();
    //prob8();
}
