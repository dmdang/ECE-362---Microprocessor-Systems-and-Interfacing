#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#include "wavetable.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#define SAMPLES 128


void micro_wait(unsigned int micro_seconds);

int s1 = 0;
int s2 = 0;
float a1 = 1;
float a2 = 1;
int circ_buf[SAMPLES] = {0};
uint32_t start = 0, end = 0;

void insert_circ_buf(int val) {
    if(start <= end && end <= (SAMPLES-1)) {
        circ_buf[end++] = val;
    } else if(start < end && end > (SAMPLES-1)) {
        circ_buf[0] = val;
        end = 0;
        start = end + 1;
    } else if(end < start && start < (SAMPLES-1)) {
        start++;
        circ_buf[end++] = val;
    } else {
        start = 0;
        circ_buf[end++] = val;
    }
}

float get_time_period(int min, int max) {
    int start_interval = 0;
    int sample_count = 0;
    int avg_samples = 0;
    int no_cycles = 0;

    for(int i = 0; i < SAMPLES; i++) {


        if(circ_buf[i] < (0.3*max) && start_interval == 1) {
            start_interval = 0;
            avg_samples += sample_count;
            sample_count = 0;
            no_cycles++;
        }

        if(circ_buf[i] > (0.8*max) && start_interval == 0) {
            start_interval = 1;

        }


        if(start_interval == 1)
            sample_count++;
    }

    int avg_interval = avg_samples / no_cycles;
    return (2 * avg_interval * 200e-6);
}

// This function
// 1) enables clock to port A,
// 2) sets PA0, PA1, PA2 and PA4 to analog mode
void setup_gpio() {

    // enable clock to port A
    RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;

    // clear pins first
    // set pa0, pa1, pa2, pa4 to analog
    GPIOA -> MODER &= ~(3<<0);
    GPIOA -> MODER &= ~(3<<2);
    GPIOA -> MODER &= ~(3<<4);
    GPIOA -> MODER &= ~(3<<8);

    GPIOA -> MODER |= 3<<0;
    GPIOA -> MODER |= 3<<2;
    GPIOA -> MODER |= 3<<4;
    GPIOA -> MODER |= 3<<8;
}

// This function should enable the clock to the
// onboard DAC, enable trigger,
// setup software trigger and finally enable the DAC.
void setup_dac() {

    // enable clock to onboard DAC
    RCC -> APB1ENR |= RCC_APB1ENR_DACEN;

    // enable trigger
    DAC -> CR |= DAC_CR_TEN1;

    // setup software trigger
    DAC -> CR |= DAC_CR_TSEL1;

    // enable DAC channel 1
    DAC -> CR |= DAC_CR_EN1;
}

// This function should,
// enable clock to timer2,
// setup prescalaer and arr so that the interrupt is triggered 100us,
// enable the timer 2 interrupt and start the timer.
void setup_timer2() {

    // enable clock to timer2
    RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;

    // setup prescaler and arr to trigger interrupt every 100us
    TIM2 -> PSC = 9;
    TIM2 -> ARR = 479;


    // enable timer2 interrupt
    TIM2 -> DIER |= TIM_DIER_UIE;
    NVIC -> ISER[0] = 1 << TIM2_IRQn;

    // start timer
    TIM2 -> CR1 |= TIM_CR1_CEN;
}

// This function should, enable clock to timer3,
// setup prescalaer and arr so that the interrupt is triggered 200us,
// enable the timer 3 interrupt and start the timer.
void setup_timer3() {

    // enable clock to timer3
    RCC -> APB1ENR |= RCC_APB1ENR_TIM3EN;

    // setup prescaler and arr to trigger interrupt every 200us
    TIM3 -> PSC = 9;
    TIM3 -> ARR = 959;


    // enable timer3 interrupt
    TIM3 -> DIER |= TIM_DIER_UIE;
    NVIC -> ISER[0] = 1 << TIM3_IRQn;

    // start timer3
    TIM3 -> CR1 |= TIM_CR1_CEN;

}

// This function should enable the clock to ADC,
// turn on the clocks, wait for ADC to be ready.
void setup_adc() {

    // enable clock to ADC
    RCC -> APB2ENR |= RCC_APB2ENR_ADC1EN;

    // turn on clocks, wait for ADC to be ready
    RCC -> CR2 |= RCC_CR2_HSI14ON;
    while(!(RCC -> CR2 & RCC_CR2_HSI14RDY));

    ADC1 -> CR |= ADC_CR_ADEN;
    while(!(ADC1 -> ISR & ADC_ISR_ADRDY));

    // wait for ADCstart to be 0 (might not need)
    while((ADC1 -> CR & ADC_CR_ADSTART));


}

