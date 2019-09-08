/*
 * LCDFunctions.h
 *
 *  Created on: Apr 13, 2019
 *      Author: David
 */
#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
#ifndef LCDFunctionsHeader
#define LCDFunctionsHeader

#define LCDD0Pin 15
#define LCDD0Port GPIOB
#define LCDD1Pin 6
#define LCDD1Port GPIOC
#define LCDD2Pin 7
#define LCDD2Port GPIOC
#define LCDD3Pin 8
#define LCDD3Port GPIOC
#define LCDD4Pin 9
#define LCDD4Port GPIOC
#define LCDD5Pin 8
#define LCDD5Port GPIOA
#define LCDD6Pin 9
#define LCDD6Port GPIOA
#define LCDD7Pin 10
#define LCDD7Port GPIOA
#define LCDEnablePin 14
#define LCDEnablePort GPIOB
#define LCDReadWritePin 13
#define LCDReadWritePort GPIOB
#define LCDRegisterSelectPin 12
#define LCDRegisterSelectPort GPIOB
#define TimeDelayBeforeEnable 400
#define TimeDelayBeforeDisable 800

void notExactTimeDelay(int timeDelay){

    volatile int i;
    for (int i; i < timeDelay; i++){


    }

}


void SetPortAndPinForOutput(GPIO_TypeDef * port, int pinNumber){

    if (port == GPIOA){
        RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
    }
    if (port == GPIOB){
        RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
    }
    if (port == GPIOC){
        RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;
    }
    if (port == GPIOD){
        RCC -> AHBENR |= RCC_AHBENR_GPIODEN;
    }
    if (port == GPIOF){
        RCC -> AHBENR |= RCC_AHBENR_GPIOFEN;
    }

    port->MODER &= ~(1 << ((pinNumber*2)+1));
    port->MODER |= (1 << pinNumber*2);

    port->OTYPER &= ~(1 << pinNumber);

    port->OSPEEDR |= (1 << ((pinNumber*2)+1));
    port->OSPEEDR |= (1 << pinNumber*2);

    port->PUPDR &= ~(1 << pinNumber);
}

void InitializePortsForLCD(){

    SetPortAndPinForOutput(LCDD0Port, LCDD0Pin);
    SetPortAndPinForOutput(LCDD1Port, LCDD1Pin);
    SetPortAndPinForOutput(LCDD2Port, LCDD2Pin);
    SetPortAndPinForOutput(LCDD3Port, LCDD3Pin);
    SetPortAndPinForOutput(LCDD4Port, LCDD4Pin);
    SetPortAndPinForOutput(LCDD5Port, LCDD5Pin);
    SetPortAndPinForOutput(LCDD6Port, LCDD6Pin);
    SetPortAndPinForOutput(LCDD7Port, LCDD7Pin);

    SetPortAndPinForOutput(LCDEnablePort, LCDEnablePin);
    SetPortAndPinForOutput(LCDReadWritePort, LCDReadWritePin);
    SetPortAndPinForOutput(LCDRegisterSelectPort, LCDRegisterSelectPin);
}

void SendBitToPortAndPin(GPIO_TypeDef * port, int pinNumber, uint8_t bitState){
    if (bitState){
        port->BSRR |= (1 << pinNumber);
    }
    else{
        port->BRR |= (1 << pinNumber);
    }
}

void LCDEnable(){

    notExactTimeDelay(TimeDelayBeforeEnable);
    SendBitToPortAndPin(LCDEnablePort, LCDEnablePin, 1);
}

void LCDSetToWrite(){

    SendBitToPortAndPin(LCDReadWritePort, LCDReadWritePin, 0);

}

void LCDSetToRead(){

    SendBitToPortAndPin(LCDReadWritePort, LCDReadWritePin, 1);
}

void LCDInstructionMode(){

    SendBitToPortAndPin(LCDRegisterSelectPort, LCDRegisterSelectPin, 0);
}

void LCDCharacterMode(){

    SendBitToPortAndPin(LCDRegisterSelectPort, LCDRegisterSelectPin, 1);
}

void LCDSendAByteToTheLCDDataPins(char character){

    SendBitToPortAndPin(LCDD0Port, LCDD0Pin, character & 0b00000001);
    SendBitToPortAndPin(LCDD1Port, LCDD1Pin, character & 0b00000010);
    SendBitToPortAndPin(LCDD2Port, LCDD2Pin, character & 0b00000100);
    SendBitToPortAndPin(LCDD3Port, LCDD3Pin, character & 0b00001000);
    SendBitToPortAndPin(LCDD4Port, LCDD4Pin, character & 0b00010000);
    SendBitToPortAndPin(LCDD5Port, LCDD5Pin, character & 0b00100000);
    SendBitToPortAndPin(LCDD6Port, LCDD6Pin, character & 0b01000000);
    SendBitToPortAndPin(LCDD7Port, LCDD7Pin, character & 0b10000000);

    notExactTimeDelay(TimeDelayBeforeDisable);
    SendBitToPortAndPin(LCDEnablePort, LCDEnablePin, 0);
}

void LCDSendACharacter(char character){
    LCDSetToWrite();
    LCDCharacterMode();
    LCDEnable();
    LCDSendAByteToTheLCDDataPins(character);
}

void LCDSendAnInstruction(char character){
    LCDSetToWrite();
    LCDInstructionMode();
    LCDEnable();
    LCDSendAByteToTheLCDDataPins(character);
}

void LCDSendAString(char * StringOfCharacters){

    while(*StringOfCharacters){

        LCDSendACharacter(*StringOfCharacters++);
    }
}

void LCDSetupDisplay(){
    LCDSendAnInstruction(LCDInstruction_Se)
}
#endif /* LCDFUNCTIONS_H_ */
