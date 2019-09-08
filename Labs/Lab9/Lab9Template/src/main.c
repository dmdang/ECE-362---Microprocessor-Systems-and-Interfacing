#include "stm32f0xx.h"
#include "stm32f0_discovery.h"

#define FAIL -1
#define SUCCESS 0
#define WR 0
#define RD 1

void serial_init(void);
void test_prob3(void);
void prob5(void);
void check_config(void);
void test_wiring(void);
void test_prob4(void);

//===========================================================================
// Check wait for the bus to be idle.
void I2C1_waitidle(void) {
    while ((I2C1->ISR & I2C_ISR_BUSY) == I2C_ISR_BUSY);  // while busy, wait.
}

//===========================================================================
// Subroutines for step 2.
//===========================================================================
// Initialize I2C1
/*
1.  Enable clock to GPIOB
2.  Configure PB6 and PB7 to alternate functions I2C1_SCL and I2C1_SDA
3.  Enable clock to I2C1
4.  Set I2C1 to 7 bit mode
5.  Enable NACK generation for I2C1
6.  Configure the I2C1 timing register so that PSC is 6, SCLDEL is 3 and SDADEL is 1 and SCLH is 3 and SCLL is 1
7.  Disable own address1 and own address 2 and set the 7 bit own address to 1
8.  Enable I2C1
 */
void init_I2C1() {
    // Student code goes here

    // enable clock to gpiob
    RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;

    // config pb6 and pb7 as alt fxns
    GPIOB -> MODER |= 2 << 12;
    GPIOB -> MODER |= 2 << 14;

    GPIOB -> AFR[0] |= (1<<24);
    GPIOB -> AFR[0] |= (1<<28);

    // enable clock to i2c1
    RCC -> APB1ENR |= RCC_APB1ENR_I2C1EN;

    // set i2c1 to 7 bit mode
    I2C1 -> CR2 &= ~(I2C_CR2_ADD10);

    // enable nack gen for i2c1
    I2C1 -> CR2 |= I2C_CR2_NACK;

    // make psc = 6, scldel = 3, sdadel = 1, sclh = 3, scll = 1
    I2C1 -> TIMINGR = 0;
    I2C1 -> TIMINGR &= ~(I2C_TIMINGR_PRESC);
    I2C1 -> TIMINGR |= 4 << 28; // psc
    I2C1 -> TIMINGR |= 3 << 20; // scldel
    I2C1 -> TIMINGR |= 1 << 16; // sdadel
    I2C1 -> TIMINGR |= 3 << 8; // sclh
    I2C1 -> TIMINGR |= 9 << 0; // scll

    // disable own address1 and own address2, set 7 bit own address to 1
    I2C1 -> OAR1 &= ~I2C_OAR1_OA1EN;
    I2C1 -> OAR2 &= ~I2C_OAR2_OA2EN;
    I2C1 -> OAR1 = I2C_OAR1_OA1EN | 0x2;

    // enable i2c1
    I2C1 -> CR1 |= I2C_CR1_PE;

    //---------End-----------
}


//===========================================================================
// Subroutines for step 3.
//===========================================================================
// See lab document for description
void I2C1_start(uint8_t addr, uint32_t dir) {
    // Student code goes here

    // clear SADD bits in i2c1_cr2
    I2C1 -> CR2 &= ~(I2C_CR2_SADD);

    // set SADD addr. in i2c1_cr2
    I2C1 -> CR2 |= ((addr<<1) & I2C_CR2_SADD);

    // check drxn bit with 'RD'
    if (dir == RD){

        I2C1 -> CR2 |= I2C_CR2_RD_WRN;
    }

    else{

        I2C1 -> CR2 &= ~(I2C_CR2_RD_WRN);

    }

    // set START bit in CR2 reg.
    I2C1 -> CR2 |= I2C_CR2_START;


    //---------End-----------
}