// This function should return the value from the ADC
// conversion of the channel specified by the �channel� variable.
// Make sure to set the right bit in channel selection
// register and do not forget to start adc conversion.
int read_adc_channel(unsigned int channel) {

    // unselect all ADC channels
    ADC1 -> CHSELR = 0;

    // Select channel
    ADC1 -> CHSELR |= 1 << (channel);

    // wait for ADC to be ready
    while(!(ADC1 -> ISR & ADC_ISR_ADRDY));

    // start ADC
    ADC1 -> CR |= ADC_CR_ADSTART;

    // wait for end of conversion
    while(!(ADC1 -> ISR & ADC_ISR_EOC));

    // return value from ADC conversion
    return ADC1 -> DR;

}

//The interrupt handler should read the value from the ADC�s channel 2 input.
// Use the insert_circ_buffer() function to insert the read value into the circular buffer.
void TIM3_IRQHandler() {

    // read value from ADC's channel 2 input
    int ch2;
    ch2 = read_adc_channel(2);

    // use insert_circ_buffer() function to insert read value into circular buffer
    insert_circ_buf(ch2);

    // acknowledge interrupt
    // clear pending bit
    TIM3 -> SR &= ~(1<<0);

}

// TIM2_IRQHandler: The interrupt handler should start the DAC conversion using the software trigger,
// and should use the wavetable.h to read from the array and write it into the DAC.
// Every time the interrupt is called you will read a new element from the �wavetable� array.
// So you might need to use a global variable as an index to the array.
// Note that the array has 100 elements, make sure you do not read wavetable[100].

// global variable index
int index = 0;
int index2 = 0;

void TIM2_IRQHandler() {

    // start DAC conversion with software trigger
    // use wavetable.h to read from array and write into DAC
    while ((DAC -> SWTRIGR & DAC_SWTRIGR_SWTRIG1) == DAC_SWTRIGR_SWTRIG1);

    // read new element from wavetable every time interrupt is called
    //DAC -> DHR12R1 = wavetable[index]; // part 6.2
    //DAC -> DHR12R1 = (wavetable[index] + wavetable[index2]) >> 1; // part 6.3

    float wx1 = wavetable[index]; // part 6.4
    float wx2 = wavetable[index2]; // part 6.4
    int result = (int) (a1 * wx1 + a2 * wx2); // part 6.4
    DAC -> DHR12R1 = result >> 1; // part 6.4

    DAC -> SWTRIGR |= DAC_SWTRIGR_SWTRIG1;

    // use global var. as array index (don't read wavetable[100])
    index += 1;
    index2 += 2; // part 6.3
    if (index >= 100){
        index = 0;
    }

    if (index2 >= 100){
        index2 = 0; // part 6.3
    }

    // clear pending bit
    TIM2 -> SR &= ~(1<<0);
}


int main(void)
{
    serial_init();
    setup_gpio();
    setup_adc();
    setup_dac();
    setup_timer2();
    setup_timer3();

    int i;
    float period;
    float frequency;
    char line[30];

    while(1) {
        /* Student code goes here */

        // call read_adc_channel()
        a1 = read_adc_channel(0); // pa0
        a2 = read_adc_channel(1); // pa1
        a1 /= 4095.0;
        a2 /= 4095.0;

        // call serial_init(), loop through circular buffer to calc min/max

        serial_init();

        int max = INT_MIN;
        int min = INT_MAX;

        //int max = circ_buf[0];
        //int min = circ_buf[0];

        for (i = 0; i < SAMPLES; i++)
        {
            if (circ_buf[i] > max)
            {
                max = circ_buf[i];
            }

            if (circ_buf[i] < min)
            {
                min = circ_buf[i];
            }
        }

        // call get_time_period(min, max) to estimate period of sample signal
        period = get_time_period(min, max);

        // calc frequency
        frequency = (float) (1 / period);

        int a1Temp;
        int a2Temp;
        int freqTemp;
        a1Temp = (int) (a1 * 100);
        a2Temp = (int) (a2 * 100);
        freqTemp = (int) frequency;
        // printf to display a1, a2, frequency to terminal
        printf("a1: %d a2: %d, frequency: %d\n", a1Temp, a2Temp, freqTemp);

        micro_wait(500000);
    }
}
