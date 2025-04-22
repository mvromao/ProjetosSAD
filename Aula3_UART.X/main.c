/* 
 * File:   main.c
 * Author: Renedito
 *
 * Created on April 15, 2025, 11:31 AM
 */

#include <stdio.h>
#include <stdlib.h>

#include <p24fj1024gb610.h>

/*
 * 
 */
void setupUART1(){
    __builtin_write_OSCCONL(OSCCON & 0xbf);
        
    RPOR8bits.RP17R = 0x0003;
    RPINR18bits.U1RXR = 0x000A;
        
    U1STA = 0;
    U1MODE = 0x8000;
    U1BRG = 12;
    U1STAbits.UTXEN = 1;
    U1STAbits.URXEN = 1;

    U1MODEbits.UARTEN = 1;
    
    IEC0bits.U1TXIE = 0;
    IEC0bits.U1RXIE = 0;
    
}   
/*void setupUART1() {
    __builtin_write_OSCCONL(OSCCON & 0xbf);
    
    IPC3bits.U1TXIP2= 1;
    IPC3bits.U1TXIP1= 0;
    IPC3bits.U1TXIP0= 0;
    IPC2bits.U1RXIP2= 1;

    IPC2bits.U1RXIP1= 0;
    IPC2bits.U1RXIP0= 0;
    
    U1STA = 0;
    U1MODE = 0x8000;
    U1BRG = 25; // (4000000/ (16 * 9600) - 1)
    
    U1STAbits.UTXEN = 1;
    IEC0bits.U1RXIE = 1;
    IEC0bits.U1TXIE = 1;
}*/
void setupTimer1(){
    #pragma config FNOSC = PRI
    #pragma config POSCMOD = HS
    #pragma config JTAGEN = OFF
    #pragma config FWDTEN = OFF
}

void setupADC() {
    ANSA = 0x0020;
    AD1CON1 = 0x0000;    // Manuel converter after sampling, manual sampling
    AD1CON2 = 0;
    AD1CON3 = 0x1F02;    // Sample time = 31 TAD, ADC clock = Fosc/2
    
    AD1CON1bits.ADON = 1;
    //AD1CON1bits.SAMP = 0;

}
void __attribute__((__interrupt__, __shadow__)) _T1Interrupt(void) {
    /* Interrupt Service Routine code goes here */
    IFS0bits.T1IF = 0; //Reset Timer1 interrupt flag and Return from ISR
    
}

void writeChar(char a){
    while(U1STAbits.UTXBF);
        U1TXREG = a;
    return;
}

void writeString(char * str){
    while (*str){
        writeChar(*str);
        *str++;
    }
}
int getPotentiometerValue() {
    AD1CHS = 5;          // AN5 as input
    
    AD1CON1bits.SAMP = 1;   // Começar o sampling
        
    for (int i = 0; i < 100; i++);  
    AD1CON1bits.SAMP = 0;
        
    while(!AD1CON1bits.DONE);
       
    return ADC1BUF0;
}

void transmitPotentiometerData(int value, char * str) {
    value = getPotentiometerValue();
    sprintf(str, "Valor do potentiometro = %d\r\n", value);
        
    //sprintf(str, "Hello World!\n");
    writeString(str);
}

int main(int argc, char** argv) {
    setupUART1();
    setupTimer1();
    
    TRISAbits.TRISA5 = 1;  // Set RA5 as input

    setupADC();
    
    int value;
    char str [50];
    
    while(1) {
        transmitPotentiometerData(value, &str);
    }
    return (1);
}