// See lab document for description
void I2C1_stop() {
    // Student code goes here

    if (I2C1 -> ISR & I2C_ISR_STOPF){

        return;
    }

    I2C1 -> CR2 |= I2C_CR2_STOP;

    while((I2C1 -> ISR & I2C_ISR_STOPF) == 0);
    I2C1 -> ICR |= I2C_ICR_STOPCF;

    //---------End-----------
}

// See lab document for description
int I2C1_senddata(uint8_t* data, uint32_t size) {
    // Student code goes here

    int i;
    // clear NBYTES of CR2
    I2C1 -> CR2 &= ~(I2C_CR2_NBYTES);

    // set NBYTES bits to param size
    I2C1 -> CR2 |= (size << 16);

    // write for loop
    for (i = 0; i < size; i++){

        int timeout = 0;

        while ((I2C1 -> ISR & I2C_ISR_TXIS) == 0){

            timeout += 1;

            if (timeout > 1000000){
                return FAIL;
            }
        }

        I2C1 -> TXDR = data[i] & I2C_TXDR_TXDATA;

    }

    // wait until TC flag is set || NACK flag is set
    while((I2C1 -> ISR & I2C_ISR_TC) == 0 && (I2C1 -> ISR & I2C_ISR_NACKF) == 0);

    if ((I2C1 -> ISR & I2C_ISR_NACKF) != 0){

        return FAIL;
    }

    else{

        //I2C1_stop(); // need this?
        return SUCCESS;
    }

    //---------End-----------
}

// See lab document for description
int I2C1_readdata(int8_t* data, uint32_t size) {
    // Student code goes here

    int i;
    // clear NBYTES of CR2
    I2C1 -> CR2 &= ~(I2C_CR2_NBYTES);

    // set NBYTES bits to param size
    I2C1 -> CR2 |= (size << 16);

    for (i = 0; i < size; i++){

        int timeout = 0;

        while ((I2C1 -> ISR & I2C_ISR_RXNE) == 0){

            timeout += 1;

            if (timeout > 1000000){
                return FAIL;
            }

        }

        I2C1 -> RXDR = data[i] & I2C_RXDR_RXDATA;

    }

    // wait until TC flag is set || NACK flag is set
    while((I2C1 -> ISR & I2C_ISR_TC) == 0 && (I2C1 -> ISR & I2C_ISR_NACKF) == 0);

    if ((I2C1 -> ISR & I2C_ISR_NACKF) != 0){

        return FAIL;
    }

    else{

        //I2C1_stop(); // need this?
        return SUCCESS;
    }

    //---------End-----------
}

//===========================================================================
// Subroutines for step 4.
//===========================================================================
// See lab document for description
int read_temperature() {
    // Student code goes here

    int data[1] = {0};
    int temp = -20;

    I2C1_waitidle();
    I2C1_start(0x4d, WR);
    I2C1_senddata(data, 1);
    I2C1_start(0x4d, RD);
    I2C1_readdata(&temp, 1);
    I2C1_stop();

    return temp;
    //---------End-----------
}


//===========================================================================
// Subroutines for step 5.
//===========================================================================
// See lab document for description
void write_EEPROM(uint16_t wr_addr, uint8_t data) {
    // Student code goes here

    uint8_t upper;
    uint8_t lower;
    upper = (wr_addr & 0xff00) >> 8;
    lower = (wr_addr & 0xff);
    uint8_t write_buf[3] = {upper, lower, data};

    I2C1_waitidle();
    I2C1_start(0x50, WR);
    I2C1_senddata(write_buf, 3);
    I2C1_stop();
    micro_wait(5000);

    //---------End-----------
}

void prob2() {
    init_I2C1();
    check_config();
}

void prob3() {
    test_prob3();
}

void prob4() {
    test_prob4();
}


int main(void)
{
    serial_init();
    //Open the serial terminal to see messages for each part
    //test_wiring();
    //prob2();
    //prob3();
    //prob4();
    prob5();

    while(1);
}
