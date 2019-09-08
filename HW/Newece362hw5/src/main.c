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

// This is used by the interrupt service routine.
volatile int counter = 0;

int recmult(int, int);
void enable_porta(void);
void enable_portc(void);
void setup_pa1(void);
void setup_pa2(void);
void setup_pc8(void);
void setup_pc9(void);
void action8(void);
void action9(void);
void EXTI2_3_IRQHandler(void);
void enable_exti2(void);

//====================================================================
// The fail subroutine is invoked when a test fails.
// The test variable indicates which test failed.
// Use the debugger to walk back up the stack to find
// the point at which fail() was called.
//====================================================================
void fail(int test) {
    for(;;)
        asm("wfi");
}

//====================================================================
// This will try out all the subroutines you wrote in hw5.s.
//====================================================================
int main(void)
{
    int x;

    // Q1: recmult
    x = recmult(5,3);    // 15
    if (x != 15)
        fail(1);
    x = recmult(-20,17); // -340
    if (x != -340)
        fail(1);
    x = recmult(15,-13); // -195
    if (x != -195)
        fail(1);
    x = recmult(-29,-34); // 986
    if (x != 986)
        fail(1);

    // Q2: enable_porta
    RCC->AHBENR &= ~RCC_AHBENR_GPIOAEN;
    enable_porta();
    if ((RCC->AHBENR & (RCC_AHBENR_GPIOAEN|RCC_AHBENR_GPIOBEN|RCC_AHBENR_GPIOCEN)) !=
            RCC_AHBENR_GPIOAEN)
        fail(2);
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    enable_porta();
    if ((RCC->AHBENR & (RCC_AHBENR_GPIOAEN|RCC_AHBENR_GPIOBEN|RCC_AHBENR_GPIOCEN)) !=
            (RCC_AHBENR_GPIOAEN|RCC_AHBENR_GPIOCEN))
        fail(2);

    // Q3: enable_portc
    RCC->AHBENR &= ~RCC_AHBENR_GPIOAEN;
    RCC->AHBENR &= ~RCC_AHBENR_GPIOCEN;
    enable_portc();
    if ((RCC->AHBENR & (RCC_AHBENR_GPIOAEN|RCC_AHBENR_GPIOBEN|RCC_AHBENR_GPIOCEN)) !=
            RCC_AHBENR_GPIOCEN)
        fail(3);
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    enable_portc();
    if ((RCC->AHBENR & (RCC_AHBENR_GPIOAEN|RCC_AHBENR_GPIOBEN|RCC_AHBENR_GPIOCEN)) !=
            (RCC_AHBENR_GPIOAEN|RCC_AHBENR_GPIOCEN))
        fail(3);

    // Q4: setup_pa1
    GPIOA->MODER |= 3<<(2*1);  // Sometimes, we deliberately mess with the initial values
    GPIOA->PUPDR |= 3<<(2*1);  // so that you get practice fully clearing/setting them.
    setup_pa1();
    if (GPIOA->MODER != 0x28000000)
        fail(4);
    if (GPIOA->PUPDR != 0x24000004)
        fail(4);
    GPIOA->MODER &= ~(3<<(2*1));
    GPIOA->PUPDR &= ~(3<<(2*1));
    setup_pa1();
    if (GPIOA->MODER != 0x28000000)
        fail(4);
    if (GPIOA->PUPDR != 0x24000004)
        fail(4);

    // Q5: setup_pa2
    GPIOA->MODER |= 3<<(2*2);  // Again, messing up these values.
    GPIOA->PUPDR |= 3<<(2*2);
    setup_pa2();
    if (GPIOA->MODER != 0x28000000)
        fail(5);
    if (GPIOA->PUPDR != 0x24000024)
        fail(5);
    GPIOA->MODER &= ~(3<<(2*2));
    GPIOA->PUPDR &= ~(3<<(2*2));
    setup_pa2();
    if (GPIOA->MODER != 0x28000000)
        fail(5);
    if (GPIOA->PUPDR != 0x24000024)
        fail(5);

    // Q6: setup_pc8
    GPIOC->MODER   |= 3<<(2*8);
    GPIOC->OSPEEDR |= 3<<(2*8);
    setup_pc8();
    if (GPIOC->MODER != 1<<(2*8))
        fail(6);
    if (GPIOC->OSPEEDR != 3<<(2*8))
        fail(6);
    GPIOC->MODER   &= ~(3<<(2*8));
    GPIOC->OSPEEDR &= ~(3<<(2*8));
    setup_pc8();
    if (GPIOC->MODER != 1<<(2*8))
        fail(6);
    if (GPIOC->OSPEEDR != 3<<(2*8))
        fail(6);

    setup_pc9();
    if (GPIOC->MODER != (1<<(2*8)) + (1<<(2*9)))
        fail(7);
    if (GPIOC->OSPEEDR != (3<<(2*8)) + (1<<(2*9)))
        fail(7);
    GPIOC->MODER |= 3<<(2*9);
    GPIOC->OSPEEDR |= 3<<(2*9);
    setup_pc9();
    if (GPIOC->MODER != (1<<(2*8)) + (1<<(2*9)))
        fail(7);
    if (GPIOC->OSPEEDR != (3<<(2*8)) + (1<<(2*9)))
        fail(7);

    // Q8: action8
    // You did all that work to configure PA1 and PA2 as inputs, and now
    // we're just going to reconfigure them as outputs just so we can test
    // action8 easily.  Remember that you can still read from the IDR when
    // the pin is configured as output.  Here, the action8 subroutine will
    // read the value that we write to the ODR.  This is our basic means of
    // evaluating the GPIO pins.
    GPIOA->MODER &= ~(3<<(2*1)) & ~(3<<(2*2));
    GPIOA->MODER |= (1<<(2*1)) + (1<<(2*2));
    for(x=0; x<4; x++) {
        GPIOA->BRR = 0x6;
        GPIOA->BSRR = x<<1;
        const static int check[] = { 0, 1, 0, 0 };
        action8();
        if (((GPIOC->IDR >> 8) & 1) != check[x])
            fail(8);
    }

    // Q9: action9
    for(x=0; x<4; x++) {
        GPIOA->BRR = 0x6;
        GPIOA->BSRR = x<<1;
        const static int check[] = { 0, 0, 1, 0 };
        action9();
        if (((GPIOC->IDR >> 9) & 1) != check[x])
            fail(9);
    }

    // Reset PA1 and PA2 to be inputs before anyone notices.
    setup_pa1();
    setup_pa2();

    // Q10: EXTI2_3_IRQHandler
    counter = 6;
    EXTI2_3_IRQHandler();
    if (counter != 7)
        fail(10);

    // Q11: enable_exti2
    counter = 0;
    SYSCFG->EXTICR[0] |= 0xf<<(4*2);
    enable_exti2();
    if (SYSCFG->EXTICR[0] != 0)
        fail(11);
    if (EXTI->RTSR != 1<<2)
        fail(11);
    if (EXTI->IMR != 0x0f940000 + (1<<2))
        fail(11);
    if (!(NVIC->ISER[0] & (1<<EXTI2_3_IRQn)))
        fail(11);
    // Try it:
    NVIC->ISPR[0] = 1<<EXTI2_3_IRQn;
    if (counter != 1)
        fail(11);

    // Total success!
    // If you made it this far through the tests, the
    // subroutines you wrote in hw5.s are working properly.
    for(;;)
        asm("wfi");

    return 0;
}
