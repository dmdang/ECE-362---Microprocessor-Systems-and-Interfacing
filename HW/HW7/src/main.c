#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include <stdlib.h>

int up_down = -1;
extern autotest(void);
void tim1_init(void);
void tim2_init(void);

int main(void)
{
    autotest();
    tim1_init();
    tim2_init();
    while(1) asm("wfi");
    return 0;
}

void tim1_init(void){

    // enable clock to gpio a
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;

    // configure pa10 (ch3), pa9 (ch2), pa8 (ch1) as alt fxns

    GPIOA -> MODER &= ~(3<<20);
    GPIOA -> MODER &= ~(3<<18);
    GPIOA -> MODER &= ~(3<<16);

    GPIOA -> MODER |= 2<<20;
    GPIOA -> MODER |= 2<<18;
    GPIOA -> MODER |= 2<<16;

    GPIOA -> AFR[1] &= ~(0xfff);

    GPIOA -> AFR[1] |= 2;
    GPIOA -> AFR[1] |= 2 << 4;
    GPIOA -> AFR[1] |= 2 << 8;


    // set clock to timer 1
    RCC -> APB2ENR |= RCC_APB2ENR_TIM1EN;

    // set tim1_psc so clock is 1 MHz
    TIM1 -> PSC = 47;

    // choose arr so pwm freq = 100 Hz
    TIM1 -> ARR = 9999;

    // configure tm1 for pwm1 output compare mode (channels 1,2,3)
    TIM1 -> CCMR1 |= TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;
    TIM1 -> CCMR1 |= TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2PE;
    TIM1 -> CCMR2 |= TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3PE;
    TIM1 -> CCER |= TIM_CCER_CC1E;
    TIM1 -> CCER |= TIM_CCER_CC2E;
    TIM1 -> CCER |= TIM_CCER_CC3E;
    TIM1 -> BDTR |= TIM_BDTR_MOE;
    TIM1 -> CR1 |= TIM_CR1_CEN;

    // write value to CCRx to set duty cycle (100 = dimmest, 0 = brightest)
    TIM1 -> CCR1 = 9900;
    TIM1 -> CCR2 = 9900;
    TIM1 -> CCR3 = 9900;

}

void tim2_init(void){

    // enable clock to timer 2
    RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;

    // trigger interrupt every 10 ms
    TIM2 -> PSC = 9;
    TIM2 -> ARR = 47999;

    TIM2 -> DIER |= TIM_DIER_UIE;
    NVIC -> ISER[0] = 1 << TIM2_IRQn;

    // start timer2
    TIM2 -> CR1 |= TIM_CR1_CEN;


}

void TIM2_IRQHandler(){

    TIM2 -> SR &= ~(1<<0);

    int a;
    int b;
    int c;

    if (up_down == 1)
    {
        a = (TIM1 -> CCR1);
        a = a * 262;
        a = a >> 8;

        if (a >= 9900){

            up_down = -1;
        }

        (TIM1 -> CCR1) = a;

        b = (TIM1 -> CCR2);
        b = b * 262;
        b = b >> 8;

        if (b >= 9900){

            up_down = -1;
        }

        (TIM1 -> CCR2) = b;

        c = (TIM1 -> CCR3);
        c = c * 262;
        c = c >> 8;

        if (c >= 9900){

            up_down = -1;
        }

        (TIM1 -> CCR3) = c;
    }

    else
    {
        a = (TIM1 -> CCR1);
        a = a * 251;
        a = a >> 8;

        if (a <= 600){

            up_down = 1;
        }

        (TIM1 -> CCR1) = a;

        b = (TIM1 -> CCR2);
        b = b * 251;
        b = b >> 8;

        if (b <= 600){

            up_down = 1;
        }

        (TIM1 -> CCR2) = b;

        c = (TIM1 -> CCR3);
        c = c * 251;
        c = c >> 8;

        if (c <= 600){

            up_down = 1;
        }

        (TIM1 -> CCR3) = c;
    }

}
