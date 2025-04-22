/* 
 * File:   main2.c
 * Author: Renedito
 *
 * Created on 8 de Abril de 2025, 11:29
 */

#include <stdio.h>
#include <stdlib.h>

#include <p24fj1024gb610.h>

unsigned char ledState = 0b00000001;

/*
 * 
 */
void setupTimer1(){
    TMR1 = 0;
    T1CON = 0x801110;
    PR1 = (8000000/2) / (8);
    IPC0bits.T1IP = 0x01;
    
    IFS0bits.T1IF = 0; //Clear the Timer1 interrupt status flag
    IEC0bits.T1IE = 1; //Enable Timer1 interrupts
    //T1CONbits.TON = 0x01;
}
void __attribute__((__interrupt__, __shadow__)) _T1Interrupt(void) {
    /* Interrupt Service Routine code goes here */
    IFS0bits.T1IF = 0; //Reset Timer1 interrupt flag and Return from ISR
    LATA = ledState;
    
    ledState <<= 1;
    if(ledState == 0)
        ledState = 0b00000001;
}

int main(int argc, char** argv) {
    ANSA = 1;
    
    TRISA = 0;
    
    LATA = 0b00000000;
    
    setupTimer1();
    
    while(1){

    }
    
    return (1);
}

